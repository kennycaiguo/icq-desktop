#include "stdafx.h"
#include "GeneralSettingsWidget.h"

#include "../../utils/utils.h"
#include "../../utils/translator.h"
#include "../../utils/InterConnector.h"
#include "../../utils/gui_coll_helper.h"

#include "../../core_dispatcher.h"
#include "../../gui_settings.h"

#include "../../../common.shared/version_info_constants.h"

#include "../contact_list/ContactListModel.h"
#include "../contact_list/contact_profile.h"

#include "../../controls/TextEmojiWidget.h"
#include "../../controls/FlatMenu.h"
#include "../../controls/TextEditEx.h"
#include "../../controls/LineEditEx.h"

namespace Ui
{
    namespace
    {
        struct Synchronizator
        {
            std::vector<QWidget *> widgets_;
            const char *signal_;
            const char *slot_;
            Synchronizator(QWidget *widget, const char *signal, const char *slot): signal_(signal), slot_(slot)
            {
                widgets_.push_back(widget);
            }
        };
        
        static Utils::SignalsDisconnector disconnector_;
        
        const QList<QString> &getLanguagesStrings()
        {
            static QList<QString> slist;
            if (slist.isEmpty())
            {
                slist.push_back(QT_TRANSLATE_NOOP("settings_languages", "ru"));
                slist.push_back(QT_TRANSLATE_NOOP("settings_languages", "en"));
                slist.push_back(QT_TRANSLATE_NOOP("settings_languages", "uk"));
                slist.push_back(QT_TRANSLATE_NOOP("settings_languages", "de"));
                slist.push_back(QT_TRANSLATE_NOOP("settings_languages", "pt"));
                slist.push_back(QT_TRANSLATE_NOOP("settings_languages", "cs"));
                assert(slist.size() == Utils::GetTranslator()->getLanguages().size());
            }
            return slist;
        }
        
        QString languageToString(const QString &code)
        {
            auto codes = Utils::GetTranslator()->getLanguages();
            auto strs = getLanguagesStrings();
            assert(codes.size() == strs.size() && "Languages codes != Languages strings (1)");
            auto i = codes.indexOf(code);
            if (i >= 0 && i < codes.size())
                return strs[i];
            return "";
        }
        
        QString stringToLanguage(const QString &str)
        {
            auto codes = Utils::GetTranslator()->getLanguages();
            auto strs = getLanguagesStrings();
            assert(codes.size() == strs.size() && "Languages codes != Languages strings (2)");
            auto i = strs.indexOf(str);
            if (i >= 0 && i < strs.size())
                return codes[i];
            return "";
        }
    }
    
    SettingsSlider::SettingsSlider(Qt::Orientation orientation, QWidget *parent/* = nullptr*/):
        QSlider(orientation, parent)
    {
        //
    }

    SettingsSlider::~SettingsSlider()
    {
        //
    }

    void SettingsSlider::mousePressEvent(QMouseEvent *event)
    {
        QSlider::mousePressEvent(event);
#ifndef __APPLE__
        if (event->button() == Qt::LeftButton)
        {
            if (orientation() == Qt::Vertical)
                setValue(minimum() + ((maximum()-minimum() + 1) * (height()-event->y())) / height());
            else
                setValue(minimum() + ((maximum()-minimum() + 1) * event->x()) / width());
            event->accept();
        }
#endif // __APPLE__
    }

	struct GeneralSettingsWidget::Creator
	{
        static inline void addHeader(QWidget* parent, QLayout* layout, const QString& text)
        {
            auto w = new TextEmojiWidget(parent, Utils::FontsFamily::SEGOE_UI, Utils::scale_value(24), QColor("#282828"), Utils::scale_value(46));
            w->setText(text);
            layout->addWidget(w);
            Utils::grabTouchWidget(w);
        }

        struct addSwitcherWidgets
        {
            TextEmojiWidget *text_;
            QCheckBox *check_;
        };
        static inline addSwitcherWidgets addSwitcher(std::map<std::string, Synchronizator> *collector, QWidget* parent, QLayout* layout, const QString& text, bool switched, std::function< QString(bool) > slot)
		{
            addSwitcherWidgets asws;
            
			auto f = new QWidget(parent);
			auto l = new QHBoxLayout(f);
			l->setAlignment(Qt::AlignLeft);
			l->setContentsMargins(0, 0, 0, 0);
			l->setSpacing(0);

            Utils::grabTouchWidget(f);

            auto w = new TextEmojiWidget(f, Utils::FontsFamily::SEGOE_UI, Utils::scale_value(16), QColor("#282828"), Utils::scale_value(44));
            asws.text_ = w;
            Utils::grabTouchWidget(w);
            w->setFixedWidth(Utils::scale_value(312));
            w->setText(text);
            w->set_multiline(true);
            l->addWidget(w);

            auto sp = new QWidget(parent);
            Utils::grabTouchWidget(sp);
            auto spl = new QVBoxLayout(sp);
            sp->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Preferred);
            spl->setAlignment(Qt::AlignBottom);
            spl->setContentsMargins(0, 0, 0, 0);
            spl->setSpacing(0);
            {
                auto s = new QCheckBox(sp);
                asws.check_ = s;
                s->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
                s->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
                s->setChecked(switched);
                s->setText(slot(s->isChecked()));
                if (collector)
                {
                    auto it = collector->find("switcher/switch");
                    if (it != collector->end())
                        it->second.widgets_.push_back(s);
                    else
                        collector->insert(std::make_pair("switcher/switch", Synchronizator(s, SIGNAL(pressed()), SLOT(toggle()))));
                }
                connect(s, &QCheckBox::toggled, [s, slot]()
                {
                    s->setText(slot(s->isChecked()));
                });
                spl->addWidget(s);
            }
            l->addWidget(sp);

            layout->addWidget(f);
            
            return asws;
        }

