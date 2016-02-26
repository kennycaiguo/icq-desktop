#include "stdafx.h"
#include "RecentsModel.h"
#include "ContactListModel.h"
#include "../../cache/avatars/AvatarStorage.h"
#include "../../main_window/MainWindow.h"

#include "../../core_dispatcher.h"
#include "../../utils/gui_coll_helper.h"
#include "../MainPage.h"
#include "../../utils/InterConnector.h"

namespace
{
	static const unsigned SORT_TIMEOUT = (build::is_debug() ? 120000 : 1000);
}

namespace Logic
{
    std::unique_ptr<RecentsModel> g_recents_model;

	RecentsModel::RecentsModel(QObject *parent)
		: QAbstractListModel(parent)
		, Timer_(new QTimer(this))
	{
		connect(GetAvatarStorage(), SIGNAL(avatarChanged(QString)), this, SLOT(contactChanged(QString)), Qt::QueuedConnection);
		connect(Ui::GetDispatcher(), SIGNAL(activeDialogHide(QString)), this, SLOT(activeDialogHide(QString)), Qt::QueuedConnection);
		connect(Ui::GetDispatcher(), SIGNAL(dlgState(Data::DlgState)), this, SLOT(dlgState(Data::DlgState)), Qt::QueuedConnection);
		connect(Logic::GetContactListModel(), SIGNAL(contactChanged(QString)), this, SLOT(contactChanged(QString)), Qt::QueuedConnection);
		Timer_->setSingleShot(true);
		connect(Timer_, SIGNAL(timeout()), this, SLOT(sortDialogs()), Qt::QueuedConnection);
	}

	int RecentsModel::rowCount(const QModelIndex &) const
	{
		return (int)Dialogs_.size();
	}

	QVariant RecentsModel::data(const QModelIndex &i, int r) const
	{
		if (!i.isValid() || (r != Qt::DisplayRole && !Testing::isAccessibleRole(r)))
			return QVariant();

		int cur = i.row();
		if ((unsigned)cur >= Dialogs_.size())
			return QVariant();

		Data::DlgState cont = Dialogs_[cur];

        if (Testing::isAccessibleRole(r))
            return cont.AimId_;

		return QVariant::fromValue(cont);
	}

	Qt::ItemFlags RecentsModel::flags(const QModelIndex &i) const
	{
		if (!i.isValid())
			return Qt::ItemIsEnabled;
		return QAbstractItemModel::flags(i) | Qt::ItemIsEnabled;
	}

	void RecentsModel::contactChanged(QString aimId)
	{
		int i = Indexes_[aimId];
		if (i != -1)
		{
			emit dataChanged(index(i), index(i));
		}
	}

	void RecentsModel::activeDialogHide(QString aimId)
	{
		Data::DlgState state;
		state.AimId_ = aimId;
		std::vector<Data::DlgState>::iterator iter = std::find(Dialogs_.begin(), Dialogs_.end(), state);
		if (iter != Dialogs_.end())
        {
            QString hideContact = iter->AimId_;
            contactChanged(hideContact);
            Indexes_[iter->AimId_] = -1;
            Dialogs_.erase(iter);
            Logic::GetContactListModel()->setCurrent("");
            if (Dialogs_.empty())
                emit Utils::InterConnector::instance().showNoRecentsYet();
		}
	}

	void RecentsModel::dlgState(Data::DlgState dlgState)
	{
		auto iter = std::find(Dialogs_.begin(), Dialogs_.end(), dlgState);
		if (iter != Dialogs_.end())
		{
            auto &existingDlgState = *iter;

            if (existingDlgState.YoursLastRead_ != dlgState.YoursLastRead_)
                emit readStateChanged(dlgState.AimId_);

            const auto existingText = existingDlgState.GetText();

			existingDlgState = dlgState;

            const auto mustRecoverText = !existingDlgState.HasText();
            if (mustRecoverText)
            {
                existingDlgState.SetText(existingText);
            }
            
			if (!Timer_->isActive())
				Timer_->start(SORT_TIMEOUT);

			int dist = (int)std::distance(Dialogs_.begin(), iter);
			emit dataChanged(index(dist), index(dist));
			emit updated();
		}
		else if (!dlgState.GetText().isEmpty())
		{
            if (Dialogs_.empty())
                emit Utils::InterConnector::instance().hideNoRecentsYet();
			Dialogs_.push_back(dlgState);
			sortDialogs();
		}

        Ui::MainWindow* w = Utils::InterConnector::instance().getMainWindow();
		if (dlgState.AimId_ == Logic::GetContactListModel()->selectedContact() && w && w->isActive())
		{
			sendLastRead(dlgState.AimId_);
		}
        emit dlgStateHandled(dlgState);
	}

	void RecentsModel::sortDialogs()
	{
		std::sort(Dialogs_.begin(), Dialogs_.end(), [](Data::DlgState first, Data::DlgState second) { return first.Time_ > second.Time_; });
		Indexes_.clear();
		int i = 0;
		for (auto iter : Dialogs_)
		{
			Indexes_[iter.AimId_] = i;
			++i;
		}
		emit dataChanged(index(0), index((int)Dialogs_.size()));
		emit orderChanged();
	}

