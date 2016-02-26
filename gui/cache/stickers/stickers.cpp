#include "stdafx.h"

#include "../../../corelib/enumerations.h"

#include "../../core_dispatcher.h"
#include "../../utils/gui_coll_helper.h"
#include "../../utils/utils.h"

#include "stickers.h"

UI_STICKERS_NS_BEGIN

std::unique_ptr<cache> g_cache;
cache& get_cache();

sticker::sticker()
	: set_id_(0)
	, id_(0)
{

}

sticker::sticker(const int32_t _id, const int32_t _set_id)
	: set_id_(_set_id)
	, id_(_id)
{
	assert(set_id_ > 0);
	assert(id_ > 0);
}

void sticker::unserialize(core::coll_helper _coll)
{
	id_ = _coll.get_value_as_int("id");
}

int32_t sticker::get_id() const
{
	return id_;
}

int32_t sticker::get_set_id() const
{
	return set_id_;
}

void sticker::set_set_id(int32_t _val)
{
	set_id_ = _val;
}

QImage sticker::get_image(const core::sticker_size _size) const
{
	const auto found = images_.find(_size);
	if (found == images_.cend())
	{
		return QImage();
	}

	return std::get<0>(found->second);
}

void sticker::set_image(const core::sticker_size _size, const QImage &_image)
{
	assert(!_image.isNull());

	auto &image_data = images_[_size];

	if (std::get<0>(image_data).isNull())
	{
		image_data = std::make_tuple(_image, false);
	}
}

bool sticker::is_image_requested(const core::sticker_size _size) const
{
	const auto found = images_.find(_size);
	if (found == images_.end())
	{
		return false;
	}

	return std::get<1>(found->second);
}

void sticker::set_image_requested(const core::sticker_size _size, const bool _val)
{
	auto &image_data = images_[_size];

	image_data = std::make_tuple(
		std::get<0>(image_data),
		_val
	);

	assert(!_val || std::get<0>(image_data).isNull());
}

void sticker::clear_cache()
{
	for (auto &pair : images_)
	{
		auto &image = std::get<0>(pair.second);

		pair.second = std::make_tuple(image, false);

		image.loadFromData(0, 0);
	}
}

set::set(int32_t _max_size)
	:	max_size_(_max_size),
		id_(-1)
{

}


QImage set::get_sticker_image(const int32_t _index, const core::sticker_size _size)
{
	assert(_index >= 0);
	assert(_size > core::sticker_size::min);
	assert(_size < core::sticker_size::max);

	if (_index >= (int32_t)stickers_.size())
	{
		assert(false);
		return QImage();
	}

	auto image = stickers_[_index]->get_image(_size);

	if (image.isNull() && !stickers_[_index]->is_image_requested(_size))
	{
		const auto set_id = stickers_[_index]->get_set_id();
		assert(set_id > 0);

		const auto sticker_id = stickers_[_index]->get_id();
		assert(sticker_id);

		Ui::GetDispatcher()->getSticker(set_id, sticker_id, _size);

		stickers_[_index]->set_image_requested(_size, true);
	}

	return image;
}

void set::set_sticker_image(const int32_t _sticker_id, const core::sticker_size _size, QImage _image)
{
	assert(_sticker_id > 0);
	assert(_size > core::sticker_size::min);
	assert(_size < core::sticker_size::max);

	for (uint32_t i = 0; i < stickers_.size(); i++)
	{
		if (_sticker_id != stickers_[i]->get_id())
		{
			continue;
		}

		stickers_[i]->set_image(_size, _image);
		break;
	}
}

void set::set_id(int32_t _id)
{
	id_= _id;
}

int32_t set::get_id() const
{
	return id_;
}

void set::set_name(const QString& _name)
{
	name_ = _name;
}

QString set::get_name() const
{
	return name_;
}

void set::load_icon(char* _data, int32_t _size)
{
	icon_.loadFromData((const uchar*)_data, _size);
	Utils::check_pixel_ratio(icon_);
}

QPixmap set::get_icon() const
{
	return icon_;
}

const stickers_array& set::get_stickers() const
{
	return stickers_;
}

void set::unserialize(core::coll_helper _coll)
{
	set_id(_coll.get_value_as_int("id"));
	set_name(_coll.get_value_as_string("name"));

	if (_coll.is_value_exist("icon"))
	{
		core::istream* icon_stream = _coll.get_value_as_stream("icon");
		if (icon_stream)
		{
			int32_t icon_size = icon_stream->size();
			if (icon_size > 0)
			{
				load_icon((char*)icon_stream->read(icon_size), icon_size);
			}
		}
	}

	core::iarray* sticks = _coll.get_value_as_array("stickers");

	stickers_.reserve(sticks->size());

	for (int32_t i = 0; i < sticks->size(); i++)
	{
		core::coll_helper coll_sticker(sticks->get_at(i)->get_as_collection(), false);

		auto inserted_sticker = std::make_shared<sticker>();
		inserted_sticker->unserialize(coll_sticker);
		inserted_sticker->set_set_id(get_id());
		stickers_.push_back(inserted_sticker);
	}
}

