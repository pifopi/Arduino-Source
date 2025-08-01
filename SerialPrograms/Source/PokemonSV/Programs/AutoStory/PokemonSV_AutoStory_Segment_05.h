/*  Autostory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_AutoStory_Segment_05_H
#define PokemonAutomation_PokemonSV_AutoStory_Segment_05_H

#include "PokemonSV_AutoStoryTools.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

class AutoStory_Segment_05 : public AutoStory_Segment{
public:
    virtual std::string name() const override;
    virtual std::string start_text() const override;
    virtual std::string end_text() const override;
    virtual void run_segment(
        SingleSwitchProgramEnvironment& env, 
        ProControllerContext& context,
        AutoStoryOptions options
    ) const override;
};


// start: Rescued Koraidon/Miraidon and escaped from the Houndoom Cave.
// end: Battled Arven and received Legendary's Pokeball.
void checkpoint_09(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    EventNotificationOption& notif_status_update
);

// start: Battled Arven and received Legendary's Pokeball.
// end: Talked to Nemona at the Lighthouse.
void checkpoint_10(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    EventNotificationOption& notif_status_update
);

}
}
}
#endif
