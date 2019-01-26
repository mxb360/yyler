#ifndef __MUSIC_H__
#define __MUSIC_H__

#define SHOW_MUSIC_STATS 0
#define SHOW_ERROR       0

#define MUSIC_NOT_READY  0
#define MUSIC_PAUSED     1
#define MUSIC_PLAYING    2
#define MUSIC_STOPPED    3

#define MUSIC_ERROR     -1
#define MUSIC_OK         0

typedef unsigned long MTLen;
typedef struct _Music *Music;

#if 0
extern char _music_retbuf[];
int Music_SendString(const char *cmd, const char *device, const char *arg);
int Music_Control(Music music, const char *cmd, const char *arg, int show);
#endif

#ifdef __cplusplus
extern "C" {
#endif 

Music Music_Create(const char *file_name);
int   Music_Play(Music music, MTLen start, MTLen end, int repeat);
void  Music_Delete(Music music);
int   Music_Pause(Music music);
int   Music_Resume(Music music);
int   Music_Stop(Music music);
MTLen Music_GetTimeLength(Music music);
MTLen Music_GetCurrentTimeLength(Music music);
int   Music_SetVolume(Music music, int volume);
int   Music_GetVolume(Music music);
int   Music_GetStatus(Music music);
const char *Music_GetFileName(Music music);
const char *Music_GetLastError(void);
const char *Music_StatusToString(int status);

#ifdef __cplusplus
}
#endif 

#endif  /* !__MUSIC_H__ */
