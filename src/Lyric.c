#include "Lyric.h"

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/* ����ʱ��Ƚ����θ�ʵĴ�С�����ڶԸ�ʶε����� */
static int __lyric_words_compare(const void *l1, const void *l2)
{
    return ((LyricWords *)l1)->time - ((LyricWords *)l2)->time;
}

/* ��ʽ����������ļ�file�н�����ʱ��浽LyricFile�� */
static int lyric_Lexer(Lyric lyric, FILE *file)
{
    char buf[256];
    unsigned long time1, time2, time3; /* ����ļ���ʽ [time1:time2.time3][...] ... */
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
        start_time = lyric->lyric_cnt;   /* �׸�[]�Ե�λ�� */
        
        while (1) {                      /* ������ǰ�и�ʵ�ʱ�䣬����������Ը��� */
            time1 = time2 = time3 = 0;
            if (buf[n++] != '[')
                break;
            while (isdigit(buf[n]))  /* ������ */
                time1 = time1 * 10 + (buf[n++] - '0');
            if (buf[n++] != ':' && !isdigit(buf[n]))
                break;
            while (isdigit(buf[n]))  /* ������ */
                time2 = time2 * 10 + (buf[n++] - '0');
            if (buf[n] != ']' && buf[n++] != '.' && !isdigit(buf[n]))
                break;
            while (isdigit(buf[n]))  /* �������� */
                time3 = time3 * 10 + (buf[n++] - '0');
            if (buf[n++] != ']')
                break;
            /* ������ʱ��(����) */
            time3 = time3 < 99 ? time3 * 10 : time3;  /* ����time3����λ�������λ��� */
            lyric->lyric[lyric->lyric_cnt].time = time1 * 60000 + time2 * 1000 + time3;
            lyric->lyric_cnt++;
            /* ���֮��û��[]�ԣ�����[]�ԵĽ��� */
            if (buf[n] != '[')
                break;
        }

        end_time = lyric->lyric_cnt;   /* ���һ��[]��֮���λ�� */
        if (start_time < end_time) {   /* �����е�[]�Զ�Ӧ�ĸ���ַ��� */
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
    /* �����ɵĸ�ʰ�ʱ������ */
    qsort(lyric->lyric, lyric->lyric_cnt, sizeof(LyricWords), __lyric_words_compare);

    return 0;
}

/* ��ӡ������ĸ�� */
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
            printf("%02d��%02d.%02d��(%d): %s\n", t1, t2, t3, t, lyric->lyric[i].words);
        }
    }
}

/* �ͷ�Lyric������������ڴ� */
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
 * ���õ�ǰӦ����ʾ�ĸ�ʵ�ʱ��
 * �˺������óɹ��󣬵�ǰ��ʾ�ĸ�ʽ���Ϊ��current_time��Ӧ�ĸ��
 * lyric:         ��Ÿ�����ݵ�ָ��
 * current_time�� �����õ�ʱ�䣬ͨ���ǵ�ǰ���ֵĲ���λ��
 * ���أ�         �ɹ�����0��������-1 
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
 * �жϵ�ǰʱ���Ƿ�Ӧ����ʾ��һ����
 * ���費��Ҫ���¸�ʣ����Ƶ�ǰ���ָ�룩
 * ����ǣ����µ�ǰʱ��Σ����Ƶ�ǰ���ָ�룩������1
 * ���򷵻�0
 * ������-1 
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
 * ��������ڵ�ǰӦ����ʾ�ĸ�ʵĵ�line�и��
 * lyric:   ��Ÿ�����ݵ�ָ��
 * line:    �ȶ��ڵ�ǰ��ʵ�ƫ��������Ϊ0��ʾ��ǰ�У�
 *          ������ʾǰ����У�������ʾ�������
 * ���أ�   ��Ӧ�еĸ�ʣ����û�ж�Ӧ���У����ؿմ�����""��
 *          �����ؿմ� 
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

/* ���ļ��л�ȡ��ʶ��󣬻�����ļ����ݣ�����LyricFile���� */
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
