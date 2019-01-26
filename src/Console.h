#ifndef __CONSOLE_H__
#define __CONSOLE_H__

typedef unsigned short Color;

/* 颜色相关（仅支持16种颜色） */
#define COLOR_BLACK         0    /* 黑色 */
#define COLOR_BLUE          1    /* 蓝色 */
#define COLOR_GREEN         2    /* 绿色 */
#define COLOR_AQUA          3    /* 浅绿色 */
#define COLOR_RED           4    /* 红色 */
#define COLOR_PURPLE        5    /* 紫色 */
#define COLOR_YELLOW        6    /* 黄色 */
#define COLOR_WHITE         7    /* 白色 */
#define COLOR_GRAY          8    /* 灰色 */
#define COLOR_LIGHT_BLUE    9    /* 淡蓝色 */
#define COLOR_LIGHT_GREEN   10   /* 淡绿色 */
#define COLOR_LIGHT_AQUA    11   /* 淡浅绿色 */
#define COLOR_LIGHT_RED     12   /* 淡红色 */
#define COLOR_LIGHT_PURPLE  13   /* 淡紫色 */
#define COLOR_LIGHT_YELLOW  14   /* 淡黄色 */
#define COLOR_BRIGHT_WHITE  15   /* 亮白色 */

/* 按键码 */
#define KEY_DOWN  -1       // 方向键 上  
#define KEY_UP    -2       // 方向键 下
#define KEY_LEFT  -3       // 方向键 左
#define KEY_RIGHT -4       // 方向键 右
#define KEY_BACK  8        // 退格键 '\b'
#define KEY_ENTER 13       // 回车键 '\r'
#define KEY_ESC   27       // Esc键
#define KEY_TAB   9        // Tab键 '\t'
#define KEY_SPACE 32       // 空格键 ' ' 

#define MATCH_CASE    0
#define NO_MATCH_CASE 1

/* 密码回显模式 */
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
