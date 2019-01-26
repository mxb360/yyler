#ifndef __MUSIC_LIST_H__
#define __MUSIC_LIST_H__

#include "music.h"
#include "lyric.h"
#include "cJSON.h"

#define START_PLAY   1
#define PLAYING      2
#define PAUSED       3

typedef struct _MusicNode { 
    struct _MusicNode *next;
    struct _MusicNode *last;
    char  file_path[256];
    char  file_name[100];
    char  name[100];
    char  singer[100];
    char  name_url[256];
    char  lyric_url[256];
    Music music;
    Lyric lyric;
    MTLen time;
    int   minute;
    int   second;
    int   id;
} *MusicNode;

typedef struct _MusicStatus {
    MTLen time;
    int   minute;
    int   second;
    int   volume;
    int   control_cmd;
    int   status;
} MusicStatus;

typedef struct _MusicList {
    char name[256];
    MusicNode first;
    MusicNode end;
    MusicNode node;
    MusicStatus status;
    int music_cnt;
} *MusicList;


MusicList MusicList_New(const char *name);
int       MusicList_AddMusicEnd(MusicList list, MusicNode node);
int       MusicList_AddMusicFirst(MusicList list, MusicNode node);
MusicNode MusicNode_CreateNodeByName(const char *name);
MusicNode MusicNode_CreateNodeByJsonData(cJSON *data);

void MusicList_Free(MusicList list);

#endif  /* !__MUSIC_LIST_H__ */
