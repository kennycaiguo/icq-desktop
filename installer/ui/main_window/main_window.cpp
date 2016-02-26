#include "stdafx.h"
#include "main_window.h"
#include "../../utils/dpi.h"

#include "pages/start_page.h"
#include "pages/progress_page.h"
#include "pages/error_page.h"
#include "pages/accounts_page.h"

#include "../../logic/worker.h"

namespace installer
{
	namespace ui
	{
		main_window::main_window()
			:	pages_(new QStackedWidget(this)),
				start_page_(new start_page(this)),
				progress_page_(nullptr),
				error_page_(nullptr),
                accounts_page_(nullptr)
		{
			setFixedHeight(dpi::scale(400));
			setFixedWidth(dpi::scale(520));
			setWindowTitle(QT_TR_NOOP("ICQ Setup"));
			pages_->addWidget(start_page_);

			setCentralWidget(pages_);

			connect(start_page_, SIGNAL(start_install()), this, SLOT(on_start_install()));
		}
		
		main_window::~main_window()
		{
		}

		void main_window::paintEvent(QPaintEvent* _e)
		{
			QStyleOption opt;
			opt.init(this);
			QPainter p(this);
			style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

			return QMainWindow::paintEvent(_e);
		}

		void main_window::on_start_install()
		{
			assert(!progress_page_);

			if (!progress_page_)
			{
				progress_page_ = new progress_page(this);
				pages_->addWidget(progress_page_);
			}

			pages_->setCurrentWidget(progress_page_);

			start_installation();
		}

		void main_window::on_finish()
		{
			close();
		}

		void main_window::start_installation()
		{
			installer::logic::get_worker()->install();
						
			connect(logic::get_worker(), SIGNAL(finish()), this, SLOT(on_finish()));
			connect(logic::get_worker(), SIGNAL(error(installer::error)), this, SLOT(on_error(installer::error)));
            connect(logic::get_worker(), SIGNAL(select_account()), this, SLOT(on_select_account()));
		}

		void main_window::on_error(installer::error _err)
		{
			if (!error_page_)
			{
				error_page_ = new error_page(this, _err);
				pages_->addWidget(error_page_);

				connect(error_page_, SIGNAL(close()), this, SLOT(on_close()));
			}

			pages_->setCurrentWidget(error_page_);
		}

        void main_window::on_select_account()
        {
            if (!accounts_page_)
            {
                accounts_page_ = new accounts_page(this);
                pages_->addWidget(accounts_page_);

                connect(accounts_page_, SIGNAL(account_selected()), this, SLOT(on_account_selected()));
            }

            pages_->setCurrentWidget(accounts_page_);
        }

        void main_window::on_account_selected()
        {
            assert(progress_page_);

            if (!progress_page_)
                return;

            pages_->setCurrentWidget(progress_page_);

            installer::logic::get_worker()->final_install();
        }

		void main_window::on_close()
		{
			close();
		}
	}
}