#ifndef __YYLERPLAYER_H__
#define __YYLERPLAYER_H__

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "Music.h"
#include "Console.h"
#include "System.h"
#include "Socket.h"
#include "CmdLine.h"
#include "Thread.h"
#include "Lyric.h"
#include "cJSON.h"

#include "MusicList.h"
#include "YylerConfig.h"

typedef struct _YylerPlayer { 
    /* 命令行参数 */
    int argc;      
    char **argv;
    int ui_mode;
    int ui_mode_changed;
    /* 错误和提示信息的输出 */
    int last_err_len;
    int err_x, err_y;        
    char err_buf1[100], err_buf2[100];
    int prompt_x, prompt_y;
    int prompt_show;
    char prompt_buf[256];
    /* 搜索*/
    char search_buf[256];
    char search_pos;
    /* 音乐播放列表 */
    MusicList search_list;
    MusicList list;
    MusicNode start_node_show;
    MusicNode end_node_show;
    MusicNode current_node_show;
    int list_node_changed;
    int repeat_one;
    /* 命令行选项 */
    int cmdline_repeat;
    int cmdline_volume;
    int have_stdin;
    int cmdline_volume_change;
    int cmdline_time_change;
    char cmd_list[256];
    int cmd_no_ui;
    int cmd_do_not_clear;
    int cmd_shuffle;
    int cmd_use_key;
    /* 其他状态信息 */
    int ui_drawed;
    int err_flag;
    char buf[4096];
} YylerPlayer;

void YylerPlayer_ShowHelp(YylerPlayer *player);
void YylerPlayer_ShowPrompt(YylerPlayer *player, int keep, const char *format, ...);
void YylerPlayer_ShowFileName(YylerPlayer *player, MusicList list);
void YylerPlayer_ShowProgressBar(YylerPlayer *player, MusicList list);
void YylerPlayer_ShowListInfo(YylerPlayer *player, MusicList list);
void YylerPlayer_ShowShortcuts(YylerPlayer *player, MusicList list);
void YylerPlayer_ShowAbout(YylerPlayer *player, MusicList list);
void YylerPlayer_ShowList(YylerPlayer *player, MusicList list);
void YylerPlayer_ClearUI(YylerPlayer *player);
void YylerPlayer_ResolveKey(YylerPlayer *player, MusicList list);
void YylerPlayer_ShowFace(YylerPlayer *player);
void YylerPlayer_PrintError(YylerPlayer *player, const char *str1, const char *str2);
void YylerPlayer_Init(YylerPlayer *player, int argc, char *argv[]);
void YylerPlayer_Quit(YylerPlayer *player, int clear, int _quit);
void YylerPlayer_ShowLyric(YylerPlayer *player, MusicList list);
void YylerPlayer_ResolveCmdLine(YylerPlayer *player);
void YylerPlayer_PlayMusicList(YylerPlayer *player, MusicList list);

MusicList YylerPlayer_GetListBySearchNet(YylerPlayer *player, const char *src, const char *words);

#endif