void set::reset_flag_requested(const int32_t _sticker_id, const core::sticker_size _size)
{
	for (uint32_t i = 0; i < stickers_.size(); i++)
	{
		if (_sticker_id == stickers_[i]->get_id())
		{
			stickers_[i]->set_image_requested(_size, false);
		}
	}
}

void set::clear_cache()
{
	for (uint32_t i = 0; i < stickers_.size(); i++)
	{
		stickers_[i]->clear_cache();
	}
}

bool set::add_sticker(std::shared_ptr<sticker> _sticker)
{
	for (auto iter = stickers_.begin(); iter != stickers_.end(); iter++)
	{
		if ((*iter)->get_id() == _sticker->get_id() && (*iter)->get_set_id() == _sticker->get_set_id())
		{
			if (iter == stickers_.begin())
				return false;

			stickers_.erase(iter);
			break;
		}
	}

	stickers_.insert(stickers_.begin(), _sticker);

	if (max_size_ != -1)
	{
		if ((int32_t) stickers_.size() > max_size_)
			stickers_.pop_back();
	}

	return true;
}

void set::push_back_sticker(std::shared_ptr<sticker> _sticker)
{
	stickers_.push_back(_sticker);
}

void unserialize(core::coll_helper _coll)
{
	get_cache().unserialize(_coll);
}

void set_sticker_data(core::coll_helper _coll)
{
	get_cache().set_sticker_data(_coll);
}

const sets_array& get_stickers_sets()
{
	return get_cache().get_sets();
}

void clear_cache()
{
	get_cache().clear_cache();
}

std::shared_ptr<sticker> get_sticker(uint32_t _set_id, uint32_t _sticker_id)
{
	return get_cache().get_sticker(_set_id, _sticker_id);
}

cache::cache()
{

}

void cache::set_sticker_data(core::coll_helper _coll)
{
	const qint32 set_id = _coll.get_value_as_int("set_id");

	for (uint32_t i = 0; i < sets_.size(); i++)
	{
		auto &set = sets_[i];

		if (set->get_id() == set_id)
		{
			const qint32 sticker_id = _coll.get_value_as_int("sticker_id");

			const auto load_data =
				[&_coll, &set, sticker_id](const char *_id, const core::sticker_size _size)
				{
					if (!_coll->is_value_exist(_id))
					{
						return;
					}

					auto data = _coll.get_value_as_stream(_id);
					const auto data_size = data->size();

					QImage image;
					if (image.loadFromData(data->read(data_size), data_size))
					{
						set->set_sticker_image(sticker_id, _size, std::move(image));
					}
				};

			load_data("data/small", core::sticker_size::small);
			load_data("data/medium", core::sticker_size::medium);
			load_data("data/large", core::sticker_size::large);

			set->reset_flag_requested(sticker_id, core::sticker_size::small);
			set->reset_flag_requested(sticker_id, core::sticker_size::medium);
			set->reset_flag_requested(sticker_id, core::sticker_size::large);

			break;
		}
	}
}

void cache::unserialize(const core::coll_helper &_coll)
{
	core::iarray* sets = _coll.get_value_as_array("sets");
	if (!sets)
		return;

	sets_.reserve(sets->size());

	for (int32_t i = 0; i < sets->size(); i++)
	{
		core::coll_helper coll_set(sets->get_at(i)->get_as_collection(), false);

		auto inserted_set = std::make_shared<set>();

		inserted_set->unserialize(coll_set);

		sets_.push_back(inserted_set);
	}
}

const sets_array& cache::get_sets() const
{
	return sets_;
}

std::shared_ptr<sticker> cache::get_sticker(uint32_t _set_id, uint32_t _sticker_id)
{
	for (uint32_t i = 0; i < sets_.size(); i++)
	{
		if (sets_[i]->get_id() == _set_id)
		{
			auto sticks = sets_[i]->get_stickers();
			for (uint32_t j = 0; j < sticks.size(); j++)
			{
				if (sticks[j]->get_id() == _sticker_id)
					return sticks[j];
			}

			break;
		}
	}

	return nullptr;
}

void cache::clear_cache()
{
	for (uint32_t i = 0; i < sets_.size(); i++)
	{
		sets_[i]->clear_cache();
	}
}

void reset_cache()
{
    if (g_cache)
        g_cache.reset();
}

cache& get_cache()
{
    if (!g_cache)
        g_cache.reset(new cache());

	return (*g_cache);
}

UI_STICKERS_NS_END