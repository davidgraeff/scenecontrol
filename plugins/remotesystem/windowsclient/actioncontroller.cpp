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
    if (method == "display") {
        const int MONITOR_ON = -1;
		const int MONITOR_OFF = 2;
		int v = data.value(QLatin1String("power")).toInt();
		switch(v) {
			case 0: PostMessage(HWND_TOPMOST, WM_SYSCOMMAND, SC_MONITORPOWER, MONITOR_OFF); break;
			case 1: PostMessage(HWND_TOPMOST, WM_SYSCOMMAND, SC_MONITORPOWER, MONITOR_ON); break;
			case 2: break;
			default: break;
		}
    } else if (method == "mute") {
		int v = data.value(QLatin1String("mute")).toInt();
		// ignore off, on, toogle value of v
		Q_UNUSED(v);
        input.ki.wVk = VK_VOLUME_MUTE;
        SendInput(1,&input,sizeof(input));
    } else if (method == "standby") {
        input.ki.wVk = VK_SLEEP;
        SendInput(1,&input,sizeof(input));
    } else if (method == "media.start") {
        input.ki.wVk = VK_LAUNCH_MEDIA_SELECT;
        SendInput(1,&input,sizeof(input));
    } else if (method == "media.playpause") {
        input.ki.wVk = VK_MEDIA_PLAY_PAUSE;
        SendInput(1,&input,sizeof(input));
    } else if (method == "media.stop") {
        input.ki.wVk = VK_MEDIA_STOP;
        SendInput(1,&input,sizeof(input));
    } else if (method == "media.next") {
        input.ki.wVk = VK_MEDIA_NEXT_TRACK;
        SendInput(1,&input,sizeof(input));
    } else if (method == "media.previous") {
        input.ki.wVk = VK_MEDIA_PREV_TRACK;
        SendInput(1,&input,sizeof(input));
    } else if (method == "volume.relative") {
		int volume = data.value(QLatin1String("volume")).toInt();
		if (volume>0) {
			input.ki.wVk = VK_VOLUME_UP;
			SendInput(1,&input,sizeof(input));
		} else if (volume < 0) {
			input.ki.wVk = VK_VOLUME_DOWN;
			SendInput(1,&input,sizeof(input));
		}
    } else if (method == "volume.absolute") {
        
    } else if (method == "media.playlist.next") {
        
    } else if (method == "media.playlist.previous") {
        
    }
}
