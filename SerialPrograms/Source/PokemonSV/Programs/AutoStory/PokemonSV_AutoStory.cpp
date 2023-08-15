/*  AutoStory
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <atomic>
#include <sstream>
#include "CommonFramework/Exceptions/FatalProgramException.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Tools/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_ScalarButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/Inference/Battles/PokemonSV_NormalBattleMenus.h"
#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Inference/PokemonSV_MainMenuDetector.h"
#include "PokemonSV/Programs/PokemonSV_Navigation.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/PokemonSV_Battles.h"
#include "PokemonSV_AutoStory.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;

AutoStory_Descriptor::AutoStory_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:AutoStory",
        STRING_POKEMON + " SV", "Auto Story",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/AutoStory.md",
        "Progress through the mainstory of SV.",
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}
struct AutoStory_Descriptor::Stats : public StatsTracker{
    Stats()
        : m_checkpoint(m_stats["Checkpoint"])
        , m_segment(m_stats["Segment"])
        , m_reset(m_stats["Reset"])
    {
        m_display_order.emplace_back("Checkpoint");
        m_display_order.emplace_back("Segment");
        m_display_order.emplace_back("Reset");
    }
    std::atomic<uint64_t>& m_checkpoint;
    std::atomic<uint64_t>& m_segment;
    std::atomic<uint64_t>& m_reset;
};
std::unique_ptr<StatsTracker> AutoStory_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



AutoStory::~AutoStory(){
    STARTPOINT.remove_listener(*this);
    ENDPOINT.remove_listener(*this);
}

AutoStory::AutoStory()
    : STARTPOINT(
        "<b>Start Point:</b><br>Program will start with this segment.",
        {
            {StartPoint::INTRO_CUTSCENE,        "00_gameintro",         "00: Intro Cutscene"},
            {StartPoint::LEAVE_HOUSE,           "01_leavehouse",        "01: Leave House"},
            {StartPoint::PICK_STARTER,          "02_pickstarter",       "02: Pick Starter"},
            {StartPoint::NEMONA_FIRST_BATTLE,   "03_nemonabattle1",     "03: First Nemona Battle"},
            {StartPoint::CATCH_TUTORIAL,        "04_catchtutorial",     "04: Catch Tutorial"},
            {StartPoint::LEGENDARY_RESCUE,      "05_legendaryrescue",   "05: Rescue Legendary"},
            {StartPoint::ARVEN_FIRST_BATTLE,    "06_arvenbattle1",      "06: First Arven Battle"},
            {StartPoint::GOTO_LOS_PLATOS,       "07_gotolosplatos",     "07: Go to Los Platos"},
        },
        LockWhileRunning::LOCKED,
        StartPoint::LEAVE_HOUSE
    )
    , ENDPOINT(
        "<b>End Point:</b><br>Program will stop after completing this segment.",
        {
            {EndPoint::LEAVE_HOUSE,             "01_leavehouse",        "01: Leave House"},
            {EndPoint::PICK_STARTER,            "02_pickstarter",       "02: Pick Starter"},
            {EndPoint::NEMONA_FIRST_BATTLE,     "03_nemonabattle1",     "03: First Nemona Battle"},
            {EndPoint::CATCH_TUTORIAL,          "04_catchtutorial",     "04: Catch Tutorial"},
            {EndPoint::LEGENDARY_RESCUE,        "05_legendaryrescue",   "05: Rescue Legendary"},
            {EndPoint::ARVEN_FIRST_BATTLE,      "06_arvenbattle1",      "06: First Arven Battle"},
            {EndPoint::GOTO_LOS_PLATOS,         "07_gotolosplatos",     "07: Go to Los Platos"},
        },
        LockWhileRunning::LOCKED,
        EndPoint::PICK_STARTER
    )
    , STARTERCHOICE(
        "<b>Starter to Choose:</b><br>Select the starter you want to pick.",
        {
            {StarterChoice::SPRIGATITO,     "sprigatito",   "Sprigatito (Grass)"},
            {StarterChoice::FUECOCO,        "fuecoco",      "Fuecoco (Fire)"},
            {StarterChoice::QUAXLY,         "quaxly",       "Quaxly (Water)"},
        },
        LockWhileRunning::LOCKED,
        StarterChoice::FUECOCO
    )
    , GO_HOME_WHEN_DONE(true)
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(30))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(STARTPOINT);
    PA_ADD_OPTION(ENDPOINT);
    PA_ADD_OPTION(STARTERCHOICE);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);

    AutoStory::value_changed();

    STARTPOINT.add_listener(*this);
    ENDPOINT.add_listener(*this);
}

void AutoStory::value_changed(){
    ConfigOptionState state = ((STARTPOINT == StartPoint::INTRO_CUTSCENE) || (STARTPOINT == StartPoint::LEAVE_HOUSE) || (STARTPOINT == StartPoint::PICK_STARTER))
            && (ENDPOINT != EndPoint::LEAVE_HOUSE)
        ? ConfigOptionState::ENABLED
        : ConfigOptionState::HIDDEN;
    STARTERCHOICE.set_visibility(state);
}

bool dialog_clearer(SingleSwitchProgramEnvironment& env, BotBaseContext& context,
    bool stop_at_prompt = false,
    bool stop_at_overworld = true,
    bool error_on_timeout = true,
    uint16_t seconds_to_timeout = 60
){
    context.wait_for_all_requests();
    env.console.log("Clearing dialog...");

    while (true){
        PromptDialogWatcher dialog_prompt(COLOR_YELLOW);
        OverworldWatcher overworld(COLOR_CYAN);
        DialogBoxWatcher dialog_normal(COLOR_RED, true);
        WhiteButtonWatcher whitebutton(COLOR_GREEN, WhiteButton::ButtonA, {0.650, 0.650, 0.140, 0.240});
        context.wait_for_all_requests();

        int ret = wait_until(
            env.console, context,
            std::chrono::seconds(seconds_to_timeout),
            {dialog_prompt, overworld, dialog_normal, whitebutton}
        );
        context.wait_for(std::chrono::milliseconds(100));

        switch (ret){
        case 0: // prompt dialog
            env.console.log("Detected prompt.");
            if (stop_at_prompt){
                pbf_wait(context, 1 * TICKS_PER_SECOND);
                return true;
            }
            pbf_press_button(context, BUTTON_A, 20, 105);
            continue;
        case 1: // overworld
            env.console.log("Detected overworld.");
            if (stop_at_overworld){
                pbf_wait(context, 1 * TICKS_PER_SECOND);
                return true;
            }
            continue;
        case 2: // normal dialog
            env.console.log("Detected normal dialog.");
            pbf_press_button(context, BUTTON_A, 20, 150);
            continue;
        case 3: // white A button
            env.console.log("Detected white A button.");
            pbf_press_button(context, BUTTON_A, 20, 1 * TICKS_PER_SECOND);
            continue;
        default:
            if (error_on_timeout){
                throw OperationFailedException(
                    ErrorReport::SEND_ERROR_REPORT, env.console,
                    "dialog_clearer(): Timed out, no recognized state found.",
                    true
                );
            } else {
                env.console.log("dialog_clearer(): No more dialog detected.");
                return false;
            }
        }
    }
}

bool move_till_dialog(SingleSwitchProgramEnvironment& env, BotBaseContext& context,
    uint8_t x, uint8_t y, uint16_t seconds_run,
    bool error_on_timeout = true,
    bool mash_a_moving = false
){
    context.wait_for_all_requests();
    env.console.log("Starting new dialog search...");
    DialogBoxWatcher dialog(COLOR_RED, true);
    context.wait_for_all_requests();

    int ret = run_until(
        env.console, context,
        [x, y, seconds_run, mash_a_moving](BotBaseContext& context){
            ssf_press_left_joystick(context, x, y, 0, seconds_run * TICKS_PER_SECOND);
            if (mash_a_moving){
                for (int i = 0; i < seconds_run; i++){
                    pbf_press_button(context, BUTTON_A, 20, 105);
                }
            } else {
                pbf_wait(context, seconds_run * TICKS_PER_SECOND);
            }
        },
        {dialog}
    );
    if (ret < 0){
        context.wait_for_all_requests();
        if (error_on_timeout){
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, env.console,
                "move_till_dialog(): Timed out, no recognized state found.",
                true
            );
        } else {
            context.wait_for_all_requests();
            env.console.log("move_till_dialog(): Timed out, no recognized state found.");
            return false;
        }
    }
    context.wait_for_all_requests();
    env.console.log("Detected dialog.");
    return true;
}

void mash_button_till_overworld(SingleSwitchProgramEnvironment& env, BotBaseContext& context, uint16_t button = BUTTON_A, uint16_t seconds_run = 360){
    OverworldWatcher overworld(COLOR_CYAN);
    context.wait_for_all_requests();

    int ret = run_until(
        env.console, context,
        [button, seconds_run](BotBaseContext& context){
            ssf_mash1_button(context, button, seconds_run * TICKS_PER_SECOND);
            pbf_wait(context, seconds_run * TICKS_PER_SECOND);
        },
        {overworld}
    );

    if (ret < 0){
        throw OperationFailedException(
            ErrorReport::SEND_ERROR_REPORT, env.console,
            "mash_button_till_overworld(): Timed out, no recognized state found.",
            true
        );
    }
}

void run_battle(SingleSwitchProgramEnvironment& env, BotBaseContext& context, bool stop_on_dialog = false){
    while (true){
        OverworldWatcher overworld(COLOR_CYAN);
        DialogBoxWatcher dialog(COLOR_RED, true);
        context.wait_for_all_requests();

        int ret = run_until(
            env.console, context,
            [&env](BotBaseContext& context){
                NormalBattleMenuWatcher battle(COLOR_YELLOW);
                int ret = wait_until(
                    env.console, context,
                    std::chrono::seconds(90),
                    {battle}
                );
                if (ret == 0) {
                    env.console.log("Detected battle.");
                    pbf_mash_button(context, BUTTON_A, 2 * TICKS_PER_SECOND);
                    context.wait_for_all_requests();
                }
            },
            {overworld, dialog}
        );

        switch (ret){
        case 0: // overworld
            env.console.log("Detected overworld, battle over.");
            return;
        case 1: // dialog
            env.console.log("Detected dialog.");
            if (stop_on_dialog){
                return;
            } else {
                pbf_press_button(context, BUTTON_A, 20, 105);
                continue;
            }
        }
    }
}

void realign_player(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    env.console.log("Realign player direction.");
    open_map_from_overworld(env.program_info(), env.console, context);
    leave_phone_to_overworld(env.program_info(), env.console, context);
    pbf_press_button(context, BUTTON_L, 20, 105);
}

bool move_toward_destination(
    SingleSwitchProgramEnvironment& env, BotBaseContext& context,
    uint8_t x = 128, uint8_t y = 0, uint16_t seconds_run = 180,
    bool error_on_timeout = true,
    bool heal_after_battle = true,
    uint16_t seconds_realign = 30
){
    WallClock start = current_time();
    while (true){
        if (current_time() - start > std::chrono::seconds(seconds_run)){
            if (error_on_timeout){
                throw OperationFailedException(
                    ErrorReport::SEND_ERROR_REPORT, env.console,
                    "move_toward_destination(): Timed out, failed to find dialog.",
                    true
                );
            } else {
                return false;
            }

        }

        NormalBattleMenuWatcher battle(COLOR_YELLOW);
        DialogBoxWatcher dialog(COLOR_RED, true);

        context.wait_for_all_requests();

        int ret = run_until(
            env.console, context,
            [x, y, seconds_realign, &env](BotBaseContext& context){
                for (int i = 0; i < 20; i++){
                    ssf_press_left_joystick(context, x, y, 0, seconds_realign * TICKS_PER_SECOND);
                    pbf_wait(context, seconds_realign * TICKS_PER_SECOND);
                    realign_player(env, context);
                }
            },
            {battle, dialog}
        );

        switch (ret){
        case 0: // battle
            run_battle(env, context);
            if (heal_after_battle){
                auto_heal_from_menu_or_overworld(env.program_info(), env.console, context, 0, true);
            }
            realign_player(env, context);
            continue;
        case 1: // dialog
            env.console.log("Detected dialog.");
            return true;
        }
    }
}

void reset_game(SingleSwitchProgramEnvironment& env, BotBaseContext& context, const std::string& error_msg){
    try{
        pbf_press_button(context, BUTTON_HOME, 20, GameSettings::instance().GAME_TO_HOME_DELAY);
        context.wait_for_all_requests();
        reset_game_from_home(env.program_info(), env.console, context, 5 * TICKS_PER_SECOND);
    }catch (OperationFailedException& e){
        // To be safe: avoid doing anything outside of game on Switch,
        // make game resetting non error recoverable
        throw FatalProgramException(std::move(e));
    }
}

void config_option(SingleSwitchProgramEnvironment& env, BotBaseContext& context,
    int change_option_value
){
    for (int i = 0; i < change_option_value; i++){
        pbf_press_dpad(context, DPAD_RIGHT, 15, 20);
    }
    pbf_press_dpad(context, DPAD_DOWN,  15, 20);
}

void enter_menu_from_overworld(SingleSwitchProgramEnvironment& env, BotBaseContext& context,
    int menu_index,
    MenuSide side = MenuSide::RIGHT,
    bool has_minimap = true
){
    if (!has_minimap){
        pbf_press_button(context, BUTTON_X, 20, 105);
    }

    WallClock start = current_time();
    bool success = false;

    while (true){
        if (current_time() - start > std::chrono::minutes(3)){
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, env.console,
                "enter_menu_from_overworld(): Failed to enter specified menu after 3 minutes.",
                true
            );
        }

        OverworldWatcher overworld(COLOR_CYAN);
        MainMenuWatcher main_menu(COLOR_RED);
        context.wait_for_all_requests();

        int ret = run_until(
            env.console, context,
            [](BotBaseContext& context){
                for (int i = 0; i < 10; i++){
                    pbf_wait(context, 1 * TICKS_PER_SECOND);
                    pbf_press_button(context, BUTTON_A, 20, 105);
                }
            },
            {overworld, main_menu}
        );
        context.wait_for(std::chrono::milliseconds(100));

        const bool fast_mode = false;
        switch (ret){
        case 0:
            env.console.log("Detected overworld.");
            pbf_press_button(context, BUTTON_X, 20, 105);
            continue;
        case 1:
            env.console.log("Detected main menu.");
            success = main_menu.move_cursor(env.program_info(), env.console, context, side, menu_index, fast_mode);
            if (success == false){
                throw OperationFailedException(
                    ErrorReport::SEND_ERROR_REPORT, env.console,
                    "enter_menu_from_overworld(): Cannot move menu cursor to specified menu.",
                    true
                );
            }
            pbf_press_button(context, BUTTON_A, 20, 105);
            return;
        default:
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, env.console,
                "enter_menu_from_overworld(): No recognized state after 30 seconds.",
                true
            );
        }
    }
}

void set_map_marker(SingleSwitchProgramEnvironment& env, BotBaseContext& context, uint8_t map_x, uint8_t map_y, uint8_t cursor_move_duration){
    open_map_from_overworld(env.program_info(), env.console, context);
    pbf_press_button(context, BUTTON_ZR, 20, 105);
    pbf_move_left_joystick(context, map_x, map_y, cursor_move_duration, 1 * TICKS_PER_SECOND);
    pbf_press_button(context, BUTTON_A, 20, 105);
    pbf_press_button(context, BUTTON_A, 20, 105);
    leave_phone_to_overworld(env.program_info(), env.console, context);
}

void AutoStory::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    assert_16_9_720p_min(env.logger(), env.console);
    AutoStory_Descriptor::Stats& stats = env.current_stats<AutoStory_Descriptor::Stats>();

    // Connect controller
    pbf_press_button(context, BUTTON_L, 20, 20);

    switch (STARTPOINT){
    case StartPoint::INTRO_CUTSCENE:
        context.wait_for_all_requests();
        env.console.log("Start Segment 00: Intro Cutscene", COLOR_ORANGE);
        env.console.overlay().add_log("Start Segment 00: Intro Cutscene", COLOR_ORANGE);

        //TODO: Automatic character settings

        // Mash A through intro cutscene
        // TODO: Stand up icon detection
        pbf_mash_button(context, BUTTON_A, 225 * TICKS_PER_SECOND);
        pbf_wait(context, 100);

        context.wait_for_all_requests();
        env.console.log("End Segment 00: Intro Cutscene", COLOR_GREEN);
        env.console.overlay().add_log("End Segment 00: Intro Cutscene", COLOR_GREEN);
        stats.m_segment++;
        env.update_stats();
        pbf_wait(context, 1 * TICKS_PER_SECOND);
    case StartPoint::LEAVE_HOUSE:
        context.wait_for_all_requests();
        env.console.log("Start Segment 01: Leave House", COLOR_ORANGE);
        env.console.overlay().add_log("Start Segment 01: Leave House", COLOR_ORANGE);

        // Stand up from chair and move left
        pbf_move_left_joystick(context, 128, 255, 3 * TICKS_PER_SECOND, 5 * TICKS_PER_SECOND);
        pbf_move_left_joystick(context,   0, 128, 6 * TICKS_PER_SECOND, 100);

        // Open main menu and enter options menu
        enter_menu_from_overworld(env, context, 0, MenuSide::RIGHT, false);
        config_option(env, context, 1); // Text Speed: Fast
        config_option(env, context, 1); // Skip Move Learning: On
        config_option(env, context, 1); // Send to Boxes: Automatic
        config_option(env, context, 1); // Give Nicknames: Off
        config_option(env, context, 0); // Vertical Camera Controls: Regular
        config_option(env, context, 0); // Horiztontal Camera Controls: Regular
        config_option(env, context, 1); // Autosave: Off
        config_option(env, context, 1); // Show Nicknames: Don't show
        config_option(env, context, 1); // Skip Cutscenes: On
        config_option(env, context, 0); // Background Music: 10
        config_option(env, context, 0); // Sound Effects: 10
        config_option(env, context, 0); // Pokemon Cries: 10
        config_option(env, context, 1); // Controller Rumble: Off
        config_option(env, context, 1); // Helpeing Functions: Off
        pbf_press_button(context, BUTTON_A, 20, 2 * TICKS_PER_SECOND);
        dialog_clearer(env, context, false, false, false, 5);
        pbf_press_button(context, BUTTON_B, 20, 2 * TICKS_PER_SECOND);

        context.wait_for_all_requests();
        env.console.log("Go downstairs, get stopped by Skwovet");
        env.console.overlay().add_log("Go downstairs, get stopped by Skwovet", COLOR_WHITE);
        pbf_move_left_joystick(context, 128,   0, 3 * TICKS_PER_SECOND, 20);
        pbf_move_left_joystick(context,   0, 128, 4 * TICKS_PER_SECOND, 20);
        pbf_move_left_joystick(context, 128, 255, 3 * TICKS_PER_SECOND, 20);
        dialog_clearer(env, context, false, false, false, 5);

        context.wait_for_all_requests();
        env.console.log("Go to the kitchen, talk with mom");
        env.console.overlay().add_log("Go to the kitchen, talk with mom", COLOR_WHITE);
        pbf_move_left_joystick(context, 128, 255, 4 * TICKS_PER_SECOND, 20);
        move_till_dialog(env, context, 0, 128, 20);
        dialog_clearer(env, context, false, false, false, 5);

        context.wait_for_all_requests();
        env.console.log("Go to the front door, talk with Clavell");
        env.console.overlay().add_log("Go to the front door, talk with Clavell", COLOR_WHITE);
        pbf_move_left_joystick(context, 230, 230, 2 * TICKS_PER_SECOND, 20);
        move_till_dialog(env, context, 255, 128, 15);
        dialog_clearer(env, context, false, false, false, 5);

        context.wait_for_all_requests();
        env.console.log("Go upstairs, dress up");
        env.console.overlay().add_log("Go upstairs, dress up", COLOR_WHITE);
        pbf_move_left_joystick(context, 255, 128, 2 * TICKS_PER_SECOND, 20);
        pbf_move_left_joystick(context, 185,  10, 2 * TICKS_PER_SECOND, 20);
        pbf_move_left_joystick(context, 128,   0, 4 * TICKS_PER_SECOND, 20);
        pbf_move_left_joystick(context, 255, 128, 4 * TICKS_PER_SECOND, 20);
        pbf_move_left_joystick(context, 115, 200, 3 * TICKS_PER_SECOND, 20);
        pbf_move_left_joystick(context, 255, 128, 2 * TICKS_PER_SECOND, 20);
        // TODO: Tutorial detection
        pbf_mash_button(context, BUTTON_A, 20 * TICKS_PER_SECOND);

        context.wait_for_all_requests();
        env.console.log("Go to the living room, talk with Clavell");
        env.console.overlay().add_log("Go to the living room, talk with Clavell", COLOR_WHITE);
        pbf_move_left_joystick(context,  25,  25, 4 * TICKS_PER_SECOND, 20);
        pbf_move_left_joystick(context,   0, 128, 4 * TICKS_PER_SECOND, 20);
        pbf_move_left_joystick(context, 128, 255, 4 * TICKS_PER_SECOND, 20);
        move_till_dialog(env, context, 0, 128, 20);
        dialog_clearer(env, context, false, false, false, 10);

        context.wait_for_all_requests();
        env.console.log("Go outside, receive Rotom Phone");
        env.console.overlay().add_log("Go outside, receive Rotom Phone", COLOR_WHITE);
        move_till_dialog(env, context, 245, 205, 60);
        dialog_clearer(env, context);

        context.wait_for_all_requests();
        env.console.log("Bump into power of science NPC");
        env.console.overlay().add_log("Bump into power of science NPC", COLOR_WHITE);
        pbf_move_left_joystick(context, 128, 0, 4125, 20);

        context.wait_for_all_requests();
        env.console.log("Reset position by trying to enter Poco Path");
        env.console.overlay().add_log("Reset position by trying to enter Poco Path", COLOR_WHITE);
        move_till_dialog(env, context, 25, 50, 60);
        dialog_clearer(env, context);

        context.wait_for_all_requests();
        env.console.log("Clear map tutorial");
        env.console.overlay().add_log("Clear map tutorial", COLOR_WHITE);
        open_map_from_overworld(env.program_info(), env.console, context, true);
        leave_phone_to_overworld(env.program_info(), env.console, context);

        save_game_from_overworld(env.program_info(), env.console, context);
        stats.m_checkpoint++;
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE, "Saved at checkpoint.");

        context.wait_for_all_requests();
        env.console.log("End Segment 01: Leave House", COLOR_GREEN);
        env.console.overlay().add_log("End Segment 01: Leave House", COLOR_GREEN);
        stats.m_segment++;
        env.update_stats();
        if (ENDPOINT == EndPoint::LEAVE_HOUSE){
            break;
        }
        pbf_wait(context, 1 * TICKS_PER_SECOND);

    case StartPoint::PICK_STARTER:
        context.wait_for_all_requests();
        env.console.log("Start Segment 02: Pick Starter", COLOR_ORANGE);
        env.console.overlay().add_log("Start Segment 02: Pick Starter", COLOR_ORANGE);

        while (true){
            set_map_marker(env, context, 255, 180, 1 * TICKS_PER_SECOND);
            pbf_move_left_joystick(context, 128,   0, 7 * TICKS_PER_SECOND, 20);
            if (!move_till_dialog(env, context, 100, 20, 60, false)){
                context.wait_for_all_requests();
                env.console.log("Did not enter Nemona's house, resetting from checkpoint...", COLOR_RED);
                env.console.overlay().add_log("Failed to enter house, reset", COLOR_RED);
                reset_game(env, context, "Did not enter Nemona's house, resetting from checkpoint...");
                stats.m_reset++;
                env.update_stats();
                continue;
            }
            context.wait_for_all_requests();
            env.console.overlay().add_log("Entered Nemona's house", COLOR_WHITE);
            mash_button_till_overworld(env, context);
            context.wait_for_all_requests();
            env.console.log("Picking a starter...");
            env.console.overlay().add_log("Picking a starter", COLOR_WHITE);
            pbf_move_left_joystick(context, 128, 0, 1 * TICKS_PER_SECOND, 20);
            switch(STARTERCHOICE){
            case StarterChoice::SPRIGATITO:
                env.console.log("Picking Sprigatito...");
                env.console.overlay().add_log("Picking Sprigatito...", COLOR_WHITE);
                pbf_move_left_joystick(context, 25, 90, 38, 20);
                break;
            case StarterChoice::FUECOCO:
                env.console.log("Picking Fuecoco...");
                env.console.overlay().add_log("Picking Fuecoco...", COLOR_WHITE);
                pbf_move_left_joystick(context, 230, 90, 38, 20);
                break;
            case StarterChoice::QUAXLY:
                env.console.log("Picking Quaxly...");
                env.console.overlay().add_log("Picking Quaxly...", COLOR_WHITE);
                break;
            }
            pbf_press_button(context, BUTTON_A, 20, 105);
            if (!dialog_clearer(env, context, true, false, false, 15)){
                context.wait_for_all_requests();
                env.console.log("Failed to pick starter, resetting from checkpoint...", COLOR_RED);
                env.console.overlay().add_log("Failed to select a starter, reset", COLOR_RED);
                reset_game(env, context, "Failed to pick starter, resetting from checkpoint...");
                stats.m_reset++;
                env.update_stats();
                continue;
            }
            pbf_press_button(context, BUTTON_A, 20, 105);
            // Don't give a nickname
            if (!dialog_clearer(env, context, true, false, false, 15)){
                context.wait_for_all_requests();
                env.console.log("Stuck trying to give a nickname, resetting from checkpoint...", COLOR_RED);
                env.console.overlay().add_log("Stuck on nickname page, reset", COLOR_RED);
                reset_game(env, context, "Stuck trying to give a nickname, resetting from checkpoint...");
                stats.m_reset++;
                env.update_stats();
                continue;
            }
            pbf_press_dpad(context, DPAD_DOWN,  20, 105);
            pbf_press_button(context, BUTTON_A, 20, 105);
            if (!dialog_clearer(env, context, false, true, false, 15)){
                context.wait_for_all_requests();
                env.console.log("Stuck trying to give a nickname, resetting from checkpoint...", COLOR_RED);
                env.console.overlay().add_log("Stuck on nickname page, reset", COLOR_RED);
                reset_game(env, context, "Stuck trying to give a nickname, resetting from checkpoint...");
                stats.m_reset++;
                env.update_stats();
                continue;
            }
            break;
        }

        context.wait_for_all_requests();
        env.console.log("Clear auto heal tutorial");
        env.console.overlay().add_log("Clear auto heal tutorial", COLOR_WHITE);
        enter_menu_from_overworld(env, context, 0, MenuSide::LEFT);
        pbf_press_button(context, BUTTON_A, 20, 8 * TICKS_PER_SECOND);

        context.wait_for_all_requests();
        env.console.log("Changing move order...");
        env.console.overlay().add_log("Changing move order...", COLOR_WHITE);
        // TODO: Detect move swapping
        pbf_press_dpad(context, DPAD_RIGHT, 15, 1 * TICKS_PER_SECOND);
        pbf_press_button(context, BUTTON_Y, 20, 40);
        pbf_press_button(context, BUTTON_A, 20, 40);
        pbf_press_dpad(context, DPAD_DOWN, 15, 40);
        pbf_press_dpad(context, DPAD_DOWN, 15, 40);
        pbf_press_button(context, BUTTON_A, 20, 40);
        leave_box_system_to_overworld(env.program_info(), env.console, context);

        save_game_from_overworld(env.program_info(), env.console, context);
        stats.m_checkpoint++;
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE, "Saved at checkpoint.");

        context.wait_for_all_requests();
        env.console.log("End Segment 02: Pick Starter", COLOR_GREEN);
        env.console.overlay().add_log("End Segment 02: Pick Starter", COLOR_GREEN);
        stats.m_segment++;
        env.update_stats();
        if (ENDPOINT == EndPoint::PICK_STARTER){
            break;
        }

    case StartPoint::NEMONA_FIRST_BATTLE:
        context.wait_for_all_requests();
        env.console.log("Start Segment 03: First Nemona Battle", COLOR_ORANGE);
        env.console.overlay().add_log("Start Segment 03: First Nemona Battle", COLOR_ORANGE);

        while (true){
            set_map_marker(env, context, 220, 245, 50);
            pbf_move_left_joystick(context, 128, 0, 16 * TICKS_PER_SECOND, 4 * TICKS_PER_SECOND);
            set_map_marker(env, context, 230, 25, 125);
            pbf_move_left_joystick(context, 140, 0, 5 * TICKS_PER_SECOND, 20);
            pbf_move_left_joystick(context, 170, 25, 55, 20);
            pbf_move_left_joystick(context, 130, 0, 2 * TICKS_PER_SECOND, 20);
            if (!move_till_dialog(env, context, 135, 0, 8, false, true)){
                context.wait_for_all_requests();
                env.console.log("Did not talk to Nemona at beach, resetting from checkpoint...", COLOR_RED);
                env.console.overlay().add_log("Can't find Nemona, reset", COLOR_RED);
                reset_game(env, context, "Did not talk to Nemona at beach, resetting from checkpoint...");
                stats.m_reset++;
                env.update_stats();
                continue;
            }
            context.wait_for_all_requests();
            env.console.overlay().add_log("Found Nemona", COLOR_WHITE);
            break;
        }

        context.wait_for_all_requests();
        env.console.log("Starting battle...");
        env.console.overlay().add_log("Starting battle...", COLOR_WHITE);
        // TODO: Battle start prompt detection
        mash_button_till_overworld(env, context);
        context.wait_for_all_requests();
        env.console.log("Finished battle.");
        env.console.overlay().add_log("Finished battle.", COLOR_WHITE);

        save_game_from_overworld(env.program_info(), env.console, context);
        stats.m_checkpoint++;
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE, "Saved at checkpoint.");

        context.wait_for_all_requests();
        env.console.log("End Segment 03: First Nemona Battle", COLOR_GREEN);
        env.console.overlay().add_log("End Segment 03: First Nemona Battle", COLOR_GREEN);
        stats.m_segment++;
        env.update_stats();
        if (ENDPOINT == EndPoint::NEMONA_FIRST_BATTLE){
            break;
        }

    case StartPoint::CATCH_TUTORIAL:
        context.wait_for_all_requests();
        env.console.log("Start Segment 04: Catch Tutorial", COLOR_ORANGE);
        env.console.overlay().add_log("Start Segment 04: Catch Tutorial", COLOR_ORANGE);

        while (true){
            set_map_marker(env, context, 40, 160, 60);
            pbf_move_left_joystick(context, 128, 0, 7 * TICKS_PER_SECOND, 20);
            set_map_marker(env, context, 40, 84, 60);
            if (!move_till_dialog(env, context, 128, 0, 20, false)){
                context.wait_for_all_requests();
                env.console.log("Did not find Mom, resetting from checkpoint...", COLOR_RED);
                env.console.overlay().add_log("Can't find mom, reset", COLOR_RED);
                reset_game(env, context, "Did not find Mom, resetting from checkpoint...");
                stats.m_reset++;
                env.update_stats();
                continue;
            }
            context.wait_for_all_requests();
            env.console.log("Get mom's sandwich");
            env.console.overlay().add_log("Get mom's sandwich", COLOR_WHITE);
            mash_button_till_overworld(env, context);
            break;
        }

        save_game_from_overworld(env.program_info(), env.console, context);
        stats.m_checkpoint++;
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE, "Saved at checkpoint.");

        while (true){
            set_map_marker(env, context, 40, 82, 60);
            pbf_move_left_joystick(context, 128, 0, 6 * TICKS_PER_SECOND, 20);
            set_map_marker(env, context, 110, 10, 60);
            if (!move_till_dialog(env, context, 128, 0, 20, false)){
                context.wait_for_all_requests();
                env.console.log("Did not find Nemona, resetting from checkpoint...", COLOR_RED);
                env.console.overlay().add_log("Can't find Nemona, reset", COLOR_RED);
                reset_game(env, context, "Did not find Nemona, resetting from checkpoint...");
                stats.m_reset++;
                env.update_stats();
                continue;
            }
            context.wait_for_all_requests();
            env.console.log("Start catch tutorial");
            env.console.overlay().add_log("Start catch tutorial", COLOR_WHITE);
            dialog_clearer(env, context, false, false, false, 10);
            // TODO: Tutorial detection
            pbf_press_button(context, BUTTON_A, 20, 105);
            pbf_press_button(context, BUTTON_A, 20, 105);
            run_battle(env, context, true);
            dialog_clearer(env, context, false, false, false, 10);
            context.wait_for_all_requests();
            env.console.log("Finished catch tutorial");
            env.console.overlay().add_log("Finished catch tutorial", COLOR_WHITE);

            // TODO: Tutorial detection
            pbf_press_button(context, BUTTON_A, 20, 105);
            pbf_press_button(context, BUTTON_A, 20, 105);
            break;
        }

        save_game_from_overworld(env.program_info(), env.console, context);
        stats.m_checkpoint++;
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE, "Saved at checkpoint.");

        context.wait_for_all_requests();
        env.console.log("Move to cliff");
        env.console.overlay().add_log("Move to cliff", COLOR_WHITE);

        while (true){
            set_map_marker(env, context, 240, 60, 80);
            if (!move_toward_destination(env, context, 116, 0, 180, false, false, 24)){
                context.wait_for_all_requests();
                env.console.log("Did not reach cliff, resetting from checkpoint...", COLOR_RED);
                env.console.overlay().add_log("Did not reach cliff, reset", COLOR_RED);
                reset_game(env, context, "Did not reach cliff, resetting from checkpoint...");
                stats.m_reset++;
                env.update_stats();
                continue;
            }
            dialog_clearer(env, context);
            context.wait_for_all_requests();
            env.console.log("Mystery cry");
            env.console.overlay().add_log("Mystery cry", COLOR_WHITE);
            break;
        }

        save_game_from_overworld(env.program_info(), env.console, context);
        stats.m_checkpoint++;
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE, "Saved at checkpoint.");

        context.wait_for_all_requests();
        env.console.log("End Segment 04: Catch Tutorial", COLOR_GREEN);
        env.console.overlay().add_log("End Segment 04: Catch Tutorial", COLOR_GREEN);
        stats.m_segment++;
        env.update_stats();
        if (ENDPOINT == EndPoint::CATCH_TUTORIAL){
            break;
        }

    case StartPoint::LEGENDARY_RESCUE:
        context.wait_for_all_requests();
        env.console.log("Start Segment 05: Rescue Legendary", COLOR_ORANGE);
        env.console.overlay().add_log("Start Segment 05: Rescue Legendary", COLOR_ORANGE);

        while (true){
            set_map_marker(env, context, 230, 70, 100);

            if (!move_till_dialog(env, context, 128, 0, 30, false)){
                context.wait_for_all_requests();
                env.console.log("Did not reach cliff, resetting from checkpoint...", COLOR_RED);
                env.console.overlay().add_log("Did not reach cliff, reset", COLOR_RED);
                reset_game(env, context, "Did not reach cliff, resetting from checkpoint...");
                stats.m_reset++;
                env.update_stats();
                continue;
            }
            dialog_clearer(env, context, false, false, false, 10);
            // long animation
            dialog_clearer(env, context, false, false, false, 15);
            if (!move_till_dialog(env, context, 128, 0, 30, false)){
                context.wait_for_all_requests();
                env.console.log("Did not reach legendary, resetting from checkpoint...", COLOR_RED);
                env.console.overlay().add_log("Did not reach legendary, reset", COLOR_RED);
                reset_game(env, context, "Did not reach legendaryf, resetting from checkpoint...");
                stats.m_reset++;
                env.update_stats();
                continue;
            }
            dialog_clearer(env, context, false, false, false, 10);

            // TODO: Bag menu navigation
            context.wait_for_all_requests();
            env.console.log("Feed mom's sandwich");
            env.console.overlay().add_log("Feed mom's sandwich", COLOR_WHITE);
            pbf_press_dpad(context, DPAD_UP, 20, 105);
            pbf_mash_button(context, BUTTON_A, 100);
            dialog_clearer(env, context, false, false, false, 20);
            // long animation
            dialog_clearer(env, context, false, false, false, 10);

            // First Nemona cave conversation
            context.wait_for_all_requests();
            env.console.log("Enter cave");
            env.console.overlay().add_log("Enter cave", COLOR_WHITE);
            pbf_move_left_joystick(context, 128, 20, 10 * TICKS_PER_SECOND, 20);
            pbf_move_left_joystick(context, 150, 20, 1 * TICKS_PER_SECOND, 20);
            pbf_move_left_joystick(context, 128, 20, 8 * TICKS_PER_SECOND, 20);
            pbf_move_left_joystick(context, 150, 20, 2 * TICKS_PER_SECOND, 20);
            if (!move_till_dialog(env, context, 128, 20, 10, false)){
                context.wait_for_all_requests();
                env.console.log("Did not enter cave, resetting from checkpoint...", COLOR_RED);
                env.console.overlay().add_log("Did not enter cave, reset", COLOR_RED);
                reset_game(env, context, "Did not enter cave, resetting from checkpoint...");
                stats.m_reset++;
                env.update_stats();
                continue;
            }
            dialog_clearer(env, context, false, false, false, 10);

            // Legendary rock break
            context.wait_for_all_requests();
            env.console.log("Rock break");
            env.console.overlay().add_log("Rock break", COLOR_WHITE);
            pbf_move_left_joystick(context, 128, 20, 3 * TICKS_PER_SECOND, 20);
            pbf_move_left_joystick(context, 230, 25, 30, 20);
            pbf_press_button(context, BUTTON_L, 20, 2 * TICKS_PER_SECOND);
            pbf_move_left_joystick(context, 128, 0, 2 * TICKS_PER_SECOND, 5 * TICKS_PER_SECOND);

            // Houndour wave
            context.wait_for_all_requests();
            env.console.log("Houndour wave");
            env.console.overlay().add_log("Houndour wave", COLOR_WHITE);
            pbf_move_left_joystick(context, 128, 20, 4 * TICKS_PER_SECOND, 20);
            pbf_move_left_joystick(context, 200, 15, 30, 20);
            pbf_press_button(context, BUTTON_L, 20, 20);
            pbf_move_left_joystick(context, 128, 20, 10 * TICKS_PER_SECOND, 2 * TICKS_PER_SECOND);
            pbf_move_left_joystick(context, 200, 25, 20, 20);
            pbf_press_button(context, BUTTON_L, 20, 20);
            pbf_move_left_joystick(context, 128, 20, 11 * TICKS_PER_SECOND, 2 * TICKS_PER_SECOND);
            pbf_move_left_joystick(context, 230, 25, 25, 20);
            pbf_press_button(context, BUTTON_L, 20, 20);
            pbf_move_left_joystick(context, 128, 20, 6 * TICKS_PER_SECOND, 20 * TICKS_PER_SECOND);

            // Houndoom encounter
            context.wait_for_all_requests();
            env.console.log("Houndoom encounter");
            env.console.overlay().add_log("Houndoom encounter", COLOR_WHITE);
            pbf_move_left_joystick(context, 128, 20, 4 * TICKS_PER_SECOND, 20);
            pbf_move_left_joystick(context, 245, 20, 20, 20);
            pbf_press_button(context, BUTTON_L, 20, 20);
            pbf_move_left_joystick(context, 128, 20, 2 * TICKS_PER_SECOND, 20);
            pbf_move_left_joystick(context, 255, 90, 20, 20);
            pbf_press_button(context, BUTTON_L, 20, 20);
            pbf_move_left_joystick(context, 128, 20, 8 * TICKS_PER_SECOND, 8 * TICKS_PER_SECOND);
            pbf_press_button(context, BUTTON_L, 20, 20);
            if (!move_till_dialog(env, context, 128, 20, 40, false)){
                context.wait_for_all_requests();
                env.console.log("Did not reach Houndoom, resetting from checkpoint...", COLOR_RED);
                env.console.overlay().add_log("Did not reach Houndoom, reset", COLOR_RED);
                reset_game(env, context, "Did not reach Houndoom, resetting from checkpoint...");
                stats.m_reset++;
                env.update_stats();
                continue;
            }
            mash_button_till_overworld(env, context, BUTTON_A);
            break;
        }

        save_game_from_overworld(env.program_info(), env.console, context);
        stats.m_checkpoint++;
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE, "Saved at checkpoint.");

        context.wait_for_all_requests();
        env.console.log("End Segment 05: Rescue Legendary", COLOR_GREEN);
        env.console.overlay().add_log("End Segment 05: Rescue Legendary", COLOR_GREEN);
        stats.m_segment++;
        env.update_stats();
        if (ENDPOINT == EndPoint::LEGENDARY_RESCUE){
            break;
        }

    case StartPoint::ARVEN_FIRST_BATTLE:
        context.wait_for_all_requests();
        env.console.log("Start Segment 06: First Arven Battle", COLOR_ORANGE);
        env.console.overlay().add_log("Start Segment 06: First Arven Battle", COLOR_ORANGE);

        while (true){
            set_map_marker(env, context, 230, 120, 100);
            if (!move_toward_destination(env, context, 128, 0, 180, false, false)){
                context.wait_for_all_requests();
                env.console.log("Did not talk to Arven at lab, resetting from checkpoint...", COLOR_RED);
                env.console.overlay().add_log("Can't find Arven, reset", COLOR_RED);
                reset_game(env, context, "Did not talk to Arven at lab, resetting from checkpoint...");
                stats.m_reset++;
                env.update_stats();
                continue;
            }
            context.wait_for_all_requests();
            env.console.log("Found Arven");
            env.console.overlay().add_log("Found Arven", COLOR_WHITE);
            mash_button_till_overworld(env, context, BUTTON_A);
            context.wait_for_all_requests();
            env.console.log("Receive legendary ball");
            env.console.overlay().add_log("Receive legendary ball", COLOR_WHITE);
            break;
        }

        save_game_from_overworld(env.program_info(), env.console, context);
        stats.m_checkpoint++;
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE, "Saved at checkpoint.");

        while (true){
            context.wait_for_all_requests();
            env.console.log("Lighthouse view");
            env.console.overlay().add_log("Lighthouse view", COLOR_WHITE);
            set_map_marker(env, context, 230, 110, 100);
            pbf_move_left_joystick(context, 128, 0, 6 * TICKS_PER_SECOND, 8 * TICKS_PER_SECOND);
            pbf_move_left_joystick(context, 128, 0, 4 * TICKS_PER_SECOND, 20);
            pbf_move_left_joystick(context, 255, 128, 15, 20);
            pbf_press_button(context, BUTTON_L, 20, 20);
            pbf_move_left_joystick(context, 128, 0, 7 * TICKS_PER_SECOND, 20);
            if (!move_till_dialog(env, context, 128, 0, 10, false, true)){
                context.wait_for_all_requests();
                env.console.log("Did not talk to Nemona on the lighthouse, resetting from checkpoint...", COLOR_RED);
                env.console.overlay().add_log("Can't find Nemona, reset", COLOR_RED);
                reset_game(env, context, "Did not talk to Nemona on the lighthouse, resetting from checkpoint...");
                stats.m_reset++;
                env.update_stats();
                continue;
            }
            mash_button_till_overworld(env, context, BUTTON_A);
            break;
        }

        save_game_from_overworld(env.program_info(), env.console, context);
        stats.m_checkpoint++;
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE, "Saved at checkpoint.");

        context.wait_for_all_requests();
        env.console.log("End Segment 06: First Arven Battle", COLOR_GREEN);
        env.console.overlay().add_log("End Segment 06: First Arven Battle", COLOR_GREEN);
        stats.m_segment++;
        env.update_stats();
        if (ENDPOINT == EndPoint::ARVEN_FIRST_BATTLE){
            break;
        }

    case StartPoint::GOTO_LOS_PLATOS:
        context.wait_for_all_requests();
        env.console.log("Start Segment 07: Go to Los Platos", COLOR_ORANGE);
        env.console.overlay().add_log("Start Segment 07: Go to Los Platos", COLOR_ORANGE);

        while (true){
            set_map_marker(env, context, 100, 210, 100);
            pbf_move_left_joystick(context, 128, 0, 187, 20);
            pbf_move_left_joystick(context, 0, 128, 30, 8 * TICKS_PER_SECOND);
            pbf_move_left_joystick(context, 128, 0, 1 * TICKS_PER_SECOND, 2 * TICKS_PER_SECOND);

            set_map_marker(env, context, 100, 60, 200);
            if (!move_toward_destination(env, context, 128, 0, 180, false, true, 15)){
                context.wait_for_all_requests();
                env.console.log("Did not reach Los Platos, resetting from checkpoint...", COLOR_RED);
                env.console.overlay().add_log("Did not reach Los Platos, reset", COLOR_RED);
                reset_game(env, context, "Did not reach Los Platos, resetting from checkpoint...");
                stats.m_reset++;
                env.update_stats();
                continue;
            }
            dialog_clearer(env, context, false, false, false, 10);
            // TODO: Tutorial detection
            pbf_press_button(context, BUTTON_A, 20, 105);
            pbf_press_button(context, BUTTON_A, 20, 105);
            pbf_press_button(context, BUTTON_A, 20, 105);
            pbf_press_button(context, BUTTON_A, 20, 105);
            context.wait_for_all_requests();
            env.console.log("Reached Los Platos");
            env.console.overlay().add_log("Reached Los Platos", COLOR_WHITE);
            break;
        }

        save_game_from_overworld(env.program_info(), env.console, context);
        stats.m_checkpoint++;
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE, "Saved at checkpoint.");

        context.wait_for_all_requests();
        env.console.log("End Segment 07: Go to Los Platos", COLOR_GREEN);
        env.console.overlay().add_log("End Segment 07: Go to Los Platos", COLOR_GREEN);
        stats.m_segment++;
        env.update_stats();
        if (ENDPOINT == EndPoint::GOTO_LOS_PLATOS){
            break;
        }

    }
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}






}
}
}