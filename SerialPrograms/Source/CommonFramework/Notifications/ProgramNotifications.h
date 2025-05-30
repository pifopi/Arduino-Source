/*  Program Notifications
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ProgramNotifications_H
#define PokemonAutomation_ProgramNotifications_H

#include <vector>
#include <string>
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "ProgramInfo.h"
#include "EventNotificationOption.h"

namespace PokemonAutomation{

class Logger;
class StatsTracker;
class ProgramEnvironment;



//  Send raw Discord notification according to the notification settings.
//  Can control the message sidebar color, message title, and message body.
//  messages: vector<pair<string, string>>, the sections of the Discord message.
//    The first string in the pair is the section title (will shown as bold) while the second
//    string is the section content.
//    e.g. {{"Message:", {"This is a message."}}} will shown as (in Markdown):
//    **Message**
//    This is a message.
//  The function will add an additional section "Powered By".
//  filepath: send the file as a message attachment.
void send_raw_program_notification_with_file(
    Logger& logger, EventNotificationOption& settings,
    Color color,
    const ProgramInfo& info,
    const std::string& title,
    const std::vector<std::pair<std::string, std::string>>& messages,
    const std::string& filepath
);

//  Send raw Discord notification according to the notification settings.
//  Can control the message sidebar color, message title, and message body.
//  messages: vector<pair<string, string>>, the sections of the Discord message.
//    The first string in the pair is the section title (will shown as bold) while the second
//    string is the section content.
//    e.g. {{"Message:", {"This is a message."}}} will shown as (in Markdown):
//    **Message**
//    This is a message.
//  The function will add an additional section "Powered By".
//  image: if not empty, send the screenshot image as part of the message according to the
//    notification settings.
//  keep_file: if true, the sent image will be saved to the screenshot dir on disk. Otherwise, it
//    will be saved to the program TEMP_FOLDER.
void send_raw_program_notification(
    Logger& logger, EventNotificationOption& settings,
    Color color,
    const ProgramInfo& info,
    const std::string& title,
    const std::vector<std::pair<std::string, std::string>>& messages,
    const ImageViewRGB32& image = ImageViewRGB32(), bool keep_file = false
);



//  Send custom Discord notification according to the notification settings.
//  Can control the message sidebar color, message title, and message body.
//  messages: vector<pair<string, string>>, the sections of the Discord message.
//    The first string in the pair is the section title (will shown as bold) while the second
//    string is the section content.
//    e.g. {{"Message:", {"This is a message."}}} will shown as (in Markdown):
//    **Message**
//    This is a message.
//  The function will add additional sections: "Current Session", "Historical Stats"
//    and "Powered By".
//  current_stats_addendum: add additional message at the end of the "Current Session" section
//  where it shows the current program stats message generated by StatsTracker.
//  filepath: send the file as a message attachment.
void send_program_notification_with_file(
    ProgramEnvironment& env, EventNotificationOption& settings,
    Color color,
    const std::string& title,
    std::vector<std::pair<std::string, std::string>> messages,
    const std::string& current_stats_addendum,
    const std::string& filepath
);

//  Send custom Discord notification according to the notification settings.
//  Can control the message sidebar color, message title, and message body.
//  messages: vector<pair<string, string>>, the sections of the Discord message.
//    The first string in the pair is the section title (will shown as bold) while the second
//    string is the section content.
//    e.g. {{"Message:", {"This is a message."}}} will shown as (in Markdown):
//    **Message**
//    This is a message.
//  The function will add additional sections: "Current Session", "Historical Stats"
//    and "Powered By".
//  current_stats_addendum: add additional message at the end of the "Current Session" section
//  where it shows the current program stats message generated by StatsTracker.
//  image: if not empty, send the screenshot image as part of the message according to the
//    notification settings.
//  keep_file: if true, the sent image will be saved to the screenshot dir on disk. Otherwise, it
//    will be saved to the program TEMP_FOLDER.
bool send_program_notification(
    ProgramEnvironment& env, EventNotificationOption& settings,
    Color color,
    const std::string& title,
    std::vector<std::pair<std::string, std::string>> messages,
    const std::string& current_stats_addendum,
    const ImageViewRGB32& image = ImageViewRGB32(), bool keep_file = false
);



//  Send Discord notification of the current program status according to the notification settings.
//  Can send additional message and screenshot image.
//  keep_file: if true, the sent image will be saved to the screenshot dir on disk. Otherwise, it
//    will be saved to the program TEMP_FOLDER.
void send_program_status_notification(
    ProgramEnvironment& env, EventNotificationOption& settings,
    const std::string& message = "",
    const ImageViewRGB32& image = ImageViewRGB32(), bool keep_file = false
);

//  Send Discord notification that the program has finished according to the notification settings.
//  Can send additional message and screenshot image.
//  keep_file: if true, the sent image will be saved to the screenshot dir on disk. Otherwise, it
//    will be saved to the program TEMP_FOLDER.
void send_program_finished_notification(
    ProgramEnvironment& env, EventNotificationOption& settings,
    const std::string& message = "",
    const ImageViewRGB32& image = ImageViewRGB32(), bool keep_file = false
);

//  Send Discord notification that the program has encountered a recoverable error according to the
//  notification settings.
//  Can send additional message and screenshot image.
//  keep_file: if true, the sent image will be saved to the screenshot dir on disk. Otherwise, it
//    will be saved to the program TEMP_FOLDER.
void send_program_recoverable_error_notification(
    ProgramEnvironment& env, EventNotificationOption& settings,
    const std::string& message,
    const ImageViewRGB32& image = ImageViewRGB32(), bool keep_file = false
);

//  Send Discord notification that the program has a fatal error according to the notification
//  settings.
//  Can send additional message and screenshot image.
//  keep_file: if true, the sent image will be saved to the screenshot dir on disk. Otherwise, it
//    will be saved to the program TEMP_FOLDER.
void send_program_fatal_error_notification(
    ProgramEnvironment& env, EventNotificationOption& settings,
    const std::string& message,
    const ImageViewRGB32& image = ImageViewRGB32(), bool keep_file = false
);






}
#endif