	Data::DlgState RecentsModel::getDlgState(const QString& aimId, bool fromDialog)
	{
		QString contact = aimId.isEmpty() ? GetContactListModel()->selectedContact() : aimId;
		Data::DlgState state;
		state.AimId_ = contact;
		std::vector<Data::DlgState>::iterator iter = std::find(Dialogs_.begin(), Dialogs_.end(), state);
		if (iter != Dialogs_.end())
			state = *iter;

		if (fromDialog)
			sendLastRead(aimId);

		return state;
	}

	void RecentsModel::sendLastRead(const QString& aimId)
	{
		Data::DlgState state;
		state.AimId_ = aimId.isEmpty() ? Logic::GetContactListModel()->selectedContact() : aimId;
		std::vector<Data::DlgState>::iterator iter = std::find(Dialogs_.begin(), Dialogs_.end(), state);
		if (iter != Dialogs_.end() && (iter->UnreadCount_ != 0 || iter->YoursLastRead_ < iter->LastMsgId_))
		{
			iter->UnreadCount_ = 0;

			Ui::gui_coll_helper collection(Ui::GetDispatcher()->create_collection(), true);
			collection.set_value_as_qstring("contact", state.AimId_);
			collection.set_value_as_int64("message", iter->LastMsgId_);
			Ui::GetDispatcher()->post_message_to_core("dlg_state/set_last_read", collection.get());

			int ind = (int)std::distance(Dialogs_.begin(), iter);
			emit dataChanged(index(ind), index(ind));
			emit updated();
		}
	}

	void RecentsModel::markAllRead()
	{
		for (std::vector<Data::DlgState>::iterator iter = Dialogs_.begin(); iter != Dialogs_.end(); ++iter)
		{
			if (iter->UnreadCount_ != 0 || iter->YoursLastRead_ < iter->LastMsgId_)
			{
				Ui::gui_coll_helper collection(Ui::GetDispatcher()->create_collection(), true);
				collection.set_value_as_qstring("contact", iter->AimId_);
				collection.set_value_as_int64("message", iter->LastMsgId_);
				Ui::GetDispatcher()->post_message_to_core("dlg_state/set_last_read", collection.get());

				int ind = (int)std::distance(Dialogs_.begin(), iter);
				emit dataChanged(index(ind), index(ind));
				emit updated();
			}
		}
	}

	void RecentsModel::hideChat(const QString& aimId)
	{
		Ui::gui_coll_helper collection(Ui::GetDispatcher()->create_collection(), true);
		collection.set_value_as_qstring("contact", aimId);
		Ui::GetDispatcher()->post_message_to_core("dialogs/hide", collection.get());
	}

	void RecentsModel::muteChat(const QString& aimId, bool mute)
	{
		Ui::gui_coll_helper collection(Ui::GetDispatcher()->create_collection(), true);
		collection.set_value_as_qstring("contact", aimId);
		collection.set_value_as_bool("mute", mute);
		Ui::GetDispatcher()->post_message_to_core("dialogs/mute", collection.get());
	}

	void RecentsModel::hideAll()
	{
		for (auto iter : Dialogs_)
		{
			hideChat(iter.AimId_);
		}
	}

	QModelIndex RecentsModel::contactIndex(const QString& aimId)
	{
		int i = 0;
		for (auto iter : Dialogs_)
		{
			if (iter.AimId_ == aimId)
				return index(i);
			++i;
		}
		return QModelIndex();
	}

	int RecentsModel::totalUnreads() const
	{
		int result = 0;
		for (auto iter : Dialogs_)
		{
			if (!Logic::GetContactListModel()->isMuted(iter.AimId_))
				result += iter.UnreadCount_;
		}
		return result;
	}
    
    QString RecentsModel::nextUnreadAimId()
    {
        for (auto iter : Dialogs_)
        {
            if (!Logic::GetContactListModel()->isMuted(iter.AimId_) &&
                iter.UnreadCount_ > 0)
            {
                return iter.AimId_;
            }
        }
        
        return "";
    }
    
    QString RecentsModel::nextAimId(QString aimId)
    {
        for (int i = 0; i < Dialogs_.size(); i++)
        {
            Data::DlgState iter = Dialogs_.at(i);
            if (iter.AimId_ == aimId &&
                i < Dialogs_.size() - 1)
            {
                return Dialogs_.at(i + 1).AimId_;
            }
        }
        
        return "";
    }
    
    QString RecentsModel::prevAimId(QString aimId)
    {
        for (int i = 0; i < Dialogs_.size(); i++)
        {
            Data::DlgState iter = Dialogs_.at(i);
            if (iter.AimId_ == aimId &&
                i > 0)
            {
                return Dialogs_.at(i - 1).AimId_;
            }
        }
        
        return "";
    }

    RecentsModel* GetRecentsModel()
    {
        if (!g_recents_model)
        {
            g_recents_model.reset(new RecentsModel(0));
        }

        return g_recents_model.get();
    }

    void ResetRecentsModel()
    {
        g_recents_model.reset();
    }
}