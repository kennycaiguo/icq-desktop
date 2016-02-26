#pragma once

#include "../../types/contact.h"
#include "../../types/message.h"

namespace Ui
{
    class MainWindow;
}

namespace Logic
{
	class RecentsModel : public QAbstractListModel
	{
		Q_OBJECT

	Q_SIGNALS:
		void orderChanged();
		void updated();
        void readStateChanged(QString);
        void selectContact(QString);
        void dlgStateHandled(Data::DlgState);

	private Q_SLOTS:

		void activeDialogHide(QString);
		void contactChanged(QString);
		void dlgState(Data::DlgState);
		void sortDialogs();
		
	public:
		explicit RecentsModel(QObject *parent);

		int rowCount(const QModelIndex &parent = QModelIndex()) const;
		QVariant data(const QModelIndex &index, int role) const;
		Qt::ItemFlags flags(const QModelIndex &index) const;

		Data::DlgState getDlgState(const QString& aimId = QString(), bool fronDialog = false);

		void sendLastRead(const QString& aimId = QString());
		void markAllRead();
		void hideChat(const QString& aimId);
		void muteChat(const QString& aimId, bool mute);
		void hideAll();

		QModelIndex contactIndex(const QString& aimId);

		int totalUnreads() const;
        
        QString nextUnreadAimId();
        QString nextAimId(QString aimId);
        QString prevAimId(QString aimId);

	private:
		void pushChange(int i);
		void processChanges();

		std::vector<Data::DlgState> Dialogs_;
		QHash<QString, int> Indexes_;
		QTimer* Timer_;
	};

	RecentsModel* GetRecentsModel();
    void ResetRecentsModel();
}