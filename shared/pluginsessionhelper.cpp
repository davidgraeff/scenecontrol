#include "pluginsessionhelper.h"

void PluginSessionsHelper::session_change(int id, bool running) {
    if ( running ) m_sessions.insert ( id );
    else m_sessions.remove ( id );
}
