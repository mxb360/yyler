#ifndef __LYRIC_H__
#define __LYRIC_H__

#define MAX_LYRIC_LINE   1000
#define MAX_LYRIC_TIME   0xffffffff

typedef struct _LyricWords {
    unsigned long time;
    const char *words;
} LyricWords;

typedef struct _Lyric {
    const char *file_name;
    LyricWords *lyric;
    int    lyric_cnt;
    int    currect_lyric;
    int    offset;
    char **__lyric_word_bak__;
    int    __lyric_word_bak_cnt;
} *Lyric;

Lyric Lyric_GetLyricByFile(const char *file);
void  Lyric_Print(Lyric lyric);
void  Lyric_Free(Lyric lyric);
int   Lyric_GotoCurrentTime(Lyric lyric, unsigned long current_time);
int   Lyric_ShouldBeNext(Lyric lyric, unsigned long currect_time);
const char *Lyric_GetLyric(Lyric lyric, int line);

#endif  /* !__LYRIC_H__ */
