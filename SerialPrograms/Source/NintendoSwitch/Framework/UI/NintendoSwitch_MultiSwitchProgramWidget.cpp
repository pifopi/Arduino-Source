/*  Multi-Switch Program Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QScrollArea>
#include "Common/Cpp/Containers/FixedLimitVector.tpp"
#include "Common/Qt/CollapsibleGroupBox.h"
#include "Common/Qt/Options/ConfigWidget.h"
//#include "Common/Qt/Options/BatchWidget.h"
#include "CommonFramework/Startup/NewVersionCheck.h"
#include "CommonFramework/Panels/PanelTools.h"
#include "CommonFramework/Panels/UI/PanelElements.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "NintendoSwitch/Framework/NintendoSwitch_MultiSwitchProgramOption.h"
#include "NintendoSwitch/Framework/NintendoSwitch_MultiSwitchProgramSession.h"
#include "NintendoSwitch_MultiSwitchProgramWidget.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{



MultiSwitchProgramWidget2::~MultiSwitchProgramWidget2(){
    auto ScopeCheck = m_sanitizer.check_scope();
    m_session.ProgramSession::remove_listener(*this);
    m_session.remove_listener(*this);
    delete m_actions_bar;
    delete m_stats_bar;
    delete m_options;
    delete m_system;
}


MultiSwitchProgramWidget2::MultiSwitchProgramWidget2(
    QWidget& parent,
    MultiSwitchProgramOption& option,
    PanelHolder& holder
)
    : QWidget(&parent)
    , m_holder(holder)
    , m_session(option)
    , m_sanitizer("MultiSwitchProgramWidget2")
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    const MultiSwitchProgramDescriptor& descriptor = option.descriptor();

    CollapsibleGroupBox* header = make_panel_header(
        *this,
        descriptor.display_name(),
        descriptor.doc_link(),
        descriptor.description(),
        descriptor.required_features(),
        descriptor.faster_if_tick_precise()
    );
    layout->addWidget(header);

    if (descriptor.deprecated()){
        QMessageBox box;
        box.warning(
            nullptr,
            "Deprecation Notice",
            "This program is deprecated and no longer maintained. Please consider using a newer alternative."
        );
    }


    {
        QScrollArea* scroll_outer = new QScrollArea(this);
        layout->addWidget(scroll_outer);
        scroll_outer->setWidgetResizable(true);

        QWidget* scroll_inner = new QWidget(scroll_outer);
        scroll_outer->setWidget(scroll_inner);
        QVBoxLayout* scroll_layout = new QVBoxLayout(scroll_inner);
        scroll_layout->setAlignment(Qt::AlignTop);

        m_system = new MultiSwitchSystemWidget(*this, m_session.system(), m_session.instance_id());
        scroll_layout->addWidget(m_system);

        m_options = option.options().make_QtWidget(*this);
        scroll_layout->addWidget(&m_options->widget());

        scroll_layout->addStretch(1);
    }

    m_stats_bar = new StatsBar(*this);
    m_stats_bar->set_stats("", m_session.historical_stats());
    layout->addWidget(m_stats_bar);

    m_actions_bar = new RunnablePanelActionBar(*this, m_session.current_state());
    layout->addWidget(m_actions_bar);

    connect(
        m_actions_bar, &RunnablePanelActionBar::start_clicked,
        this, [&](ProgramState state){
            std::string error;
            switch (state){
            case ProgramState::STOPPED:
                error = m_session.start_program();
                break;
            case ProgramState::RUNNING:
                error = m_session.stop_program();
                break;
            default:;
            }
            if (!error.empty()){
                this->error(error);
            }
        }
    );
    connect(
        m_actions_bar, &RunnablePanelActionBar::defaults_clicked,
        this, [&]{
            std::lock_guard<std::mutex> lg(m_session.program_lock());
            option.restore_defaults();
            m_options->update_all(false);
        }
    );

    m_session.add_listener(*this);
    m_session.ProgramSession::add_listener(*this);
}




void MultiSwitchProgramWidget2::state_change(ProgramState state){
    auto ScopeCheck = m_sanitizer.check_scope();
    QMetaObject::invokeMethod(this, [this, state]{
        m_system->update_ui(state);
        m_options->option().report_program_state(state != ProgramState::STOPPED);
//        cout << "state = " << (state != ProgramState::STOPPED) << endl;
//        if (m_option.descriptor().lock_options_while_running()){
//            m_options->widget().setEnabled(state == ProgramState::STOPPED);
//        }
        m_actions_bar->set_state(state);
        if (state == ProgramState::STOPPED){
            m_holder.on_idle();
            check_new_version();
        }else{
            m_holder.on_busy();
        }
    });
}
void MultiSwitchProgramWidget2::stats_update(const StatsTracker* current_stats, const StatsTracker* historical_stats){
    auto ScopeCheck = m_sanitizer.check_scope();
    QMetaObject::invokeMethod(this, [this, current_stats, historical_stats]{
        m_stats_bar->set_stats(
            current_stats == nullptr ? "" : current_stats->to_str(StatsTracker::DISPLAY_ON_SCREEN),
            historical_stats == nullptr ? "" : historical_stats->to_str(StatsTracker::DISPLAY_ON_SCREEN)
        );
    });
}
void MultiSwitchProgramWidget2::error(const std::string& message){
    auto ScopeCheck = m_sanitizer.check_scope();
    QMetaObject::invokeMethod(this, [message]{
        QMessageBox box;
        box.critical(nullptr, "Error", QString::fromStdString(message));
    });
}

void MultiSwitchProgramWidget2::redraw_options(){
    auto ScopeCheck = m_sanitizer.check_scope();
    QMetaObject::invokeMethod(this, [this]{
        m_options->update_all(false);
    });
}






}
}
