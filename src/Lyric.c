#include "Lyric.h"

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/* 依据时间比较两段歌词的大小，用于对歌词段的排序 */
static int __lyric_words_compare(const void *l1, const void *l2)
{
    return ((LyricWords *)l1)->time - ((LyricWords *)l2)->time;
}

/* 歌词解析器，从文件file中解析歌词保存到LyricFile中 */
static int lyric_Lexer(Lyric lyric, FILE *file)
{
    char buf[256];
    unsigned long time1, time2, time3; /* 歌词文件格式 [time1:time2.time3][...] ... */
    int n, start_time, end_time;
    //int offset = 0;

    if (!lyric || !file)
        return -1;

    lyric->lyric_cnt = 1;
    lyric->currect_lyric = 0;

    lyric->lyric = (LyricWords *)malloc(sizeof(struct _LyricWords) * MAX_LYRIC_LINE);
    lyric->lyric[0].time = 0;
    lyric->lyric[0].words = ""; 
    lyric->__lyric_word_bak_cnt = 0;
    lyric->__lyric_word_bak__ = (char **)malloc(sizeof(char *) * MAX_LYRIC_LINE);

    while (fgets(buf, sizeof buf, file) && lyric->lyric_cnt <= MAX_LYRIC_LINE) {
        n = 0;
        start_time = lyric->lyric_cnt;   /* 首个[]对的位置 */
        
        while (1) {                      /* 解析当前行歌词的时间，如果出错，忽略该行 */
            time1 = time2 = time3 = 0;
            if (buf[n++] != '[')
                break;
            while (isdigit(buf[n]))  /* 解析分 */
                time1 = time1 * 10 + (buf[n++] - '0');
            if (buf[n++] != ':' && !isdigit(buf[n]))
                break;
            while (isdigit(buf[n]))  /* 解析秒 */
                time2 = time2 * 10 + (buf[n++] - '0');
            if (buf[n] != ']' && buf[n++] != '.' && !isdigit(buf[n]))
                break;
            while (isdigit(buf[n]))  /* 解析毫秒 */
                time3 = time3 * 10 + (buf[n++] - '0');
            if (buf[n++] != ']')
                break;
            /* 计算总时长(毫秒) */
            time3 = time3 < 99 ? time3 * 10 : time3;  /* 区别time3的两位情况和三位情况 */
            lyric->lyric[lyric->lyric_cnt].time = time1 * 60000 + time2 * 1000 + time3;
            lyric->lyric_cnt++;
            /* 如果之后没有[]对，结束[]对的解析 */
            if (buf[n] != '[')
                break;
        }

        end_time = lyric->lyric_cnt;   /* 最后一个[]对之后的位置 */
        if (start_time < end_time) {   /* 绑定所有的[]对对应的歌词字符串 */
            char *word;
            int i, len;

            len = strlen(buf + n);
            word = (char *)malloc(len + 1);
            strcpy(word, buf + n);
            if (word[len - 1] == '\n' || word[len - 1] == '\r')
                word[len - 1] = '\0';
            if (word[len - 2] == '\n' || word[len - 2] == '\r')
                word[len - 2] = '\0';
            for (i = start_time; i < end_time; i++) 
                lyric->lyric[i].words = word;
            lyric->__lyric_word_bak__[lyric->__lyric_word_bak_cnt++] = word;
        }
     }

    lyric->lyric = realloc(lyric->lyric, sizeof(LyricWords) * lyric->lyric_cnt);
    lyric->__lyric_word_bak__ = realloc(lyric->__lyric_word_bak__, sizeof(char *) * lyric->__lyric_word_bak_cnt);
    /* 将生成的歌词按时间排序 */
    qsort(lyric->lyric, lyric->lyric_cnt, sizeof(LyricWords), __lyric_words_compare);

    return 0;
}

/* 打印解析后的歌词 */
void Lyric_Print(Lyric lyric)
{
    int i;
    int t, t1, t2, t3;

    if (lyric) {
        for (i = 0; i < lyric->lyric_cnt; i++) {
            t = lyric->lyric[i].time;
            t1 = t / 1000 / 60;
            t2 = t / 1000 % 60;
            t3 = t / 10 % 100;
            printf("%02d分%02d.%02d秒(%d): %s\n", t1, t2, t3, t, lyric->lyric[i].words);
        }
    }
}

/* 释放Lyric所申请的所有内存 */
void Lyric_Free(Lyric lyric)
{
    if (lyric) {
        int i;
        for (i = 0; i < lyric->__lyric_word_bak_cnt; i++) 
            free(lyric->__lyric_word_bak__[i]);
        free(lyric->lyric);
        free(lyric->__lyric_word_bak__);
        free(lyric);
    }
}

/* Lyric_GotoCurrentTime
 * 设置当前应该显示的歌词的时间
 * 此函数调用成功后，当前显示的歌词将变为设current_time对应的歌词
 * lyric:         存放歌词内容的指针
 * current_time： 待设置的时间，通常是当前音乐的播放位置
 * 返回：         成功返回0，出错返回-1 
 */
int Lyric_GotoCurrentTime(Lyric lyric, unsigned long current_time)
{
    int i = 0;
    if (!lyric)
        return -1;
    while (i < lyric->lyric_cnt && lyric->lyric[i].time + lyric->offset < current_time)
        i++;
    lyric->currect_lyric = i ? i - 1 : 0;
    return 0;
}

/* Lyric_ShouldBeNext
 * 判断当前时间是否应该显示下一句歌词
 * 即需不需要更新歌词（下移当前歌词指针）
 * 如果是，更新当前时间段（下移当前歌词指针）并返回1
 * 否则返回0
 * 出错返回-1 
 */
int Lyric_ShouldBeNext(Lyric lyric, unsigned long currect_time)
{
    int flag = 0;
    if (!lyric)
        return -1;
    while (lyric->currect_lyric + 1 < lyric->lyric_cnt && 
           currect_time >= lyric->lyric[lyric->currect_lyric + 1].time + lyric->offset)
    {
        lyric->currect_lyric++;
        flag = 1;
    }
    return flag;
}

/* Lyric_GetLyric
 * 返回相对于当前应该显示的歌词的第line行歌词
 * lyric:   存放歌词内容的指针
 * line:    先对于当前歌词的偏移行数，为0表示当前行，
 *          负数表示前面的行，正数表示后面的行
 * 返回：   对应行的歌词，如果没有对应的行，返回空串（即""）
 *          出错返回空串 
 */
const char *Lyric_GetLyric(Lyric lyric, int line)
{
    int _line;
    if (!lyric)
        return "";
    _line = lyric->currect_lyric + line;
    if (_line >= 0 && _line < lyric->lyric_cnt)
        return lyric->lyric[_line].words;
    return "";
}

/* 从文件中获取歌词对象，会解析文件内容，生成LyricFile对象 */
Lyric Lyric_GetLyricByFile(const char *file_name)
{
    Lyric lyric;
    FILE *file;

    lyric = (Lyric)malloc(sizeof(struct _Lyric));
    if (lyric) {
        lyric->file_name = file_name;
        lyric->offset = 0;
        if ((file = fopen(file_name, "r")) == NULL || lyric_Lexer(lyric, file)) {
            free(lyric);
            lyric = NULL;
        }
        if (file)
            fclose(file);
    }
    return lyric;
}
