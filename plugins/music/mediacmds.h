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
    InfoCmd,
    AspectRatioCmd,
    NextSubtitleCmd,
    NextLanguageCmd,
    NavigationUpCmd,
    NavigationDownCmd,
    NavigationLeftCmd,
    NavigationRightCmd,
    NavigationBackCmd,
    NavigationOKCmd,
    NavigationHomeCmd,
    NavigationCloseCmd,
    NavigationContextMenuCmd,
    FastForwardCmd,
    FastRewindCmd
};
Q_ENUMS(EnumMediaCmd);

#endif