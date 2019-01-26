#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "MusicList.h"

MusicList MusicList_New(const char *name)
{
    MusicList list = (MusicList)malloc(sizeof (struct _MusicList));
    if (list) {
        list->node = list->first = list->end = NULL;
        list->music_cnt = 0;
        strcpy(list->name, name);
        memset(&list->status, 0, sizeof list->status);
    }
    return list;
}

int MusicList_AddMusicEnd(MusicList list, MusicNode music)
{
    if (list == NULL || music == NULL)
        return -1;

    if (list->end == NULL) {
        list->first = music;
        music->last = music->next = NULL;
    } else {
        list->end->next = music;
        music->next = NULL;
        music->last = list->end;
    }

    list->end = music;
    list->music_cnt++;
    music->id = list->music_cnt;

    return 0;
}

int MusicList_AddMusicFirst(MusicList list, MusicNode music)
{
    int i;

    if (list == NULL || music == NULL)
        return -1;

    if (list->first == NULL) {
        list->first = music;
        music->last = music->next = NULL;
    } else {
        list->first->last = music;
        music->next = list->first;
        music->last = NULL;
    }

    list->first = music;
    list->music_cnt++;
    music->id = list->music_cnt;

    for (i = 1; music; music = music->next)
        music->id = i++;
    return 0;
}

static void get_name_and_singer(const char *file_name, char *name, char *singer, int mode)
{
    int name_pos = 0, singer_pos = 0;

    const char *point = strrchr(file_name, '.');
    if (mode) {
        char *p = singer;
        singer = name;
        name = p;
    }

    while (*file_name == ' ')
        file_name++;
    while (*file_name && *file_name != '-' && file_name != point)
        name[name_pos++] = *file_name++;
    while (name_pos > 0 && name[name_pos - 1] == ' ')
        name_pos--;

    if (*file_name == '-')
        file_name++;

    while (*file_name == ' ')
        file_name++;
    while (file_name != point)
        singer[singer_pos++] = *file_name++;
    while (singer_pos && singer[singer_pos - 1] == ' ')
        singer_pos--;

    if (name_pos)
        name[name_pos] = 0;
    else
        strcpy(name, "<未知歌名>");
    if (singer_pos)
        singer[singer_pos] = 0;
    else
        strcpy(singer, "<未知歌手>");
}

MusicNode MusicNode_CreateNodeByName(const char *name)
{
    MusicNode node;
    Music music;
    /* 检查音乐是否能正常播放 */
    music = Music_Create(name);
    if (music == NULL) 
        return NULL;
    node = (MusicNode)malloc(sizeof(struct _MusicNode));
    strcpy(node->file_path, name);
    strcpy(node->file_name, Music_GetFileName(music));
    get_name_and_singer(node->file_name, node->name, node->singer, 0);
    node->music = NULL;
    node->lyric = NULL;
    node->time = Music_GetTimeLength(music);
    node->minute = node->time / 1000 / 60;
    node->second = node->time / 1000 % 60;
    Music_Delete(music);
    return node;
}

MusicNode MusicNode_CreateNodeByJsonData(cJSON *data)
{
    MusicNode node = (MusicNode)malloc(sizeof(struct _MusicNode));
    cJSON *c_name = cJSON_GetObjectItem(data, "name");
    cJSON *c_singer = cJSON_GetObjectItem(data, "singer");
    cJSON *c_time = cJSON_GetObjectItem(data, "time");
    cJSON *c_url = cJSON_GetObjectItem(data, "url");
    cJSON *c_lrc = cJSON_GetObjectItem(data, "lrc");

    if (c_name && c_singer && c_time && c_url && c_url && node) {
        int i;
        strcpy(node->name, c_name->valuestring);
        strcpy(node->singer, c_singer->valuestring);
        strcpy(node->name_url, c_url->valuestring);
        for (i = 0; node->singer[i]; i++)
            if (node->singer[i] == '/')
                node->singer[i] = ',';
        strcpy(node->lyric_url, c_lrc->valuestring);
        node->time = c_time->valueint * 1000;
        node->minute = node->time / 1000 / 60;
        node->second = node->time / 1000 % 60;
        node->music = NULL;
        node->lyric = NULL;
    }

    return node;
}


void MusicList_Free(MusicList list)
{
    MusicNode node, tmp;
    if (list == NULL)
        return;

    for (node = list->first; node; ) {
        Music_Delete(node->music);
        Lyric_Free(node->lyric);
        tmp = node;
        node = node->next;
        free(tmp);
    }
    free(list);
}
