#pragma once

namespace Ui
{
    class MainWindow;
    class HistoryControlPage;
    class ContactDialog;
}

namespace Utils
{
    class InterConnector : public QObject
    {
        Q_OBJECT

    public:
        enum CommonSettingsType
        {
            CommonSettingsType_General  = 0,
            CommonSettingsType_VoiceVideo,
            CommonSettingsType_Notifications,
            CommonSettingsType_About,
            CommonSettingsType_ContactUs,
        };

    Q_SIGNALS:
        void profileSettingsShow(QString uin);
        void profileSettingsBack();

        void generalSettingsShow(int type);
        void generalSettingsBack();

        void profileSettingsDoMessage(QString uin);
        void profileSettingsUnknownAdd(QString uin);
        void profileSettingsUnknownIgnore(QString uin);
        void profileSettingsUnknownSpam(QString uin);

        void makeSearchWidgetVisible(bool);
        void showIconInTaskbar(bool _show);

        void popPagesToRoot();

        void showNoContactsYetSuggestions();
        void hideNoContactsYetSuggestions();
        void showNoContactsYet();
        void hideNoContactsYet();
        void showNoRecentsYet();
        void hideNoRecentsYet();

        void stopTyping(QString, QString);

    public:
        static InterConnector& instance();
        ~InterConnector();

        void setMainWindow(Ui::MainWindow* window);
        Ui::MainWindow* getMainWindow() const;
        Ui::HistoryControlPage* getHistoryPage(const QString& aimId) const;
        Ui::ContactDialog* getContactDialog() const;

    private:
        InterConnector();

        InterConnector(InterConnector&&);
        InterConnector(const InterConnector&);
        InterConnector& operator=(const InterConnector&);

        Ui::MainWindow* MainWindow_;
    };
}