        static inline void addChooser(QWidget* parent, QLayout* layout, const QString& info, const QString& value, std::function< void(QPushButton*) > slot)
        {
            auto f = new QWidget(parent);
            auto l = new QHBoxLayout(f);
            l->setAlignment(Qt::AlignLeft);
            l->setContentsMargins(0, 0, 0, 0);
            l->setSpacing(0);

            Utils::grabTouchWidget(f);

            auto w = new TextEmojiWidget(f, Utils::FontsFamily::SEGOE_UI, Utils::scale_value(16), QColor("#282828"), Utils::scale_value(44));
            Utils::grabTouchWidget(w);
            w->setSizePolicy(QSizePolicy::Policy::Preferred, w->sizePolicy().verticalPolicy());
            w->setText(info);
            l->addWidget(w);

            auto sp = new QWidget(parent);
            auto spl = new QVBoxLayout(sp);
            Utils::grabTouchWidget(sp);
            sp->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Preferred);
            spl->setAlignment(Qt::AlignBottom);
            spl->setContentsMargins(Utils::scale_value(5), 0, 0, 0);
            spl->setSpacing(0);
            {
                auto b = new QPushButton(sp);
                b->setStyleSheet("* { color: #579e1c; }");
                b->setFlat(true);
                b->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Fixed);
                b->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
                b->setText(value);
                connect(b, &QPushButton::pressed, [b, slot]()
                {
                    slot(b);
                });
                spl->addWidget(b);
            }
            l->addWidget(sp);

            layout->addWidget(f);
        }

        struct DropperInfo { QMenu* menu; TextEmojiWidget* currentSelected; };
        static DropperInfo addDropper(QWidget* parent, QLayout* layout, const QString& info, const std::vector< QString >& values, int selected, std::function< void(QString, int, TextEmojiWidget*) > slot1, bool isCheckable, bool switched, std::function< QString(bool) > slot2)
        {
            TextEmojiWidget* w1 = nullptr;
            TextEmojiWidget* aw1 = nullptr;
            auto ap = new QWidget(parent);
            auto apl = new QHBoxLayout(ap);
            Utils::grabTouchWidget(ap);
            ap->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
            apl->setAlignment(Qt::AlignLeft);
            apl->setContentsMargins(0, 0, 0, 0);
            apl->setSpacing(Utils::scale_value(5));
            {
                w1 = new TextEmojiWidget(ap, Utils::FontsFamily::SEGOE_UI, Utils::scale_value(16), QColor("#282828"), Utils::scale_value(44));
                Utils::grabTouchWidget(w1);
                w1->setSizePolicy(QSizePolicy::Policy::Preferred, w1->sizePolicy().verticalPolicy());
                w1->setText(info);
                apl->addWidget(w1);

                aw1 = new TextEmojiWidget(ap, Utils::FontsFamily::SEGOE_UI, Utils::scale_value(16), QColor("#282828"), Utils::scale_value(44));
                Utils::grabTouchWidget(aw1);
                aw1->setSizePolicy(QSizePolicy::Policy::Preferred, aw1->sizePolicy().verticalPolicy());
                aw1->setText(" ");
                apl->addWidget(aw1);
            }
            layout->addWidget(ap);

            auto g = new QWidget(parent);
            auto gl = new QHBoxLayout(g);
            Utils::grabTouchWidget(g);
            g->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
            g->setFixedHeight(Utils::scale_value(46));
            gl->setContentsMargins(0, 0, 0, 0);
            gl->setSpacing(Utils::scale_value(32));
            gl->setAlignment(Qt::AlignLeft);

            DropperInfo di;
            di.currentSelected = NULL;
            di.menu = NULL;
            {
                auto d = new QPushButton(g);
                auto dl = new QVBoxLayout(d);
                d->setFlat(true);
                d->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
                d->setFixedSize(QSize(Utils::scale_value(280), Utils::scale_value(46)));
                d->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
                dl->setContentsMargins(0, 0, 0, 0);
                dl->setSpacing(0);
                {
                    auto w2 = new TextEmojiWidget(d, Utils::FontsFamily::SEGOE_UI, Utils::scale_value(16), QColor("#282828"), Utils::scale_value(29));
                    Utils::grabTouchWidget(w2);
                    w2->setText((!values.empty() && selected < values.size()) ? values[selected] : " ");
                    w2->set_ellipsis(true);
                    dl->addWidget(w2);
                    di.currentSelected = w2;

                    auto lp = new QWidget(d);
                    auto lpl = new QHBoxLayout(lp);
                    Utils::grabTouchWidget(lp);
                    lpl->setContentsMargins(0, 0, 0, 0);
                    lpl->setSpacing(0);
                    lpl->setAlignment(Qt::AlignBottom);
                    {
                        auto ln = new QFrame(lp);
                        Utils::grabTouchWidget(ln);
                        ln->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
                        ln->setFrameShape(QFrame::StyledPanel);
                        ln->setStyleSheet("border-width: 1; border-bottom-style: solid; border-color: #d7d7d7;");
                        lpl->addWidget(ln);
                    }
                    dl->addWidget(lp);

                    auto m = new FlatMenu(d);
                    for (auto v : values)
                        m->addAction(v);
                    connect(m, &QMenu::triggered, parent, [m, aw1, w2, slot1](QAction* a)
                    {
                        int ix = -1;
                        QList<QAction*> allActions = m->actions();
                        for (QAction* action : allActions) {
                            ix++;
                            if (a == action) {
                                w2->setText(a->text());
                                slot1(a->text(), ix, aw1);
                                break;
                            }
                        }
                    });
                    d->setMenu(m);
                    di.menu = m;
                }
                gl->addWidget(d);
            }
            if (isCheckable)
            {
                auto c = new QCheckBox(g);
                Utils::ApplyPropertyParameter(c, "ordinary", true);
                c->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
                c->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
                c->setChecked(switched);
                c->setText(slot2(c->isChecked()));
                connect(c, &QCheckBox::toggled, [c, slot2]()
                {
                    c->setText(slot2(c->isChecked()));
                });
                gl->addWidget(c);
            }
            layout->addWidget(g);

            return di;
        }

        static inline void addProgresser(QWidget* parent, QLayout* layout, const std::vector< QString >& values, int selected, std::function< void(TextEmojiWidget*, TextEmojiWidget*, int) > slot)
        {
            TextEmojiWidget* w = nullptr;
            TextEmojiWidget* aw = nullptr;
            auto ap = new QWidget(parent);
            Utils::grabTouchWidget(ap);
            auto apl = new QHBoxLayout(ap);
            ap->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
            apl->setAlignment(Qt::AlignLeft);
            apl->setContentsMargins(0, 0, 0, 0);
            apl->setSpacing(Utils::scale_value(5));
            {
                w = new TextEmojiWidget(ap, Utils::FontsFamily::SEGOE_UI, Utils::scale_value(16), QColor("#282828"), Utils::scale_value(44));
                Utils::grabTouchWidget(w);
                w->setSizePolicy(QSizePolicy::Policy::Preferred, w->sizePolicy().verticalPolicy());
                w->setText(" ");
                apl->addWidget(w);

                aw = new TextEmojiWidget(ap, Utils::FontsFamily::SEGOE_UI, Utils::scale_value(16), QColor("#282828"), Utils::scale_value(44));
                Utils::grabTouchWidget(aw);
                aw->setSizePolicy(QSizePolicy::Policy::Preferred, aw->sizePolicy().verticalPolicy());
                aw->setText(" ");
                apl->addWidget(aw);

                slot(w, aw, selected);
            }
            layout->addWidget(ap);

            auto sp = new QWidget(parent);
            Utils::grabTouchWidget(sp);
            auto spl = new QVBoxLayout(sp);
            sp->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
            sp->setFixedWidth(Utils::scale_value(280));
            spl->setAlignment(Qt::AlignBottom);
            spl->setContentsMargins(0, Utils::scale_value(12), 0, 0);
            spl->setSpacing(0);
            {
                auto p = new SettingsSlider(Qt::Orientation::Horizontal, parent);
                Utils::grabTouchWidget(p);
                p->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Preferred);
                p->setFixedWidth(Utils::scale_value(280));
                p->setFixedHeight(Utils::scale_value(24));
                p->setMinimum(0);
                p->setMaximum((int)values.size() - 1);
                p->setValue(selected);
                p->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
                p->setStyleSheet(QString("* QSlider::handle:horizontal { margin: %1 0 %2 0; }").arg(Utils::scale_value(-12)).arg(Utils::scale_value(-12)));
                connect(p, &QSlider::valueChanged, [w, aw, slot](int v)
                {
                    slot(w, aw, v);
                    w->update();
                    aw->update();
                });
                spl->addWidget(p);
            }
            layout->addWidget(sp);
        }

        static void initGeneral(QWidget* parent, std::map<std::string, Synchronizator> &collector)
        {
			auto scroll_area = new QScrollArea(parent);
			scroll_area->setWidgetResizable(true);
            Utils::grabTouchWidget(scroll_area->viewport(), true);

			auto scroll_area_content = new QWidget(scroll_area);
			scroll_area_content->setGeometry(QRect(0, 0, Utils::scale_value(800), Utils::scale_value(600)));
            Utils::grabTouchWidget(scroll_area_content);

			auto scroll_area_content_layout = new QVBoxLayout(scroll_area_content);
			scroll_area_content_layout->setSpacing(0);
			scroll_area_content_layout->setAlignment(Qt::AlignTop);
			scroll_area_content_layout->setContentsMargins(Utils::scale_value(48), 0, 0, Utils::scale_value(48));

			scroll_area->setWidget(scroll_area_content);

            auto layout = new QHBoxLayout(parent);
            layout->setSpacing(0);
            layout->setContentsMargins(0, 0, 0, 0);
			layout->addWidget(scroll_area);

            {
                addHeader(scroll_area, scroll_area_content_layout, QT_TRANSLATE_NOOP("settings_pages","General settings"));

                if (platform::is_windows())
                {
                    addSwitcher(0, scroll_area, scroll_area_content_layout, QT_TRANSLATE_NOOP("settings_pages", "Launch ICQ when system starts"), Utils::is_start_on_startup(), [](bool c) -> QString
                    {
                        if (Utils::is_start_on_startup() != c)
                            Utils::set_start_on_startup(c);
                        return (c ? QT_TRANSLATE_NOOP("settings_pages", "On") : QT_TRANSLATE_NOOP("settings_pages", "Off"));
                    });
                    addSwitcher(0, scroll_area, scroll_area_content_layout, QT_TRANSLATE_NOOP("settings_pages", "Show taskbar icon"), get_gui_settings()->get_value<bool>(settings_show_in_taskbar, true), [](bool c) -> QString
                    {
                        emit Utils::InterConnector::instance().showIconInTaskbar(c);
                        get_gui_settings()->set_value<bool>(settings_show_in_taskbar, c);
                        return (c ? QT_TRANSLATE_NOOP("settings_pages", "On") : QT_TRANSLATE_NOOP("settings_pages", "Off"));
                    });
                }

                addSwitcher(&collector, scroll_area, scroll_area_content_layout, QT_TRANSLATE_NOOP("settings_pages", "Sounds"), get_gui_settings()->get_value<bool>(settings_sounds_enabled, true), [](bool c) -> QString
                {
                    Ui::GetDispatcher()->getVoipController().setMuteSounds(!c);
                    if (get_gui_settings()->get_value<bool>(settings_sounds_enabled, true) != c)
                        get_gui_settings()->set_value<bool>(settings_sounds_enabled, c);
                    return (c ? QT_TRANSLATE_NOOP("settings_pages", "On") : QT_TRANSLATE_NOOP("settings_pages", "Off"));
                });
                /*
                addSwitcher(scroll_area, scroll_area_content_layout, QT_TRANSLATE_NOOP("settings_pages","Download files automatically"), get_gui_settings()->get_value<bool>(settings_download_files_automatically, true), [](bool c) -> QString
                {
                    if (get_gui_settings()->get_value<bool>(settings_download_files_automatically, true) != c)
                        get_gui_settings()->set_value<bool>(settings_download_files_automatically, c);
                    return (c ? QT_TRANSLATE_NOOP("settings_pages","On") : QT_TRANSLATE_NOOP("settings_pages","Off"));
                });
                */
                if (!get_gui_settings()->get_value(settings_download_directory, QString()).length())
                {
                    // workaround for core
                    get_gui_settings()->set_value(settings_download_directory, Utils::DefaultDownloadsPath());
                }
                addChooser(scroll_area, scroll_area_content_layout, QT_TRANSLATE_NOOP("settings_pages", "Save to:"), QDir::toNativeSeparators(get_gui_settings()->get_value(settings_download_directory, Utils::DefaultDownloadsPath())), [parent](QPushButton* b)
                {
#ifdef __linux__
                    QWidget* parentForDialog = 0;
#else
                    QWidget* parentForDialog = parent;
#endif //__linux__
                    auto r = QFileDialog::getExistingDirectory(parentForDialog, QT_TRANSLATE_NOOP("settings_pages", "Choose new path"), QDir::toNativeSeparators(get_gui_settings()->get_value(settings_download_directory, Utils::DefaultDownloadsPath())),
                        QFileDialog::ShowDirsOnly|QFileDialog::DontResolveSymlinks);
                    if (r.length())
                    {
                        b->setText(r);
                        get_gui_settings()->set_value(settings_download_directory, QDir::toNativeSeparators(r));
                    }
                });
                std::vector< QString > vs; vs.push_back("Enter"); vs.push_back("Ctrl+Enter"); vs.push_back("Shift+Enter");// vs.push_back("Enter+Enter");
                int key_1_to_send = get_gui_settings()->get_value<int>(settings_key1_to_send_message, Qt::NoModifier);
                int ki = ((key_1_to_send == Qt::Key_Control) * 1) + ((key_1_to_send == Qt::Key_Shift) * 2) + ((key_1_to_send == Qt::Key_Enter) * 3);
                addDropper(scroll_area, scroll_area_content_layout, QT_TRANSLATE_NOOP("settings_pages", "Send message by:"), vs, ki, [](QString v, int/* ix*/, TextEmojiWidget*)
                {
                    auto p = v.split("+");
                    if (p.length() != 2)
                        get_gui_settings()->set_value<int>(settings_key1_to_send_message, 0);
                    else if (p.length() == 2 && p[0] == "Ctrl")
                        get_gui_settings()->set_value<int>(settings_key1_to_send_message, Qt::Key_Control);
                    else if (p.length() == 2 && p[0] == "Shift")
                        get_gui_settings()->set_value<int>(settings_key1_to_send_message, Qt::Key_Shift);
                    else if (p.length() == 2)
                        get_gui_settings()->set_value<int>(settings_key1_to_send_message, Qt::Key_Enter);
                },
                false, false, [](bool) -> QString { return ""; });
            }
            {
                addHeader(scroll_area, scroll_area_content_layout, QT_TRANSLATE_NOOP("settings_pages", "Privacy"));
                addSwitcher(0, scroll_area, scroll_area_content_layout, QT_TRANSLATE_NOOP("settings_pages", "Show last message in recents"), get_gui_settings()->get_value<bool>(settings_show_last_message, true), [](bool c) -> QString
                {
                    if (get_gui_settings()->get_value<bool>(settings_show_last_message, true) != c)
                        get_gui_settings()->set_value<bool>(settings_show_last_message, c);
                    return (c ? QT_TRANSLATE_NOOP("settings_pages", "On") : QT_TRANSLATE_NOOP("settings_pages", "Off"));
                });

                addSwitcher(0, scroll_area, scroll_area_content_layout, QT_TRANSLATE_NOOP("settings_pages", "Preview images and videos"), get_gui_settings()->get_value<bool>(settings_show_video_and_images, true), [](bool c)
                {
                    if (get_gui_settings()->get_value<bool>(settings_show_video_and_images, true) != c)
                        get_gui_settings()->set_value<bool>(settings_show_video_and_images, c);
                    return (c ? QT_TRANSLATE_NOOP("settings_pages", "On") : QT_TRANSLATE_NOOP("settings_pages", "Off"));
                });
            }
            {
                auto i = (get_gui_settings()->get_value<double>(settings_scale_coefficient, Utils::get_basic_scale_coefficient()) - 1.f) / .25f; if (i > 3) i = 3;
                addHeader(scroll_area, scroll_area_content_layout, QT_TRANSLATE_NOOP("settings_pages", "Interface"));
                std::vector< QString > sc; sc.push_back("100"); sc.push_back("125"); sc.push_back("150"); sc.push_back("200");
                addProgresser(scroll_area, scroll_area_content_layout, sc, i, [sc](TextEmojiWidget* w, TextEmojiWidget* aw, int i) -> void
                {
                    static auto su = get_gui_settings()->get_value(settings_scale_coefficient, Utils::get_basic_scale_coefficient());
                    double r = sc[i].toDouble() / 100.f;
                    if (fabs(get_gui_settings()->get_value<double>(settings_scale_coefficient, Utils::get_basic_scale_coefficient()) - r) >= 0.25f)
                        get_gui_settings()->set_value<double>(settings_scale_coefficient, r);
                    w->setText(QString("%1 %2%").arg(QT_TRANSLATE_NOOP("settings_pages", "Interface scale:")).arg(sc[i]), QColor("#282828"));
                    if (fabs(su - r) >= 0.25f)
                        aw->setText(QT_TRANSLATE_NOOP("settings_pages", "(ICQ restart required)"), QColor("#579e1c"));
                    else if (fabs(Utils::get_basic_scale_coefficient() - r) < 0.05f)
                        aw->setText(QT_TRANSLATE_NOOP("settings_pages", "(Recommended)"), QColor("#282828"));
                    else
                        aw->setText(" ", QColor("#282828"));
                });

                auto ls = getLanguagesStrings();
                auto lc = languageToString(get_gui_settings()->get_value(settings_language, QString("")));
                auto li = ls.indexOf(lc);
                addDropper(scroll_area, scroll_area_content_layout, QT_TRANSLATE_NOOP("settings_pages", "Language:"), ls.toVector().toStdVector(), li, [scroll_area](QString v, int ix, TextEmojiWidget* ad)
                {
                    static auto sl = get_gui_settings()->get_value(settings_language, QString(""));
                    {
                        auto cl = stringToLanguage(v);
                        get_gui_settings()->set_value(settings_language, cl);
                        if (ad && cl != sl)
                            ad->setText(QT_TRANSLATE_NOOP("settings_pages", "(ICQ restart required)"), QColor("#579e1c"));
                        else if (ad)
                            ad->setText(" ", QColor("#282828"));
                    }
                },
                false, false, [](bool) -> QString { return ""; });
            }
		}

        static void initVoiceVideo(QWidget* parent, VoiceAndVideoOptions& voiceAndVideo, std::map<std::string, Synchronizator> &collector)
        {
            auto scroll_area = new QScrollArea(parent);
            scroll_area->setWidgetResizable(true);
            Utils::grabTouchWidget(scroll_area->viewport(), true);

            auto scroll_area_content = new QWidget(scroll_area);
            scroll_area_content->setGeometry(QRect(0, 0, Utils::scale_value(800), Utils::scale_value(600)));
            Utils::grabTouchWidget(scroll_area_content);

            auto scroll_area_content_layout = new QVBoxLayout(scroll_area_content);
            scroll_area_content_layout->setSpacing(0);
            scroll_area_content_layout->setAlignment(Qt::AlignTop);
            scroll_area_content_layout->setContentsMargins(Utils::scale_value(48), 0, 0, Utils::scale_value(48));

            scroll_area->setWidget(scroll_area_content);

            auto layout = new QHBoxLayout(parent);
            layout->setSpacing(0);
            layout->setContentsMargins(0, 0, 0, 0);
            layout->addWidget(scroll_area);

            auto __deviceChanged = [&voiceAndVideo] (const int ix, const voip_proxy::evoip_dev_types dev_type)
            {
                assert(ix >= 0);
                if (ix < 0) { return; }

                std::vector<DeviceInfo>* devList = NULL;
                QString settingsName;
                switch (dev_type) {
                case voip_proxy::kvoip_dev_type_audio_playback: { settingsName = settings_speakers; devList   = &voiceAndVideo.aPlaDeviceList; break; }
                case voip_proxy::kvoip_dev_type_audio_capture:  { settingsName = settings_microphone; devList = &voiceAndVideo.aCapDeviceList; break; }
                case voip_proxy::kvoip_dev_type_video_capture:  { settingsName = settings_webcam; devList     = &voiceAndVideo.vCapDeviceList; break; }
                case voip_proxy::kvoip_dev_type_undefined:
                default:
                    assert(!"unexpected device type");
                    return;
                };

                assert(devList);
                if (devList->empty()) { return; }

                assert(ix < (int)devList->size());
                const DeviceInfo& info = (*devList)[ix];

                voip_proxy::device_desc description;
                description.name = info.name;
                description.uid  = info.uid;
                description.dev_type = dev_type;

                Ui::GetDispatcher()->getVoipController().setActiveDevice(description);

                if (get_gui_settings()->get_value<QString>(settingsName, "") != description.uid.c_str())
                    get_gui_settings()->set_value<QString>(settingsName, description.uid.c_str());
            };

            addHeader(scroll_area, scroll_area_content_layout, QT_TRANSLATE_NOOP("settings_pages", "Voice and video"));
            {
                std::vector< QString > vs;
                const auto di = addDropper(scroll_area, scroll_area_content_layout, QT_TRANSLATE_NOOP("settings_pages", "Microphone:"), vs, 0, [__deviceChanged](QString v, int ix, TextEmojiWidget*)
                {
                    __deviceChanged(ix, voip_proxy::kvoip_dev_type_audio_capture);
                },
                /*
                true, get_gui_settings()->get_value<bool>(settings_microphone_gain, false), [](bool c) -> QString
                {
                    if (get_gui_settings()->get_value<bool>(settings_microphone_gain, false) != c)
                        get_gui_settings()->set_value<bool>(settings_microphone_gain, c);
                    return QT_TRANSLATE_NOOP("settings_pages", "Gain");
                });
                */
                false, false, [](bool) -> QString { return ""; });

                voiceAndVideo.audioCaptureDevices = di.menu;
                voiceAndVideo.aCapSelected = di.currentSelected;
            }
            {
                std::vector< QString > vs;
                const auto di = addDropper(scroll_area, scroll_area_content_layout, QT_TRANSLATE_NOOP("settings_pages", "Speakers:"), vs, 0, [__deviceChanged](QString v, int ix, TextEmojiWidget*)
                {
                    __deviceChanged(ix, voip_proxy::kvoip_dev_type_audio_playback);
                },
                false, false, [](bool) -> QString { return ""; });

                voiceAndVideo.audioPlaybackDevices = di.menu;
                voiceAndVideo.aPlaSelected = di.currentSelected;
            }
            {
                std::vector< QString > vs;
                const auto di = addDropper(scroll_area, scroll_area_content_layout, QT_TRANSLATE_NOOP("settings_pages", "Webcam:"), vs, 0, [__deviceChanged](QString v, int ix, TextEmojiWidget*)
                {
                    __deviceChanged(ix, voip_proxy::kvoip_dev_type_video_capture);
                },
                false, false, [](bool) -> QString { return ""; });

                voiceAndVideo.videoCaptureDevices = di.menu;
                voiceAndVideo.vCapSelected = di.currentSelected;
            }
        }

        static void initNotifications(QWidget* parent, std::map<std::string, Synchronizator> &collector)
        {
            auto scroll_area = new QScrollArea(parent);
            scroll_area->setWidgetResizable(true);
            Utils::grabTouchWidget(scroll_area->viewport(), true);

            auto scroll_area_content = new QWidget(scroll_area);
            scroll_area_content->setGeometry(QRect(0, 0, Utils::scale_value(800), Utils::scale_value(600)));
            Utils::grabTouchWidget(scroll_area_content);

            auto scroll_area_content_layout = new QVBoxLayout(scroll_area_content);
            scroll_area_content_layout->setSpacing(0);
            scroll_area_content_layout->setAlignment(Qt::AlignTop);
            scroll_area_content_layout->setContentsMargins(Utils::scale_value(48), 0, 0, Utils::scale_value(48));

            scroll_area->setWidget(scroll_area_content);

            auto layout = new QHBoxLayout(parent);
            layout->setSpacing(0);
            layout->setContentsMargins(0, 0, 0, 0);
            layout->addWidget(scroll_area);

            addHeader(scroll_area, scroll_area_content_layout, QT_TRANSLATE_NOOP("settings_pages", "Notifications"));
            {
                auto enableSoundsWidgets = addSwitcher(&collector, scroll_area, scroll_area_content_layout, QT_TRANSLATE_NOOP("settings_pages", "Sounds"), get_gui_settings()->get_value<bool>(settings_sounds_enabled, true), [](bool c) -> QString
                {
                    Ui::GetDispatcher()->getVoipController().setMuteSounds(!c);
                    if (get_gui_settings()->get_value<bool>(settings_sounds_enabled, true) != c)
                        get_gui_settings()->set_value<bool>(settings_sounds_enabled, c);
                    return (c ? QT_TRANSLATE_NOOP("settings_pages", "On") : QT_TRANSLATE_NOOP("settings_pages", "Off"));
                });
                auto outgoingSoundWidgets = addSwitcher(0, scroll_area, scroll_area_content_layout, QT_TRANSLATE_NOOP("settings_pages", "Outgoing messages sound"), get_gui_settings()->get_value<bool>(settings_outgoing_message_sound_enabled, false), [](bool c) -> QString
                {
                    if (get_gui_settings()->get_value(settings_outgoing_message_sound_enabled, false) != c)
                        get_gui_settings()->set_value(settings_outgoing_message_sound_enabled, c);
                    return (c ? QT_TRANSLATE_NOOP("settings_pages", "On") : QT_TRANSLATE_NOOP("settings_pages", "Off"));
                });
                disconnector_.add("sounds/outgoing", connect(enableSoundsWidgets.check_, &QCheckBox::toggled, [enableSoundsWidgets, outgoingSoundWidgets]()
                {
                    bool c = enableSoundsWidgets.check_->isChecked();
                    if (!c)
                    {
                        outgoingSoundWidgets.check_->setChecked(false);
                        outgoingSoundWidgets.check_->setEnabled(false);
                        outgoingSoundWidgets.text_->setEnabled(false);
                        if (get_gui_settings()->get_value(settings_outgoing_message_sound_enabled, false) != c)
                            get_gui_settings()->set_value(settings_outgoing_message_sound_enabled, c);
                    }
                    else
                    {
                        outgoingSoundWidgets.check_->setEnabled(true);
                        outgoingSoundWidgets.text_->setEnabled(true);
                    }
                }));
                addSwitcher(0, scroll_area, scroll_area_content_layout, QT_TRANSLATE_NOOP("settings_pages", "Notifications"), get_gui_settings()->get_value<bool>(settings_notify_new_messages, true), [](bool c) -> QString
                {
                    if (get_gui_settings()->get_value<bool>(settings_notify_new_messages, true) != c)
                        get_gui_settings()->set_value<bool>(settings_notify_new_messages, c);
                    return (c ? QT_TRANSLATE_NOOP("settings_pages", "On") : QT_TRANSLATE_NOOP("settings_pages", "Off"));
                });
                /*
                addSwitcher(0, scroll_area, scroll_area_content_layout, QT_TRANSLATE_NOOP("settings_pages", "Birthdays"), get_gui_settings()->get_value<bool>(settings_notify_birthdays, true), [](bool c) -> QString
                {
                    if (get_gui_settings()->get_value(settings_notify_birthdays, true) != c)
                        get_gui_settings()->set_value(settings_notify_birthdays, c);
                    return (c ? QT_TRANSLATE_NOOP("settings_pages", "On") : QT_TRANSLATE_NOOP("settings_pages", "Off"));
                });
                */
                /*
                addSwitcher(scroll_area, scroll_area_content_layout, QT_TRANSLATE_NOOP("settings_pages","Contacts coming online"), get_gui_settings()->get_value<bool>(settings_notify_contact_appearance, false), [](bool c) -> QString
                {
                    if (get_gui_settings()->get_value<bool>(settings_notify_contact_appearance, false) != c)
                        get_gui_settings()->set_value<bool>(settings_notify_contact_appearance, false);
                    return (c ? QT_TRANSLATE_NOOP("settings_pages","On") : QT_TRANSLATE_NOOP("settings_pages","Off"));
                });
                */

            }
        }

        static void initAbout(QWidget* parent, std::map<std::string, Synchronizator> &collector)
        {
            auto scroll_area = new QScrollArea(parent);
            scroll_area->setWidgetResizable(true);
            Utils::grabTouchWidget(scroll_area->viewport(), true);

            auto scroll_area_content = new QWidget(scroll_area);
            scroll_area_content->setGeometry(QRect(0, 0, Utils::scale_value(800), Utils::scale_value(600)));
            Utils::grabTouchWidget(scroll_area_content);

            auto scroll_area_content_layout = new QVBoxLayout(scroll_area_content);
            scroll_area_content_layout->setSpacing(0);
            scroll_area_content_layout->setAlignment(Qt::AlignTop);
            scroll_area_content_layout->setContentsMargins(Utils::scale_value(48), 0, 0, Utils::scale_value(48));

            scroll_area->setWidget(scroll_area_content);

            auto layout = new QHBoxLayout(parent);
            layout->setSpacing(0);
            layout->setContentsMargins(0, 0, 0, 0);
            layout->addWidget(scroll_area);

            addHeader(scroll_area, scroll_area_content_layout, QT_TRANSLATE_NOOP("settings_pages", "About ICQ"));
            {
                auto p = new QWidget(scroll_area);
                Utils::grabTouchWidget(p);
                auto pl = new QHBoxLayout(p);
                pl->setContentsMargins(0, Utils::scale_value(28), Utils::scale_value(48), 0);
                pl->setSpacing(Utils::scale_value(32));
                pl->setAlignment(Qt::AlignTop);
                {
                    auto sp = new QWidget(p);
                    auto spl = new QVBoxLayout(sp);
                    Utils::grabTouchWidget(sp);
                    spl->setContentsMargins(0, 0, 0, 0);
                    spl->setSpacing(0);
                    spl->setAlignment(Qt::AlignTop);
                    {
                        auto i = new QPushButton(sp);
                        i->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
                        i->setFixedSize(QSize(Utils::scale_value(80), Utils::scale_value(80)));
                        i->setFlat(true);
                        i->setStyleSheet("border-image: url(:/resources/main_window/content_logo_200.png);");
                        spl->addWidget(i);
                    }
                    pl->addWidget(sp);
                }
                {
                    auto sp = new QWidget(p);
                    auto spl = new QVBoxLayout(sp);
                    Utils::grabTouchWidget(sp);
                    spl->setContentsMargins(0, 0, 0, 0);
                    spl->setSpacing(0);
                    spl->setAlignment(Qt::AlignTop);
                    {
                        //auto t = QString("%1 (%2 %3)").arg(QT_TRANSLATE_NOOP("settings_pages","ICQ")).arg(QT_TRANSLATE_NOOP("settings_pages","build")).arg(VERSION_INFO_STR);
                        auto t = QString("%1 (%2)").arg(QT_TRANSLATE_NOOP("settings_pages", "ICQ")).arg(VERSION_INFO_STR);
                        auto w = new TextEmojiWidget(sp, Utils::FontsFamily::SEGOE_UI, Utils::scale_value(16), QColor("#282828"), -1);
                        Utils::grabTouchWidget(w);
                        w->setText(t);
                        spl->addWidget(w);
                    }
                    {
                        auto w = new TextEmojiWidget(sp, Utils::FontsFamily::SEGOE_UI, Utils::scale_value(16), QColor("#282828"), Utils::scale_value(44));
                        Utils::grabTouchWidget(w);
                        w->set_multiline(true);
                        w->setText(QT_TRANSLATE_NOOP("settings_pages", "This product includes software developed by the OpenSSL project for use in the OpenSSL Toolkit"));
                        spl->addWidget(w);
                    }
                    {
                        auto b = new QPushButton(sp);
                        auto bl = new QVBoxLayout(b);
                        b->setFlat(true);
                        b->setStyleSheet("background-color: transparent;");
                        b->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
                        b->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
                        bl->setContentsMargins(0, 0, 0, 0);
                        bl->setSpacing(0);
                        bl->setAlignment(Qt::AlignTop);
                        {
                            auto w = new TextEmojiWidget(b, Utils::FontsFamily::SEGOE_UI, Utils::scale_value(16), QColor("#579e1c"), Utils::scale_value(28));
                            Utils::grabTouchWidget(w);
                            w->setText(QT_TRANSLATE_NOOP("settings_pages", "http://openssl.org"));
                            connect(b, &QPushButton::pressed, [w]()
                            {
                                QDesktopServices::openUrl(w->text());
                            });
                            b->setFixedHeight(w->height());
                            bl->addWidget(w);
                        }
                        spl->addWidget(b);
                    }
                    {
                        auto w = new TextEmojiWidget(sp, Utils::FontsFamily::SEGOE_UI, Utils::scale_value(16), QColor("#282828"), Utils::scale_value(44));
                        Utils::grabTouchWidget(w);
                        w->set_multiline(true);
                        w->setText(QT_TRANSLATE_NOOP("settings_pages", "Copyright © 2012, the WebRTC project authors. All rights reserved."));
                        spl->addWidget(w);
                    }
					{
						auto w = new TextEmojiWidget(sp, Utils::FontsFamily::SEGOE_UI, Utils::scale_value(16), QColor("#282828"), Utils::scale_value(28));
						Utils::grabTouchWidget(w);
						w->set_multiline(true);
						w->setText(QT_TRANSLATE_NOOP("settings_pages", "Emoji provided free by Emoji One"));
						spl->addWidget(w);
					}
                    {
                        auto w = new TextEmojiWidget(sp, Utils::FontsFamily::SEGOE_UI, Utils::scale_value(16), QColor("#282828"), Utils::scale_value(44));
                        Utils::grabTouchWidget(w);
                        w->setText(QT_TRANSLATE_NOOP("settings_pages", "© ICQ LLC") + ", " + QDate::currentDate().toString("yyyy"));
                        spl->addWidget(w);
                    }
                    pl->addWidget(sp);
                }
                scroll_area_content_layout->addWidget(p);
            }
        }

        static void initContactUs(QWidget* parent, std::map<std::string, Synchronizator> &collector)
        {
            static std::map<QString, QString> filesToSend;

            auto scroll_area = new QScrollArea(parent);
            scroll_area->setWidgetResizable(true);
            Utils::grabTouchWidget(scroll_area->viewport(), true);
            
            auto scroll_area_content = new QWidget(scroll_area);
            scroll_area_content->setGeometry(QRect(0, 0, Utils::scale_value(800), Utils::scale_value(600)));
            Utils::grabTouchWidget(scroll_area_content);
            
            auto scroll_area_content_layout = new QVBoxLayout(scroll_area_content);
            scroll_area_content_layout->setSpacing(0);
            scroll_area_content_layout->setAlignment(Qt::AlignTop);
            scroll_area_content_layout->setContentsMargins(Utils::scale_value(48), 0, 0, Utils::scale_value(48));
            
            scroll_area->setWidget(scroll_area_content);
            
            auto layout = new QHBoxLayout(parent);
            layout->setSpacing(0);
            layout->setContentsMargins(0, 0, 0, 0);
            layout->addWidget(scroll_area);
            
            addHeader(scroll_area, scroll_area_content_layout, QT_TRANSLATE_NOOP("contactus_page", "Contact us"));
            {
                static quint64 filesSizeLimiter = 0;

                TextEditEx *suggestioner = nullptr;
                TextEmojiWidget *suggestionMinSizeError = nullptr;
                TextEmojiWidget *suggestionMaxSizeError = nullptr;
                QWidget *suggestionerCover = nullptr;
                QWidget *filer = nullptr;
                QWidget *filerCover = nullptr;
                TextEmojiWidget *filerTotalSizeError = nullptr;
                TextEmojiWidget *filerSizeError = nullptr;
                LineEditEx *emailer = nullptr;
                QWidget *emailerCover = nullptr;
                TextEmojiWidget *emailerError = nullptr;
                
                auto successPage = new QWidget(scroll_area);
                auto sendingPage = new QWidget(scroll_area);

                Utils::grabTouchWidget(successPage);
                Utils::grabTouchWidget(sendingPage);
                
                auto sendingPageLayout = new QVBoxLayout(sendingPage);
                sendingPageLayout->setContentsMargins(0, Utils::scale_value(28), Utils::scale_value(48), 0);
                sendingPageLayout->setSpacing(Utils::scale_value(8));
                sendingPageLayout->setAlignment(Qt::AlignTop);
                {
                    suggestioner = new TextEditEx(sendingPage, Utils::FontsFamily::SEGOE_UI, Utils::scale_value(18), QColor(0x28, 0x28, 0x28), true, false);
                    Utils::grabTouchWidget(suggestioner);
                    suggestioner->setContentsMargins(0, Utils::scale_value(24), 0, 0);
                    suggestioner->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Minimum);
                    suggestioner->setFixedWidth(Utils::scale_value(440));
                    suggestioner->setMinimumHeight(Utils::scale_value(88));
                    suggestioner->setMaximumHeight(Utils::scale_value(252));
                    suggestioner->setPlaceholderText(QT_TRANSLATE_NOOP("contactus_page","Your comments or suggestions"));
                    suggestioner->setAutoFillBackground(false);
                    suggestioner->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
                    suggestioner->setTextInteractionFlags(Qt::TextEditable | Qt::TextEditorInteraction);
                    {
                        QString sgs = "QWidget[normal=\"true\"] { font-family: \"%FONT_FAMILY%\"; font-weight: %FONT_WEIGHT%; font-size: 18dip; color: #282828; background: #ffffff; border-radius: 8dip; border-color: #d7d7d7; border-width: 1dip; border-style: solid; padding-left: 12dip; padding-right: 0dip; padding-top: 10dip; padding-bottom: 10dip; } QWidget[normal=\"false\"] { font-family: \"%FONT_FAMILY%\"; font-weight: %FONT_WEIGHT%; font-size: 18dip; color: #e30f04; background: #ffffff; border-radius: 8dip; border-color: #e30f04; border-width: 1dip; border-style: solid; padding-left: 12dip; padding-right: 0dip; padding-top: 10dip; padding-bottom: 10dip; }";
                        suggestioner->setProperty("normal", true);
                        Utils::ApplyStyle(suggestioner, sgs);
                    }
                    {
                        suggestionerCover = new QWidget(suggestioner);
                        Utils::grabTouchWidget(suggestionerCover);
                        suggestionerCover->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
                        suggestionerCover->setFixedSize(suggestioner->minimumWidth(), suggestioner->minimumHeight());
                        suggestionerCover->setStyleSheet("background-color: #7fffffff; border: none;");
                        suggestionerCover->setVisible(false);
                    }
                    {
                        disconnector_.add("suggestioner", scroll_area->connect(suggestioner->document(), &QTextDocument::contentsChanged, [=]()
                        {
                            auto nh = suggestioner->document()->size().height() + Utils::scale_value(12 + 10 + 2); // paddings + border_width*2
                            if (nh > suggestioner->maximumHeight())
                                suggestioner->setMinimumHeight(suggestioner->maximumHeight());
                            else if (nh > Utils::scale_value(88))
                                suggestioner->setMinimumHeight(nh);
                            else
                                suggestioner->setMinimumHeight(Utils::scale_value(88));
                        }));
                    }
                    sendingPageLayout->addWidget(suggestioner);
                    
                    suggestionMinSizeError = new TextEmojiWidget(suggestioner, Utils::FontsFamily::SEGOE_UI, Utils::scale_value(16), QColor("#e30f04"), Utils::scale_value(12));
                    Utils::grabTouchWidget(suggestionMinSizeError);
                    suggestionMinSizeError->setText(QT_TRANSLATE_NOOP("contactus_page", "Message is too short"));
                    suggestionMinSizeError->setVisible(false);
                    sendingPageLayout->addWidget(suggestionMinSizeError);
                    
                    suggestionMaxSizeError = new TextEmojiWidget(suggestioner, Utils::FontsFamily::SEGOE_UI, Utils::scale_value(16), QColor("#e30f04"), Utils::scale_value(12));
                    Utils::grabTouchWidget(suggestionMaxSizeError);
                    suggestionMaxSizeError->setText(QT_TRANSLATE_NOOP("contactus_page", "Message is too long"));
                    suggestionMaxSizeError->setVisible(false);
                    sendingPageLayout->addWidget(suggestionMaxSizeError);
                }
                {
                    filer = new QWidget(sendingPage);
                    Utils::grabTouchWidget(filer);
                    auto fcl = new QVBoxLayout(filer);
                    filer->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Preferred);
                    filer->setFixedWidth(Utils::scale_value(440));
                    fcl->setContentsMargins(0, Utils::scale_value(4), 0, 0);
                    fcl->setSpacing(Utils::scale_value(8));
                    {
                        filerCover = new QWidget(filer);
                        Utils::grabTouchWidget(filerCover);
                        filerCover->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
                        filerCover->setFixedSize(filer->minimumWidth(), filer->minimumHeight());
                        filerCover->setStyleSheet("background-color: #7fffffff; border: none;");
                        filerCover->setVisible(false);
                    }
                    {
                        filerSizeError = new TextEmojiWidget(filer, Utils::FontsFamily::SEGOE_UI, Utils::scale_value(16), QColor("#e30f04"), Utils::scale_value(12));
                        Utils::grabTouchWidget(filerSizeError);
                        filerSizeError->setText(QT_TRANSLATE_NOOP("contactus_page", "File size exceeds 1 MB"));
                        filerSizeError->setVisible(false);
                        fcl->addWidget(filerSizeError);

                        filerTotalSizeError = new TextEmojiWidget(filer, Utils::FontsFamily::SEGOE_UI, Utils::scale_value(16), QColor("#e30f04"), Utils::scale_value(12));
                        Utils::grabTouchWidget(filerTotalSizeError);
                        filerTotalSizeError->setText(QT_TRANSLATE_NOOP("contactus_page", "Attachments size exceeds 25 MB"));
                        filerTotalSizeError->setVisible(false);
                        fcl->addWidget(filerTotalSizeError);
                        
                        auto bsc = [=](QString fileName, QString fileSize, QString realName, qint64 realSize)
                        {
                            auto bf = new QWidget(filer);
                            auto bfl = new QHBoxLayout(bf);
                            Utils::grabTouchWidget(bf);
                            bf->setObjectName(fileName);
                            bf->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
                            bf->setFixedWidth(Utils::scale_value(440));
                            bf->setFixedHeight(Utils::scale_value(32));
                            bfl->setContentsMargins(Utils::scale_value(12), 0, Utils::scale_value(12), 0);
                            bfl->setSpacing(Utils::scale_value(12));
                            bfl->setAlignment(Qt::AlignVCenter);
                            {
                                QString bfs = "background: #ebebeb; border-radius: 8dip; border-color: #d7d7d7; border-style: none; padding-left: 15dip; padding-right: 15dip; padding-top: 12dip; padding-bottom: 10dip;";
                                Utils::ApplyStyle(bf, bfs);
                            }
                            {
                                auto fns = new QWidget(bf);
                                auto fnsl = new QHBoxLayout(fns);
                                Utils::grabTouchWidget(fns);
                                fns->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
                                fnsl->setContentsMargins(0, 0, 0, 0);
                                fnsl->setSpacing(0);
                                fnsl->setAlignment(Qt::AlignLeft);
                                {
                                    auto fn = new QLabel(fns);
                                    Utils::grabTouchWidget(fn);
                                    fn->setSizePolicy(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Preferred);
                                    fn->setText(fileName);
                                    {
                                        QString fns = "font-family: \"%FONT_FAMILY%\"; font-weight: %FONT_WEIGHT%; font-size: 16dip; color: #282828; padding: 0;";
                                        Utils::ApplyStyle(fn, fns);
                                    }
                                    fnsl->addWidget(fn);
                                    
                                    auto fs = new QLabel(fns);
                                    Utils::grabTouchWidget(fs);
                                    fs->setSizePolicy(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Preferred);
                                    fs->setText(QString(" - %1").arg(fileSize));
                                    {
                                        QString fss = "font-family: \"%FONT_FAMILY%\"; font-weight: %FONT_WEIGHT%; font-size: 16dip; color: #696969; padding: 0;";
                                        Utils::ApplyStyle(fs, fss);
                                    }
                                    fnsl->addWidget(fs);
                                }
                                bfl->addWidget(fns);
                                
                                auto d = new QPushButton(bf);
                                d->setFlat(true);
                                d->setStyleSheet("QPushButton { border-image: url(:/resources/controls_cotext_close_200_active.png); } QPushButton:hover { border-image: url(:/resources/controls_cotext_close_200_hover.png); }");
                                d->setCursor(Qt::PointingHandCursor);
                                d->setFixedSize(Utils::scale_value(12), Utils::scale_value(12));
                                disconnector_.add("filer", filer->connect(d, &QPushButton::clicked, [=]()
                                {
                                    filesToSend.erase(bf->objectName());
                                    filesSizeLimiter -= realSize;
                                    filerSizeError->setVisible(false);
                                    filerTotalSizeError->setVisible(false);
                                    bf->setVisible(false);
                                    delete bf;
                                }));
                                bfl->addWidget(d);
                            }
                            fcl->insertWidget(0, bf); // always insert at the top
                        };

                        auto sc = new QWidget(sendingPage);
                        auto scl = new QHBoxLayout(sc);
                        Utils::grabTouchWidget(sc);
                        sc->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
                        sc->setFixedWidth(Utils::scale_value(440));
                        sc->setCursor(Qt::PointingHandCursor);
                        scl->setContentsMargins(0, Utils::scale_value(0), 0, 0);
                        scl->setSpacing(Utils::scale_value(12));
                        {
                            auto attachFilesRoutine = [=]()
                            {
                                filerSizeError->setVisible(false);
                                filerTotalSizeError->setVisible(false);
#ifdef __linux__
                                QWidget *parentForDialog = nullptr;
#else
                                QWidget *parentForDialog = scroll_area;
#endif //__linux__
                                static auto dd = QDir::homePath();
                                QFileDialog d(parentForDialog);
                                d.setDirectory(dd);
                                d.setFileMode(QFileDialog::ExistingFiles);
                                d.setNameFilter(QT_TRANSLATE_NOOP("contactus_page", "Images (*.jpg *.jpeg *.png *.bmp *.gif)"));
                                QStringList fileNames;
                                if (d.exec())
                                {
                                    dd = d.directory().absolutePath();
                                    auto fls = d.selectedFiles();
                                    for (auto f: fls)
                                    {
                                        const auto rf = f;
                                        const auto rfs = QFileInfo(f).size();
                                        
                                        if ((rfs / 1024. / 1024.) > 1)
                                        {
                                            filerSizeError->setVisible(true);
                                            continue;
                                        }
                                            
                                        filesSizeLimiter += rfs;
                                        if ((filesSizeLimiter / 1024. / 1024.) > 25)
                                        {
                                            filesSizeLimiter -= rfs;
                                            filerTotalSizeError->setVisible(true);
                                            break;
                                        }
                                        
                                        double fs = rfs / 1024.f;
                                        QString fss;
                                        if (fs < 100)
                                        {
                                            fss = QString("%1 %2").arg(QString::number(fs, 'f', 2)). arg(QT_TRANSLATE_NOOP("contactus_page", "KB"));
                                        }
                                        else
                                        {
                                            fs /= 1024.;
                                            fss = QString("%1 %2").arg(QString::number(fs, 'f', 2)). arg(QT_TRANSLATE_NOOP("contactus_page", "MB"));
                                        }

                                        auto ls = f.lastIndexOf('/');
                                        if (ls < 0 || ls >= f.length())
                                            ls = f.lastIndexOf('\\');
                                        auto fsn = f.remove(0, ls + 1);

                                        if (filesToSend.find(fsn) == filesToSend.end())
                                        {
                                            //filerSizeError->setVisible(false);
                                            //filerTotalSizeError->setVisible(false);
                                            filesToSend.insert(std::make_pair(fsn, rf));
                                            bsc(fsn, fss, rf, rfs);
                                        }
                                        else
                                        {
                                            filesSizeLimiter -= rfs;
                                        }
                                    }
                                }
                            };
                            
                            auto b = new QPushButton(sc);
                            b->setFlat(true);
                            const QString addImageStyle = "QPushButton { border-image: url(:/resources/controls_cotext_addimg_100_active.png); } QPushButton:hover { border-image: url(:/resources/controls_cotext_addimg_100_hover.png); }";
                            b->setStyleSheet(Utils::ScaleStyle(addImageStyle, Utils::get_scale_coefficient()));
                            b->setFixedSize(Utils::scale_value(24), Utils::scale_value(24));
                            disconnector_.add("attach/button", scroll_area->connect(b, &QPushButton::clicked, attachFilesRoutine));
                            scl->addWidget(b);
                            
                            auto w = new TextEmojiWidget(sc, Utils::FontsFamily::SEGOE_UI, Utils::scale_value(16), QColor("#579e1c"), Utils::scale_value(16));
                            Utils::grabTouchWidget(w);
                            w->setText(QT_TRANSLATE_NOOP("contactus_page", "Add screenshot"));
                            disconnector_.add("attach/text", scroll_area->connect(w, &TextEmojiWidget::clicked, attachFilesRoutine));
                            scl->addWidget(w);
                        }
                        fcl->addWidget(sc);
                    }
                    sendingPageLayout->addWidget(filer);
                }
                {
                    emailer = new LineEditEx(sendingPage);
                    {
                        auto f = QFont(Utils::appFontFamily(Utils::FontsFamily::SEGOE_UI));
                        f.setPixelSize(Utils::scale_value(18));
                        emailer->setFont(f);
                    }
                    emailer->setContentsMargins(0, Utils::scale_value(16), 0, 0);
                    emailer->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Preferred);
                    emailer->setFixedWidth(Utils::scale_value(440));
                    emailer->setPlaceholderText(QT_TRANSLATE_NOOP("contactus_page", "Your Email"));
                    emailer->setAutoFillBackground(false);
                    emailer->setText(get_gui_settings()->get_value(settings_feedback_email, QString("")));
                    {
                        QString ms = "QWidget[normal=\"true\"] { font-family: \"%FONT_FAMILY%\"; font-weight: %FONT_WEIGHT%; font-size: 18dip; color: #282828; background: #ffffff; border-radius: 8dip; border-color: #d7d7d7; border-width: 1dip; border-style: solid; padding-left: 12dip; padding-right: 12dip; padding-top: 10dip; padding-bottom: 10dip; } QWidget[normal=\"false\"] { font-family: \"%FONT_FAMILY%\"; font-weight: %FONT_WEIGHT%; font-size: 18dip; color: #e30f04; background: #ffffff; border-radius: 8dip; border-color: #e30f04; border-width: 1dip; border-style: solid; padding-left: 12dip; padding-right: 12dip; padding-top: 10dip; padding-bottom: 10dip; }";
                        emailer->setProperty("normal", true);
                        Utils::ApplyStyle(emailer, ms);
                    }
                    {
                        emailerCover = new QWidget(emailer);
                        Utils::grabTouchWidget(emailerCover);
                        emailerCover->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
                        emailerCover->setFixedSize(emailer->minimumWidth(), emailer->minimumHeight());
                        emailerCover->setStyleSheet("background-color: #7fffffff; border: none;");
                        emailerCover->setVisible(false);
                    }
                    sendingPageLayout->addWidget(emailer);
                    
                    emailerError = new TextEmojiWidget(sendingPage, Utils::FontsFamily::SEGOE_UI, Utils::scale_value(16), QColor("#e30f04"), Utils::scale_value(12));
                    Utils::grabTouchWidget(emailerError);
                    emailerError->setText(QT_TRANSLATE_NOOP("contactus_page","Please enter a valid email address"));
                    emailerError->setVisible(false);
                    sendingPageLayout->addWidget(emailerError);
                    
                    disconnector_.add("emailer/checking", connect(&Utils::InterConnector::instance(), &Utils::InterConnector::generalSettingsShow, [=](int type)
                    {
                        if (type == Utils::InterConnector::CommonSettingsType_ContactUs && !Utils::isValidEmailAddress(emailer->text()))
                            emailer->setText("");
                        if (!emailer->property("normal").toBool())
                            Utils::ApplyPropertyParameter(emailer, "normal", true);
                        emailerError->setVisible(false);
                    }));
                }
                {
                    auto sp = new QWidget(sendingPage);
                    auto spl = new QHBoxLayout(sp);
                    Utils::grabTouchWidget(sp);
                    sp->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);
                    spl->setContentsMargins(0, Utils::scale_value(16), 0, 0);
                    spl->setSpacing(Utils::scale_value(12));
                    spl->setAlignment(Qt::AlignLeft);
                    {
                        auto sendButton = new QPushButton(sp);
                        sendButton->setFlat(true);
                        sendButton->setText(QT_TRANSLATE_NOOP("contactus_page", "Send"));
                        sendButton->setCursor(Qt::PointingHandCursor);
                        sendButton->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Fixed);
                        sendButton->setMinimumWidth(Utils::scale_value(100));
                        sendButton->setFixedHeight(Utils::scale_value(40));
                        {
                            QString bss = "QPushButton[active=\"true\"] { font-family: \"%FONT_FAMILY%\"; font-weight: %FONT_WEIGHT%; font-size: 18dip; color: #579e1c; background-color: #66cae6b3; border-style: solid; border-width: 2dip; border-color: #579e1c; margin: 0; padding: 0 42dip 0 42dip; } QPushButton:hover[active=\"true\"] { color: #ffffff; background-color: #579e1c; } QPushButton[active=\"false\"] { font-family: \"%FONT_FAMILY%\"; font-weight: %FONT_WEIGHT%; font-size: 18dip; color: #7f282828; background-color: #ebebeb; border-style: solid; border-width: 2dip; border-color: #ebebeb; margin: 0; padding: 0 42dip 0 42dip; }";
                            sendButton->setProperty("active", (suggestioner->getPlainText().length() && emailer->text().length()));
                            Utils::ApplyStyle(sendButton, bss);
                        }
                        spl->addWidget(sendButton);
                        auto updateSendButton = [=](bool state)
                        {
                            Utils::ApplyPropertyParameter(sendButton, "active", state);
                        };
                        disconnector_.add("suggestioner/sendbutton", scroll_area->connect(suggestioner->document(), &QTextDocument::contentsChanged, [=]()
                        {
                            bool state = suggestioner->getPlainText().length();
                            if (state && suggestioner->property("normal").toBool() != state)
                            {
                                Utils::ApplyPropertyParameter(suggestioner, "normal", state);
                                suggestionMinSizeError->setVisible(false);
                                suggestionMaxSizeError->setVisible(false);
                            }
                            updateSendButton(state && emailer->text().length());
                        }));
                        disconnector_.add("emailer/sendbutton", scroll_area->connect(emailer, &QLineEdit::textChanged, [=](const QString &)
                        {
                            bool state = emailer->text().length();
                            if (state && emailer->property("normal").toBool() != state)
                            {
                                Utils::ApplyPropertyParameter(emailer, "normal", state);
                                emailerError->setVisible(false);
                            }
                            updateSendButton(suggestioner->getPlainText().length() && state);
                        }));

                        auto errorOccuredSign = new TextEmojiWidget(sp, Utils::FontsFamily::SEGOE_UI, Utils::scale_value(16), QColor("#e30f04"), Utils::scale_value(16));
                        Utils::grabTouchWidget(errorOccuredSign);
                        errorOccuredSign->setText(QT_TRANSLATE_NOOP("contactus_page", "Error occured, try again later"));
                        errorOccuredSign->setVisible(false);
                        spl->addWidget(errorOccuredSign);
                        
                        auto sendSpinner = new QLabel(sp);
                        auto abm = new QMovie(":/resources/r_spiner200.gif");
                        Utils::grabTouchWidget(sendSpinner);
                        sendSpinner->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
                        sendSpinner->setContentsMargins(0, 0, 0, 0);
                        sendSpinner->setFixedSize(Utils::scale_value(40), Utils::scale_value(40));
                        abm->setScaledSize(QSize(Utils::scale_value(40), Utils::scale_value(40)));
                        abm->start();
                        sendSpinner->setMovie(abm);
                        sendSpinner->setVisible(false);
                        spl->addWidget(sendSpinner);

                        auto sendingSign = new TextEmojiWidget(sp, Utils::FontsFamily::SEGOE_UI, Utils::scale_value(16), QColor("#579e1c"), Utils::scale_value(16));
                        Utils::grabTouchWidget(sendingSign);
                        sendingSign->setText(QT_TRANSLATE_NOOP("contactus_page", "Sending..."));
                        sendingSign->setVisible(false);
                        spl->addWidget(sendingSign);
                        
                        disconnector_.add("feedback/send", scroll_area->connect(sendButton, &QPushButton::pressed, [=]()
                        {
                            get_gui_settings()->set_value(settings_feedback_email, emailer->text());
                            
                            const auto sb = suggestioner->property("normal").toBool();
                            const auto eb = emailer->property("normal").toBool();
                            if (!sb)
                                suggestioner->setProperty("normal", true);
                            if (!eb)
                                emailer->setProperty("normal", true);
                            suggestionMinSizeError->setVisible(false);
                            suggestionMaxSizeError->setVisible(false);
                            emailerError->setVisible(false);
                            filerSizeError->setVisible(false);
                            filerTotalSizeError->setVisible(false);
                            auto sg = suggestioner->getPlainText();
                            sg.remove(' ');
                            if (!sg.length())
                            {
                                suggestioner->setProperty("normal", false);
                            }
                            else if (suggestioner->getPlainText().length() < 10)
                            {
                                suggestioner->setProperty("normal", false);
                                suggestionMinSizeError->setVisible(true);
                            }
                            else if (suggestioner->getPlainText().length() > 2048)
                            {
                                suggestioner->setProperty("normal", false);
                                suggestionMaxSizeError->setVisible(true);
                            }
                            else if (!Utils::isValidEmailAddress(emailer->text()))
                            {
                                emailer->setProperty("normal", false);
                                emailerError->setVisible(true);
                            }
                            else
                            {
                                sendButton->setVisible(false);
                                errorOccuredSign->setVisible(false);
                                sendSpinner->setVisible(true);
                                sendingSign->setVisible(true);
                                
                                suggestioner->setEnabled(false);
                                filer->setEnabled(false);
                                emailer->setEnabled(false);

                                suggestionerCover->setFixedSize(suggestioner->minimumWidth(), suggestioner->minimumHeight());
                                filerCover->setFixedSize(filer->contentsRect().width(), filer->contentsRect().height());
                                emailerCover->setFixedSize(emailer->minimumWidth(), emailer->minimumHeight());

                                suggestionerCover->setVisible(true);
                                filerCover->setVisible(true);
                                emailerCover->setVisible(true);

                                filerCover->raise();
                                
                                Logic::GetContactListModel()->get_contact_profile("", [=](Logic::profile_ptr _profile)
                                {
                                    core::coll_helper col(GetDispatcher()->create_collection(), true);
                                    col.set_value_as_string("url", "http://help.mail.ru/icqdesktop-support/all");
                                    col.set_value_as_string("fb.screen_resolution", (QString("%1x%2").arg(qApp->desktop()->screenGeometry().width()).arg(qApp->desktop()->screenGeometry().height())).toStdString());
                                    col.set_value_as_string("fb.referrer", "icq");
                                    {
                                        auto icqv = QString("ICQ %1").arg(VERSION_INFO_STR);
                                        auto osv = QSysInfo::prettyProductName();
                                        if (!osv.length() || osv == "unknown")
                                            osv = QString("%1 %2 (%3 %4)").arg(QSysInfo::productType()).arg(QSysInfo::productVersion()).arg(QSysInfo::kernelType()).arg(QSysInfo::kernelVersion());
                                        auto concat = QString("%1 %2 icq:%3").arg(osv).arg(icqv).arg(_profile ? _profile->get_aimid() : "");
                                        col.set_value_as_string("fb.question.3004", concat.toStdString());
                                        col.set_value_as_string("fb.question.159", osv.toStdString());
                                        col.set_value_as_string("fb.question.178", build::is_debug() ? "beta" : "live");
                                        if (platform::is_apple())
                                            col.set_value_as_string("fb.question.3005", "OSx");
                                        else if (platform::is_windows())
                                            col.set_value_as_string("fb.question.3005", "Windows");
                                        else if (platform::is_linux())
                                            col.set_value_as_string("fb.question.3005", "Linux");
                                        else
                                            col.set_value_as_string("fb.question.3005", "Unknown");
                                    }
                                    if (_profile)
                                    {
                                        auto fn = QString("%1%2%3").arg(_profile->get_first_name()).arg(_profile->get_first_name().length() ? " " : "").arg(_profile->get_last_name());
                                        if (!fn.length())
                                        {
                                            if (_profile->get_contact_name().length())
                                                fn = _profile->get_contact_name();
                                            else if (_profile->get_displayid().length())
                                                fn = _profile->get_displayid();
                                            else if (_profile->get_friendly().length())
                                                fn = _profile->get_friendly();
                                        }
                                        col.set_value_as_string("fb.user_name", fn.toStdString());
                                    }
                                    else
                                    {
                                        col.set_value_as_string("fb.user_name", "");
                                    }
                                    col.set_value_as_string("fb.message", suggestioner->getPlainText().toStdString());
                                    col.set_value_as_string("fb.communication_email", emailer->text().toStdString());
                                    col.set_value_as_string("Lang", QLocale::system().name().toStdString());
                                    col.set_value_as_string("attachements_count", QString::number((filesToSend.size() + 1)).toStdString()); // +1 'coz we're sending log txt
                                    if (!filesToSend.empty())
                                    {
                                        core::ifptr<core::iarray> farray(col->create_array());
                                        farray->reserve((int)filesToSend.size());
                                        for (const auto &f: filesToSend)
                                        {
                                            core::ifptr<core::ivalue> val(col->create_value());
                                            val->set_as_string(f.second.toUtf8().data(), (int)f.second.toUtf8().length());
                                            farray->push_back(val.get());
                                        }
                                        col.set_value_as_array("fb.attachement", farray.get());
                                    }
                                    GetDispatcher()->post_message_to_core("feedback/send", col.get());
                                    //emit GetDispatcher()->feedbackSent(true);
                                });
                            }
                            if (sb != suggestioner->property("normal").toBool())
                                Utils::ApplyStyle(suggestioner, suggestioner->styleSheet());
                            if (eb != emailer->property("normal").toBool())
                                Utils::ApplyStyle(emailer, emailer->styleSheet());
                        }));
                        
                        disconnector_.add("feedback/sent", scroll_area->connect(GetDispatcher(), &core_dispatcher::feedbackSent, [=](bool succeeded)
                        {
                            sendButton->setVisible(true);
                            sendSpinner->setVisible(false);
                            sendingSign->setVisible(false);

                            suggestioner->setEnabled(true);
                            filer->setEnabled(true);
                            emailer->setEnabled(true);

                            suggestionerCover->setVisible(false);
                            filerCover->setVisible(false);
                            emailerCover->setVisible(false);

                            if (!succeeded)
                            {
                                errorOccuredSign->setVisible(true);
                            }
                            else
                            {
                                for (auto p: filesToSend)
                                {
                                    auto f = filer->findChild<QWidget *>(p.first);
                                    if (f)
                                    {
                                        f->setVisible(false);
                                        delete f;
                                    }
                                }
                                filesToSend.clear();
                                filesSizeLimiter = 0;

                                suggestioner->setText("");

                                sendingPage->setVisible(false);
                                successPage->setVisible(true);
                            }
                        }));

                    }
                    sendingPageLayout->addWidget(sp);
                }
                scroll_area_content_layout->addWidget(sendingPage);
                
                auto successPageLayout = new QVBoxLayout(successPage);
                successPage->setSizePolicy(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);
                successPageLayout->setContentsMargins(0, 0, 0, 0);
                successPageLayout->setSpacing(0);
                successPageLayout->setAlignment(Qt::AlignCenter);
                {
                    auto mp = new QWidget(successPage);
                    Utils::grabTouchWidget(mp);
                    auto mpl = new QVBoxLayout(mp);
                    mp->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
                    mpl->setAlignment(Qt::AlignCenter);
                    mpl->setContentsMargins(0, 0, 0, 0);
                    {
                        auto m = new QPushButton(mp);
                        m->setFlat(true);
                        m->setStyleSheet("border-image: url(:/resources/placeholders/content_illustration_good_200.png);");
                        m->setFixedSize(Utils::scale_value(120), Utils::scale_value(136));
                        mpl->addWidget(m);
                    }
                    successPageLayout->addWidget(mp);

                    auto t1p = new QWidget(successPage);
                    auto t1pl = new QVBoxLayout(t1p);
                    Utils::grabTouchWidget(t1p);
                    t1p->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
                    t1pl->setAlignment(Qt::AlignCenter);
                    t1pl->setContentsMargins(0, 0, 0, 0);
                    {
                        auto t1 = new TextEmojiWidget(t1p, Utils::FontsFamily::SEGOE_UI, Utils::scale_value(16), QColor("#282828"), Utils::scale_value(32));
                        t1->setText(QT_TRANSLATE_NOOP("contactus_page", "Thank you for contacting us!"));
                        t1->setSizePolicy(QSizePolicy::Policy::Preferred, t1->sizePolicy().verticalPolicy());
                        t1pl->addWidget(t1);
                    }
                    successPageLayout->addWidget(t1p);
                    
                    auto t2p = new QWidget(successPage);
                    auto t2pl = new QVBoxLayout(t2p);
                    Utils::grabTouchWidget(t2p);
                    t2p->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
                    t2pl->setAlignment(Qt::AlignCenter);
                    t2pl->setContentsMargins(0, 0, 0, 0);
                    {
                        auto t2 = new TextEmojiWidget(t2p, Utils::FontsFamily::SEGOE_UI, Utils::scale_value(16), QColor("#579e1c"), Utils::scale_value(32));
                        t2->setText(QT_TRANSLATE_NOOP("contactus_page", "Send another review"));
                        t2->setSizePolicy(QSizePolicy::Policy::Preferred, t2->sizePolicy().verticalPolicy());
                        t2->setCursor(Qt::PointingHandCursor);
                        disconnector_.add("feedback/repeat", scroll_area->connect(t2, &TextEmojiWidget::clicked, [=]()
                        {
                            successPage->setVisible(false);
                            sendingPage->setVisible(true);
                        }));
                        t2pl->addWidget(t2);
                    }
                    successPageLayout->addWidget(t2p);
                }
                successPage->setVisible(false);
                scroll_area_content_layout->addWidget(successPage);
            }
        }

        //
	};

    GeneralSettingsWidget::GeneralSettingsWidget(QWidget* parent):
        QStackedWidget(parent),
        general_(nullptr),
        notifications_(nullptr),
        about_(nullptr),
        contactus_(nullptr)
    {
        _voiceAndVideo.rootWidget = NULL;
        _voiceAndVideo.audioCaptureDevices = NULL;
        _voiceAndVideo.audioPlaybackDevices = NULL;
        _voiceAndVideo.videoCaptureDevices = NULL;
        _voiceAndVideo.aCapSelected = NULL;
        _voiceAndVideo.aPlaSelected = NULL;
        _voiceAndVideo.vCapSelected = NULL;

        setStyleSheet(Utils::LoadStyle(":/main_window/settings/general_settings.qss", Utils::get_scale_coefficient(), true));
        setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

        QHBoxLayout* main_layout;
        main_layout = new QHBoxLayout();
        main_layout->setSpacing(0);
        main_layout->setContentsMargins(0, 0, 0, 0);

        std::map<std::string, Synchronizator> collector;
        
        general_ = new QWidget(this);
        general_->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
        Creator::initGeneral(general_, collector);
        addWidget(general_);

        _voiceAndVideo.rootWidget = new QWidget(this);
        _voiceAndVideo.rootWidget->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
        Creator::initVoiceVideo(_voiceAndVideo.rootWidget, _voiceAndVideo, collector);
        addWidget(_voiceAndVideo.rootWidget);

        notifications_ = new QWidget(this);
        notifications_->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
        Creator::initNotifications(notifications_, collector);
        addWidget(notifications_);

        about_ = new QWidget(this);
        about_->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
        Creator::initAbout(about_, collector);
        addWidget(about_);

        contactus_ = new QWidget(this);
        contactus_->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
        Creator::initContactUs(contactus_, collector);
        addWidget(contactus_);

        setCurrentWidget(general_);
        
        for (auto cs: collector)
        {
            auto &s = cs.second;
            for (size_t si = 0, sz = s.widgets_.size(); sz > 1 && si < (sz - 1); ++si)
                for (size_t sj = si + 1; sj < sz; ++sj)
                    connect(s.widgets_[si], s.signal_, s.widgets_[sj], s.slot_),
                    connect(s.widgets_[sj], s.signal_, s.widgets_[si], s.slot_);
        }
        
        auto setActiveDevice = [] (const voip_proxy::evoip_dev_types& type) {
            QString settingsName;
            switch (type) {
            case voip_proxy::kvoip_dev_type_audio_playback: { settingsName = settings_speakers;   break; }
            case voip_proxy::kvoip_dev_type_audio_capture:  { settingsName = settings_microphone; break; }
            case voip_proxy::kvoip_dev_type_video_capture:  { settingsName = settings_webcam;     break; }
            case voip_proxy::kvoip_dev_type_undefined:
            default:
                assert(!"unexpected device type");
                return;
            };

            QString val = get_gui_settings()->get_value<QString>(settingsName, "");
            if (val != "") {
                voip_proxy::device_desc description;
                description.uid      = std::string(val.toUtf8());
                description.dev_type = type;

                Ui::GetDispatcher()->getVoipController().setActiveDevice(description);
            }
        };

        setActiveDevice(voip_proxy::kvoip_dev_type_audio_playback);
        setActiveDevice(voip_proxy::kvoip_dev_type_audio_capture);
        setActiveDevice(voip_proxy::kvoip_dev_type_video_capture);

        QObject::connect(&Ui::GetDispatcher()->getVoipController(), SIGNAL(onVoipDeviceListUpdated(const std::vector<voip_proxy::device_desc>&)), this, SLOT(onVoipDeviceListUpdated(const std::vector<voip_proxy::device_desc>&)), Qt::DirectConnection);
    }

    GeneralSettingsWidget::~GeneralSettingsWidget()
    {
        disconnector_.clean();
    }

    void GeneralSettingsWidget::setType(int type)
    {
        if (type == Utils::InterConnector::CommonSettingsType_General)
        {
            setCurrentWidget(general_);
        }
        else if (type == Utils::InterConnector::CommonSettingsType_VoiceVideo)
        {
            setCurrentWidget(_voiceAndVideo.rootWidget);
            if (devices_.empty())
                Ui::GetDispatcher()->getVoipController().setRequestSettings();
        }
        else if (type == Utils::InterConnector::CommonSettingsType_Notifications)
        {
            setCurrentWidget(notifications_);
        }
        else if (type == Utils::InterConnector::CommonSettingsType_About)
        {
            setCurrentWidget(about_);
        }
        else if (type == Utils::InterConnector::CommonSettingsType_ContactUs)
        {
            setCurrentWidget(contactus_);
        }
    }


    void GeneralSettingsWidget::onVoipDeviceListUpdated(const std::vector< voip_proxy::device_desc >& devices) {
        devices_ = devices;

        bool video_ca_upd = false;
        bool audio_pl_upd = false;
        bool audio_ca_upd = false;

        //const QString aCapDev = get_gui_settings()->get_value<QString>(settings_microphone, "");
        //const QString aPlaDev = get_gui_settings()->get_value<QString>(settings_speakers, "");
        //const QString vCapDev = get_gui_settings()->get_value<QString>(settings_webcam, "");

        using namespace voip_proxy;
        for (unsigned ix = 0; ix < devices_.size(); ix++) {
            const device_desc& desc = devices_[ix];

            QMenu* menu  = NULL;
            bool* flag_ptr = NULL;
            std::vector<DeviceInfo>* deviceList = NULL;
            TextEmojiWidget* currentSelected = NULL;
            //const QString* currentUID = NULL;

            switch (desc.dev_type) {
                case kvoip_dev_type_audio_capture:
                    menu = _voiceAndVideo.audioCaptureDevices;
                    flag_ptr = &audio_ca_upd;
                    deviceList = &_voiceAndVideo.aCapDeviceList;
                    currentSelected = _voiceAndVideo.aCapSelected;
                    //currentUID = &aCapDev;
                    break;

                case kvoip_dev_type_audio_playback:
                    menu = _voiceAndVideo.audioPlaybackDevices;
                    flag_ptr = &audio_pl_upd;
                    deviceList = &_voiceAndVideo.aPlaDeviceList;
                    currentSelected = _voiceAndVideo.aPlaSelected;
                    //currentUID = &aPlaDev;
                    break;

                case  kvoip_dev_type_video_capture:
                    menu = _voiceAndVideo.videoCaptureDevices;
                    flag_ptr = &video_ca_upd;
                    deviceList = &_voiceAndVideo.vCapDeviceList;
                    currentSelected = _voiceAndVideo.vCapSelected;
                    //currentUID = &vCapDev;
                    break;

                case kvoip_dev_type_undefined:
                default:
                    assert(false);
                    continue;
            }

            assert(menu && flag_ptr && deviceList);
            if (!menu || !flag_ptr || !deviceList) {
                continue;
            }

            if (!*flag_ptr)
            {
                *flag_ptr = true;
                menu->clear();
                deviceList->clear();
                if (currentSelected) {
                    currentSelected->setText(desc.name.c_str());
                }
            }

            DeviceInfo di;
            di.name = desc.name;
            di.uid  = desc.uid;

            menu->addAction(desc.name.c_str());
            deviceList->push_back(di);

            if ((currentSelected && desc.isActive)) {
                currentSelected->setText(desc.name.c_str());
            }
        }
    }

    void GeneralSettingsWidget::paintEvent(QPaintEvent* event)
    {
        QWidget::paintEvent(event);

        QPainter painter(this);
        painter.setBrush(QBrush(QColor("#ffffff")));
        painter.drawRect(geometry().x() - 1, geometry().y() - 1, visibleRegion().boundingRect().width() + 2, visibleRegion().boundingRect().height() + 2);
    }
    
}
