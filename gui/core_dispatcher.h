#pragma once

#include "voip/VoipProxy.h"

#include "../corelib/core_face.h"
#include "../corelib/collection_helper.h"

#include "types/contact.h"
#include "types/message.h"
#include "types/chat.h"

#include "collection_helper_ext.h"
#include "../corelib/enumerations.h"

namespace voip_manager
{
	struct Contact;
	struct ContactEx;
}

namespace voip_proxy
{
	struct device_desc;
}

namespace core
{
	enum class file_sharing_function;
	enum class sticker_size;
    enum class group_chat_info_errors;
}

namespace launch
{
    int main(int argc, char *argv[]);
}

namespace Ui
{
	namespace stickers
	{
		enum class sticker_size;
	}

	class gui_signal : public QObject
	{
		Q_OBJECT
	public:

		Q_SIGNALS:
			void received(const QString, const qint64, core::icollection*);

	};

	class gui_connector : public gui_signal, public core::iconnector
	{
		std::atomic<int>	ref_count_;

		// ibase interface
		virtual int addref() override;
		virtual int release() override;

		// iconnector interface
		virtual void link(iconnector*) override;
		virtual void unlink() override;
        virtual void receive(const char *, int64_t, core::icollection*) override;
	public:
		gui_connector() : ref_count_(1) {}
	};

	enum class MessagesBuddiesOpt
	{
		Min,

		Requested,
		FromServer,
		DlgState,
		Pending,

		Max
	};

	typedef std::function<void(core::icollection*)> message_processed_callback;

	typedef std::shared_ptr<class core_dispatcher> core_dispatcher_sptr;

	typedef std::weak_ptr<core_dispatcher> core_dispatcher_wptr;

	class core_dispatcher
		: public QObject
		, public std::enable_shared_from_this<core_dispatcher>
	{
		Q_OBJECT

	Q_SIGNALS:
		void needLogin();
		void contactList(std::shared_ptr<Data::ContactList>, QString);
		void login_complete();
		void messageBuddies(std::shared_ptr<Data::MessageBuddies>, QString, Ui::MessagesBuddiesOpt, bool, qint64);
		void getSmsResult(int code);
		void loginResult(int code);
		void avatarLoaded(const QString&, QPixmap*, int);
		void presense(Data::Buddy*);
		void searchResult(QStringList);
		void dlgState(Data::DlgState);
		void activeDialogHide(QString);
		void guiSettings();
        void chatInfo(qint64, std::shared_ptr<Data::ChatInfo>);
        void chatInfoFailed(qint64, core::group_chat_info_errors);
        void myInfo();

        void typing(QString, QVector< QString >);
        void stopTyping(QString, QVector< QString >, int);
        void messagesReceived(QString, QVector< QString >);
        
		void contactRemoved(QString);

        void feedbackSent(bool);

		// sticker signals
		void on_stickers();
		void on_sticker(qint32 _set_id, qint32 _sticker_id);

		// remote files signals
		void imageDownloaded(qint64, QString, QPixmap);
		void fileSharingDownloadError(qint64, QString, qint32);
		void fileSharingFileDownloaded(qint64, QString, QString);
		void fileSharingFileDownloading(qint64, QString, qint64);
		void fileSharingMetadataDownloaded(qint64, QString, QString, QString, QString, QString, qint64, bool);
		void fileSharingLocalCopyCheckCompleted(qint64, bool, QString);
		void fileSharingUploadingProgress(QString, qint64);

        void signedUrl(QString);
        void speechToText(qint64, int, QString);

        void recv_permit_deny(bool);
	public Q_SLOTS:
		void received(const QString, const qint64, core::icollection*);

	public:
		static core_dispatcher_sptr create() { return core_dispatcher_sptr(new core_dispatcher); }
		virtual ~core_dispatcher();

		core::icollection* create_collection() const;
		qint64 post_message_to_core(const QString& message, core::icollection *collection, const message_processed_callback callback = nullptr);
        qint64 post_stats_to_core(core::stats::stats_event_names event_name);
		voip_proxy::VoipController& getVoipController();

		qint64 downloadSharedFile(const QString &contact, const QString &url, const QString &download_dir, const core::file_sharing_function function);
		qint64 abortSharedFileDownloading(const QString &contact, const QString &url, const qint64 downloadingSeq);

		qint64 uploadSharedFile(const QString &contact, const QString &localPath);
		qint64 abortSharedFileUploading(const QString &contact, const QString &localPath, const QString &uploadingProcessId);

		qint64 getSticker(const qint32 setId, const qint32 stickerId, const core::sticker_size size);

        qint64 downloadImagePreview(const QUrl &uri);

	private:
		typedef std::tuple<message_processed_callback, QDateTime> callback_info;

	private:
		core_dispatcher();
		bool init();
		void uninit();
		void cleanup_callbacks();
		void execute_callback(const int64_t seq, core::icollection* params);
		void fileSharingDownloadResult(const int64_t seq, core::coll_helper &params);
		void previewDownloadResult(const int64_t seq, core::coll_helper &params);
		void fileUploadingProgress(core::coll_helper &params);

        void typingEmitter(QString aimId, QVector< QString > chattersAimIds);
        Q_SLOT void stopTypingEmitter();

	private:
		core::iconnector*		core_connector_;
		core::icore_interface*	core_face_;
		voip_proxy::VoipController _voip_controller;
		core::iconnector*		gui_connector_;
		std::unordered_map<int64_t, callback_info> callbacks_;
		QDateTime last_time_callbacks_cleaned_up_;
        
        std::deque< std::pair< QString, QVector< QString > > > typingTimerQueue_;
        std::map< QString, std::map< QString, int > > typingFires_;
	};

	core_dispatcher* GetDispatcher();
}