/*  Shiny Encounter Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "PokemonBDSP/Inference/ShinyDetection/PokemonBDSP_ShinyEncounterDetector.h"
#include "PokemonBDSP_ShinyEncounterTester.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


ShinyEncounterTester_Descriptor::ShinyEncounterTester_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonBDSP:ShinyEncounterTester",
        STRING_POKEMON + " BDSP", "Shiny Encounter Tester",
        "",
        "Test the shiny encounter detector. Start this program just before an encounter.",
        FeedbackType::REQUIRED, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



ShinyEncounterTester::ShinyEncounterTester(const ShinyEncounterTester_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , ENCOUNTER_TYPE(
        "<b>Encounter Type:</b>",
        {
            "Starter Battle",
            "Wild Encounter",
        },
        1
    )
{
    PA_ADD_OPTION(ENCOUNTER_TYPE);
}


void ShinyEncounterTester::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    DoublesShinyDetection result_wild;
    ShinyDetectionResult result_own;
    detect_shiny_battle(
        env.console, context,
        result_wild, result_own,
        ENCOUNTER_TYPE == 0 ? YOUR_POKEMON : WILD_POKEMON,
        std::chrono::seconds(30)
    );
}





}
}
}