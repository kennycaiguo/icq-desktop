#pragma once

namespace voip_proxy
{
    struct device_desc;
}

namespace Ui
{
    class TextEmojiWidget;
    
    class SettingsSlider: public QSlider
    {
    private:
        void mousePressEvent(QMouseEvent *event) override;
        
    public:
        explicit SettingsSlider(Qt::Orientation orientation, QWidget *parent = nullptr);
        ~SettingsSlider();
    };
    
    class GeneralSettingsWidget : public QStackedWidget
    {
        struct Creator;
        
        struct DeviceInfo
        {
            std::string name;
            std::string uid;
        };
        
        Q_OBJECT

    private:
        std::vector< voip_proxy::device_desc > devices_;
        
    private:
        struct VoiceAndVideoOptions
        {
            QWidget* rootWidget;

            std::vector<DeviceInfo> aCapDeviceList;
            std::vector<DeviceInfo> aPlaDeviceList;
            std::vector<DeviceInfo> vCapDeviceList;

            TextEmojiWidget* aCapSelected;
            TextEmojiWidget* aPlaSelected;
            TextEmojiWidget* vCapSelected;

            QMenu* audioCaptureDevices;
            QMenu* audioPlaybackDevices;
            QMenu* videoCaptureDevices;
        }
        _voiceAndVideo;

        QWidget* general_;
        QWidget* notifications_;
        QWidget* about_;
        QWidget* contactus_;

    public:
        GeneralSettingsWidget(QWidget* parent = nullptr);
        ~GeneralSettingsWidget();
        
        void setType(int type);
        
    private:
        void paintEvent(QPaintEvent *event) override;
        
    private Q_SLOTS:
        void onVoipDeviceListUpdated(const std::vector< voip_proxy::device_desc >& devices);
    };
}