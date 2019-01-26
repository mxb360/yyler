#if defined(_WIN32) || defined(_WIN64)

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "Music.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>

#ifdef _MSC_VER
#pragma comment(lib, "winmm.lib")
#endif 

static int  _music_id;
static char _music_cmd[256];
static char _music_err[256];
       char _music_retbuf[256];
static char _music_argbuf[256];

struct _Music
{
    char file_name[256];
    char device[50];
};

static const char *_music_status_str[] = {
    "NotReady", "Paused", "Playing", "Stopped",
};

static char *get_file_name(char *path)
{
    int i = strlen(path);
    //while (--i >= 0 && path[i] != '.');
    //if (i > 0)
    //    path[i] = 0;
    while (--i >= 0 && path[i] != '\\' && path[i] != '/');
    if (i > 0)
        strcpy(path, path + i + 1);
    return path;
}

/* 对mciSendString的一个简易封装
 * - 私有函数
 */
int Music_SendString(const char *cmd, const char *device, const char *arg)
{
    MCIERROR error;

    sprintf(_music_cmd, "%s %s %s", cmd, device, arg);
    if ((error = mciSendString(_music_cmd, _music_retbuf, sizeof _music_retbuf, NULL)))
    {
        mciGetErrorString(error, _music_err, sizeof _music_err);
#if SHOW_ERROR
        fprintf(stderr, "Music %s Error: %s\n", cmd, _music_err);
#endif
        return MUSIC_ERROR;
    }
    strcpy(_music_err, "OK");
    return MUSIC_OK;
}

/* 控制mciSendString
 * - 私有函数
 */
int Music_Control(Music music, const char *cmd, const char *arg, int show)
{
    if (music == NULL)
    {
        strcpy(_music_err, "NULL Music object");
        fprintf(stderr, "Music %s Error: %s\n", cmd, _music_err);
        return MUSIC_ERROR;
    }
    
    if (Music_SendString(cmd, music->device, arg))
        return MUSIC_ERROR;
    if (show)
        fprintf(stdout, "Music: %s music: \"%s\"\n", cmd, music->file_name);
    return MUSIC_OK;
}

/* 创建一个音乐对象并打开对应的音乐文件
 * file_name:  音乐文件的路径
 * return：    创建成功返回Music指针，失败返回NULL
 */
Music Music_Create(const char *file_name)
{
    Music music = (Music)malloc(sizeof(struct _Music));

    if (music)
    {   
        strcpy(music->file_name, file_name);
        get_file_name(music->file_name);
        sprintf(music->device, "_M_-%d-", _music_id++);
        sprintf(_music_argbuf, "\"%s\" alias %s", file_name, music->device);
        if (Music_SendString("open", "", _music_argbuf))
        {
            free(music);
            music = NULL;
        }
    }
    return music;
}

/* 开始播放音乐
 * music:    Music指针
 * start:    播放的起始位置（毫秒）,如果为0，从头开始播放
 * end:      播放结束位置（毫秒），如果为0，播放到结束]
 * repeat:   播放完成后是否从新开始播放
 * return:   成功返回0，出错返回-1
 */
int Music_Play(Music music, MTLen start, MTLen end, int repeat)
{
    char buf[50];

    start ? sprintf(buf, "from %ld ", start) : sprintf(buf, " ");
    strcpy(_music_argbuf, buf);
    end ? sprintf(buf, "to %ld ", end) : sprintf(buf, " ");
    strcat(_music_argbuf, buf);
    repeat ? strcat(_music_argbuf, "repeat") : 0;
    return Music_Control(music, "play", _music_argbuf, SHOW_MUSIC_STATS);
}

/* 删除（释放）Music指针
 * music:    Music指针
 */
void Music_Delete(Music music)
{
    if (music)
        Music_Control(music, "close", "", SHOW_MUSIC_STATS);
    free(music);
}

/* 暂停播放音乐
 * music:    Music指针
 * return:   成功返回0，出错返回-1
 */
int Music_Pause(Music music)
{
    return Music_Control(music, "pause", "", SHOW_MUSIC_STATS);
}

/* 继续播放音乐
 * music:    Music指针
 * return:   成功返回0，出错返回-1
 */
int Music_Resume(Music music)
{
    return Music_Control(music, "resume", "", SHOW_MUSIC_STATS);
}

/* 停止播放音乐
 * music:    Music指针
 * return:   成功返回0，出错返回-1
 */
int Music_Stop(Music music)
{
    return Music_Control(music, "stop", "", SHOW_MUSIC_STATS);
}

/* 获取音乐时长（毫秒）
 * music:    Music指针
 * return:   成功返回时长，出错返回0
 */
MTLen Music_GetTimeLength(Music music)
{
    MTLen len;
    if (Music_Control(music, "status", "length", SHOW_MUSIC_STATS))
        return 0;
    sscanf(_music_retbuf, "%ld", &len);
    return len;
}

/* 获取音乐当前已播放的时长（毫秒）
 * music:    Music指针
 * return:   成功返回当前已播放的时长，出错返回0
 */
MTLen Music_GetCurrentTimeLength(Music music)
{
    MTLen len;
    if (Music_Control(music, "status", "position", SHOW_MUSIC_STATS))
        return 0;
    sscanf(_music_retbuf, "%ld", &len);
    return len;
}

/* 设置音量（0-1000）
 * music:    Music指针
 * return:   成功返回0，出错返回-1
 */
int Music_SetVolume(Music music, int volume)
{
    sprintf(_music_argbuf, "volume to %d", volume);
    return Music_Control(music, "setaudio", _music_argbuf, SHOW_MUSIC_STATS);
}

/* 获取音量
 * music:    Music指针
 * return:   音量(0-1000)，出错返回-1
 */
int Music_GetVolume(Music music)
{
    int volume;
    if (Music_Control(music, "status", "volume", SHOW_MUSIC_STATS))
        return MUSIC_ERROR;
    sscanf(_music_retbuf, "%d", &volume);
    return volume;
}

 int Music_GetStatus(Music music)
 {
     if (Music_Control(music, "status", "mode", SHOW_MUSIC_STATS))
         return MUSIC_ERROR;
     if (strstr(_music_retbuf, "not ready"))
         return MUSIC_NOT_READY;
     else if (strstr(_music_retbuf, "paused"))
         return MUSIC_PAUSED;
     else if (strstr(_music_retbuf, "stopped"))
         return MUSIC_STOPPED;
     else if (strstr(_music_retbuf, "playing"))
         return MUSIC_PLAYING;
     return MUSIC_OK;
 }

/* 获取音乐文件名
 * music:    Music指针
 * return:   音乐文件名，出错返回空串
 */
const char *Music_GetFileName(Music music)
{
    if (music)
        return music->file_name;
    strcpy(_music_err, "NULL Music object");
    return "";
}


/* 获取上一个Music函数的执行错误描述字符串
 * 如果上一个Music函数执行正确，返回"OK"
 */
const char *Music_GetLastError(void)
{
    return _music_err;
}

const char * Music_StatusToString(int status)
{
    if (status < 0 || status > 3)
        return "";
    return _music_status_str[status];
}

#endif   /* defined(_WIN32) || defined(_WIN64) */
