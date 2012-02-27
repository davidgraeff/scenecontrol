#include "actioncontroller.h"
#include <windows.h>
#include <winuser.h>

ActionController::ActionController(QObject *parent) :
    QObject(parent)
{
}

void ActionController::serverJSON(const QVariantMap &data)
{
    INPUT input;
    input.type = INPUT_KEYBOARD;
    input.ki.dwFlags = 0;
    input.ki.time = 0;
    input.ki.dwExtraInfo = 0;

    const QByteArray method = data.value(QLatin1String("member_")).toByteArray();
    if (method == "display_on") {
        const int MONITOR_ON = -1;
        PostMessage(HWND_TOPMOST, WM_SYSCOMMAND, SC_MONITORPOWER, MONITOR_ON);
    } else if (method == "display_off") {
        const int MONITOR_OFF = 2;
        PostMessage(HWND_TOPMOST, WM_SYSCOMMAND, SC_MONITORPOWER, MONITOR_OFF);
    } else if (method == "togglemute") {
        input.ki.wVk = VK_VOLUME_MUTE;
        SendInput(1,&input,sizeof(input));
    } else if (method == "standby") {
        input.ki.wVk = VK_SLEEP;
        SendInput(1,&input,sizeof(input));
    } else if (method == "startmediaapp") {
        input.ki.wVk = VK_LAUNCH_MEDIA_SELECT;
        SendInput(1,&input,sizeof(input));
    } else if (method == "startmedia") {
        input.ki.wVk = VK_MEDIA_PLAY_PAUSE;
        SendInput(1,&input,sizeof(input));
    } else if (method == "stopmedia") {
        input.ki.wVk = VK_MEDIA_STOP;
        SendInput(1,&input,sizeof(input));
    } else if (method == "nextmedia") {
        input.ki.wVk = VK_MEDIA_NEXT_TRACK;
        SendInput(1,&input,sizeof(input));
    } else if (method == "previousmedia") {
        input.ki.wVk = VK_MEDIA_PREV_TRACK;
        SendInput(1,&input,sizeof(input));
    } else if (method == "volume_relative" && data.value(QLatin1String("volume")).toInt()>0) {
        input.ki.wVk = VK_VOLUME_UP;
        SendInput(1,&input,sizeof(input));
    } else if (method == "volume_relative" && data.value(QLatin1String("volume")).toInt()<0) {
        input.ki.wVk = VK_VOLUME_DOWN;
        SendInput(1,&input,sizeof(input));
    }
}
