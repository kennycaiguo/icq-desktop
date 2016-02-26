#pragma once

#include "../../../corelib/collection_helper.h"

#define UI_STICKERS_NS_BEGIN namespace Ui { namespace stickers {
#define UI_STICKERS_NS_END } }

namespace core
{
	enum class sticker_size;
}


UI_STICKERS_NS_BEGIN

class sticker
{
	typedef std::tuple<QImage, bool> image_data;

	uint32_t										id_;
	uint32_t										set_id_;
	std::map<core::sticker_size, image_data>		images_;

public:
	sticker();

	sticker(const int32_t _id, const int32_t _set_id);

	int32_t get_id() const;

	int32_t get_set_id() const;
	void set_set_id(int32_t _val);

	QImage get_image(const core::sticker_size _size) const;
	void set_image(const core::sticker_size _size, const QImage &_image);

	bool is_image_requested(const core::sticker_size _size) const;
	void set_image_requested(const core::sticker_size _size, const bool _val);

	void unserialize(core::coll_helper _coll);
	void clear_cache();

};

typedef std::shared_ptr<sticker> sticker_sptr;

typedef std::vector<sticker_sptr> stickers_array;

class set
{
	int32_t			id_;
	QString			name_;
	QPixmap			icon_;
	stickers_array	stickers_;
	int32_t			max_size_;

public:
	typedef std::shared_ptr<set> sptr;

	set(int32_t _max_size = -1);

	void set_id(int32_t _id);
	int32_t get_id() const;

	void set_name(const QString& _name);
	QString get_name() const;

	void load_icon(char* _data, int32_t _size);
	QPixmap get_icon() const;

	QImage get_sticker_image(const int32_t _index, const core::sticker_size _size);
	void set_sticker_image(const int32_t _sticker_id, const core::sticker_size _size, const QImage _image);
	void reset_flag_requested(const int32_t _sticker_id, const core::sticker_size _size);

	bool add_sticker(std::shared_ptr<sticker> _sticker);
	void push_back_sticker(std::shared_ptr<sticker> _sticker);

	const stickers_array& get_stickers() const;
	void unserialize(core::coll_helper _coll);

	void clear_cache();
};

typedef std::shared_ptr<set> set_sptr;

typedef std::vector<set_sptr> sets_array;

class cache
{
public:
	cache();

	void unserialize(const core::coll_helper &_coll);

	void set_sticker_data(core::coll_helper _coll);

	sticker_sptr get_sticker(uint32_t _set_id, uint32_t _sticker_id);

	const sets_array& get_sets() const;

	void clear_cache();

private:
	sets_array sets_;

};

void unserialize(core::coll_helper _coll);

void set_sticker_data(core::coll_helper _coll);

std::shared_ptr<sticker> get_sticker(uint32_t _set_id, uint32_t _sticker_id);

const sets_array& get_stickers_sets();

void clear_cache();
void reset_cache();

UI_STICKERS_NS_END