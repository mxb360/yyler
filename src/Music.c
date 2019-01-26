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

/* ��mciSendString��һ�����׷�װ
 * - ˽�к���
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

/* ����mciSendString
 * - ˽�к���
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

/* ����һ�����ֶ��󲢴򿪶�Ӧ�������ļ�
 * file_name:  �����ļ���·��
 * return��    �����ɹ�����Musicָ�룬ʧ�ܷ���NULL
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

/* ��ʼ��������
 * music:    Musicָ��
 * start:    ���ŵ���ʼλ�ã����룩,���Ϊ0����ͷ��ʼ����
 * end:      ���Ž���λ�ã����룩�����Ϊ0�����ŵ�����]
 * repeat:   ������ɺ��Ƿ���¿�ʼ����
 * return:   �ɹ�����0��������-1
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

/* ɾ�����ͷţ�Musicָ��
 * music:    Musicָ��
 */
void Music_Delete(Music music)
{
    if (music)
        Music_Control(music, "close", "", SHOW_MUSIC_STATS);
    free(music);
}

/* ��ͣ��������
 * music:    Musicָ��
 * return:   �ɹ�����0��������-1
 */
int Music_Pause(Music music)
{
    return Music_Control(music, "pause", "", SHOW_MUSIC_STATS);
}

/* ������������
 * music:    Musicָ��
 * return:   �ɹ�����0��������-1
 */
int Music_Resume(Music music)
{
    return Music_Control(music, "resume", "", SHOW_MUSIC_STATS);
}

/* ֹͣ��������
 * music:    Musicָ��
 * return:   �ɹ�����0��������-1
 */
int Music_Stop(Music music)
{
    return Music_Control(music, "stop", "", SHOW_MUSIC_STATS);
}

/* ��ȡ����ʱ�������룩
 * music:    Musicָ��
 * return:   �ɹ�����ʱ����������0
 */
MTLen Music_GetTimeLength(Music music)
{
    MTLen len;
    if (Music_Control(music, "status", "length", SHOW_MUSIC_STATS))
        return 0;
    sscanf(_music_retbuf, "%ld", &len);
    return len;
}

/* ��ȡ���ֵ�ǰ�Ѳ��ŵ�ʱ�������룩
 * music:    Musicָ��
 * return:   �ɹ����ص�ǰ�Ѳ��ŵ�ʱ����������0
 */
MTLen Music_GetCurrentTimeLength(Music music)
{
    MTLen len;
    if (Music_Control(music, "status", "position", SHOW_MUSIC_STATS))
        return 0;
    sscanf(_music_retbuf, "%ld", &len);
    return len;
}

/* ����������0-1000��
 * music:    Musicָ��
 * return:   �ɹ�����0��������-1
 */
int Music_SetVolume(Music music, int volume)
{
    sprintf(_music_argbuf, "volume to %d", volume);
    return Music_Control(music, "setaudio", _music_argbuf, SHOW_MUSIC_STATS);
}

/* ��ȡ����
 * music:    Musicָ��
 * return:   ����(0-1000)��������-1
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

/* ��ȡ�����ļ���
 * music:    Musicָ��
 * return:   �����ļ����������ؿմ�
 */
const char *Music_GetFileName(Music music)
{
    if (music)
        return music->file_name;
    strcpy(_music_err, "NULL Music object");
    return "";
}


/* ��ȡ��һ��Music������ִ�д��������ַ���
 * �����һ��Music����ִ����ȷ������"OK"
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
