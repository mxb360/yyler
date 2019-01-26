#ifndef __CONSOLE_H__
#define __CONSOLE_H__

typedef unsigned short Color;

/* ��ɫ��أ���֧��16����ɫ�� */
#define COLOR_BLACK         0    /* ��ɫ */
#define COLOR_BLUE          1    /* ��ɫ */
#define COLOR_GREEN         2    /* ��ɫ */
#define COLOR_AQUA          3    /* ǳ��ɫ */
#define COLOR_RED           4    /* ��ɫ */
#define COLOR_PURPLE        5    /* ��ɫ */
#define COLOR_YELLOW        6    /* ��ɫ */
#define COLOR_WHITE         7    /* ��ɫ */
#define COLOR_GRAY          8    /* ��ɫ */
#define COLOR_LIGHT_BLUE    9    /* ����ɫ */
#define COLOR_LIGHT_GREEN   10   /* ����ɫ */
#define COLOR_LIGHT_AQUA    11   /* ��ǳ��ɫ */
#define COLOR_LIGHT_RED     12   /* ����ɫ */
#define COLOR_LIGHT_PURPLE  13   /* ����ɫ */
#define COLOR_LIGHT_YELLOW  14   /* ����ɫ */
#define COLOR_BRIGHT_WHITE  15   /* ����ɫ */

/* ������ */
#define KEY_DOWN  -1       // ����� ��  
#define KEY_UP    -2       // ����� ��
#define KEY_LEFT  -3       // ����� ��
#define KEY_RIGHT -4       // ����� ��
#define KEY_BACK  8        // �˸�� '\b'
#define KEY_ENTER 13       // �س��� '\r'
#define KEY_ESC   27       // Esc��
#define KEY_TAB   9        // Tab�� '\t'
#define KEY_SPACE 32       // �ո�� ' ' 

#define MATCH_CASE    0
#define NO_MATCH_CASE 1

/* �������ģʽ */
#define ECHO_NOTHING   0
#define ECHO_STAR      1
#define ECHO_CHAR      2

#ifdef __cplusplus
extern "C" {
#endif 

void  Console_GetCurPos(int *x, int *y);
Color Console_GetBlackColor(void);
Color Console_GetFrontColor(void);
Color Console_GetDefaultBlackColor(void);
Color Console_GetDefaultFrontColor(void);
void  Console_GetOrigin(int *x, int *y);
void  Console_GetAxis(int *dx, int *dy);
void  Console_SetCurPos(int x, int y);
void  Console_SetFrontColor(Color color);
void  Console_SetBlackColor(Color color);
void  Console_SetOrigin(int x, int y);
void  Console_SetAxis(int dx, int dy);
int   Console_HaveKey(void);
int   Console_GetKey(int _case);
void  Console_Pause(const char *prompt);
void  Console_Clear(void);
void  Console_Reset(void);
void  Console_ResetColor(void);
void  Console_UseColor(int use_color);
char *Console_GetPassword(char *buf, char size, int echo_mode);
const char *Console_ColorToString(Color color);
char *Console_UTF8ToGBK(char *gbkString, const char *utf8String);
char *Console_GBKToUTF8(char *utf8String, const char *gbkString);

#ifdef __cplusplus
}
#endif 

#endif  /* !__CONSOLE_H__ */
