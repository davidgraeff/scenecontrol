#ifndef MEDIACMDS_H
#define MEDIACMDS_H

enum EnumMediaState
{
    PlayState,
    PauseState,
    StopState
};
Q_ENUMS(EnumMediaState);

enum EnumMediaCmd
{
    PlayCmd,
    StopCmd,
    PauseCmd,
    NextCmd,
    PrevCmd,
    NextPlaylistCmd,
    PrevPlaylistCmd,
    FullscreenCmd,
    NextSubtitleCmd
};
Q_ENUMS(EnumMediaCmd);

#endif