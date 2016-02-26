#include "stdafx.h"
#include "ui/main_window/main_window.h"
#include "utils/styles.h"
#include <comutil.h>
#include <comdef.h>

#include "logic/tools.h"
#include "logic/worker.h"

#ifdef _WIN32
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);
Q_IMPORT_PLUGIN(QICOPlugin);
#endif //_WIN32

using namespace installer;

const wchar_t* installer_singlton_mutex_name = L"{5686F5E0-121F-449B-ABB7-B01021D7DE65}";

int main(int _argc, char* _argv[])
{
	int res = 0;

#ifdef _WIN32
    CHandle mutex(::CreateSemaphore(NULL, 0, 1, installer_singlton_mutex_name));
    if (ERROR_ALREADY_EXISTS == ::GetLastError())
        return res;
#endif //_WIN32


	QApplication app(_argc, _argv);
			
	logic::get_translator()->init();

	std::unique_ptr<ui::main_window> wnd;

	{
		QStringList arguments = app.arguments();

		logic::install_config config;

		for (auto iter = arguments.cbegin(); iter != arguments.cend(); ++iter)
		{
			if (*iter == "-uninstalltmp")
			{
				config.set_uninstalltmp(true);
			}
			else if (*iter == "-uninstall")
			{
				config.set_uninstall(true);
			}
			else if (*iter == "-silent")
			{
				config.set_silent(true);
			}
			else if (*iter == "-update")
			{
				config.set_update(true);
			}
            else if (*iter == update_final_command)
            {
                config.set_update_final(true);
            }
            else if (*iter == delete_updates_command)
            {
                config.set_delete_updates(true);
            }
		}

		set_install_config(config);

		if (logic::get_install_config().is_uninstalltmp())
		{
			logic::get_worker()->uninstalltmp();
		}
        else if (logic::get_install_config().is_delete_updates())
        {
            logic::get_worker()->clear_updates();
        }
		else if (logic::get_install_config().is_uninstall())
		{
			QObject::connect(logic::get_worker(), &logic::worker::finish, []()
			{
				QApplication::exit();
			});

			QObject::connect(logic::get_worker(), &logic::worker::error, [](installer::error _err)
			{
				QApplication::exit();
			});

			logic::get_worker()->uninstall();

			res = app.exec();						
		}
		else if (logic::get_install_config().is_update())
		{
			QObject::connect(logic::get_worker(), &logic::worker::finish, []()
			{
				QApplication::exit();
			});

			QObject::connect(logic::get_worker(), &logic::worker::error, [](installer::error _err)
			{
				QApplication::exit();
			});

			logic::get_worker()->update();

			res = app.exec();						
		}
        else if (logic::get_install_config().is_update_final())
        {
            CHandle mutex(::CreateSemaphore(NULL, 0, 1, updater_singlton_mutex_name.c_str()));
            if (ERROR_ALREADY_EXISTS == ::GetLastError())
                return true;

            QObject::connect(logic::get_worker(), &logic::worker::finish, []()
            {
                QApplication::exit();
            });

            QObject::connect(logic::get_worker(), &logic::worker::error, [](installer::error _err)
            {
                QApplication::exit();
            });

            logic::get_worker()->update_final(mutex);

            res = app.exec();						
        }
		else
		{
			app.setStyleSheet(ui::styles::load_style(":/styles/styles.qss"));

			wnd.reset(new ui::main_window());

			QIcon icon(":/images/appicon.ico");
			wnd->setWindowIcon(icon);

			wnd->show();

			res = app.exec();
		}
		
	}
		
	return res;
}