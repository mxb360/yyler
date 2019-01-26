#if defined(_WIN32) || defined(_WIN64)

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "Console.h"

#include <stdio.h>
#include <Windows.h>
#include <conio.h>

static int _is_console_init;
static COORD _origin;  
static COORD _axis;
static Color _raw_color;
static int _use_color;
static HANDLE hOut;

static const char *color_str[] = {
    "Black", "Blue", "Green", "Aqua",
    "Red", "Purple", "Yellow", "White",
    "Gray", "LightBlue", "LightGreen", "LightAqua",
    "LightRed", "LightPurple", "LightYellow", "LightWhite",
};

/* 重置控制台的各个属性
 */
void Console_Reset(void)
{
    _origin.X = _origin.Y = 0;
    _axis.X = _axis.Y = 1;

    hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (!_is_console_init)
    {
        _use_color = 1;
        CONSOLE_SCREEN_BUFFER_INFO bInfo;
        GetConsoleScreenBufferInfo(hOut, &bInfo);
        _raw_color = bInfo.wAttributes;
    }
    _is_console_init = 1;
}

/* 重置控制台的颜色为原始颜色
 */
void Console_ResetColor(void)
{
    if (!_is_console_init)
        Console_Reset();
    if (_use_color)
        SetConsoleTextAttribute(hOut, _raw_color);
}

void Console_UseColor(int use_color)
{
    if (!_is_console_init)
        Console_Reset();
    _use_color = use_color;
}

/* 获取当前光标的坐标
 */
void Console_GetCurPos(int *x, int *y)
{
    CONSOLE_SCREEN_BUFFER_INFO bInfo;

    if (!_is_console_init)
        Console_Reset();
    GetConsoleScreenBufferInfo(hOut, &bInfo);
    *x = _origin.X + _axis.X * bInfo.dwCursorPosition.X;
    *y = _origin.Y + _axis.Y * bInfo.dwCursorPosition.Y;
}

/* 获取当前屏幕所设置的前景色
 */
Color Console_GetBlackColor(void)
{
    CONSOLE_SCREEN_BUFFER_INFO bInfo;

    if (!_is_console_init)
        Console_Reset();
    GetConsoleScreenBufferInfo(hOut, &bInfo);
    return bInfo.wAttributes >> 4;
}

/* 获取当前屏幕所设置的前景色
 */
Color Console_GetFrontColor(void)
{
    CONSOLE_SCREEN_BUFFER_INFO bInfo;

    if (!_is_console_init)
        Console_Reset();
    GetConsoleScreenBufferInfo(hOut, &bInfo);
    return bInfo.wAttributes & 0xF;
}

/* 获取控制台的默认背景色
 */
Color Console_GetDefaultBlackColor(void)
{
    if (!_is_console_init)
        Console_Reset();
    return _raw_color >> 4;
}

/* 获取控制台的默认前景色
 */
Color Console_GetDefaultFrontColor(void)
{
    if (!_is_console_init)
        Console_Reset();
    return _raw_color & 0x0f;
}

/* 获取逻辑坐标原点的绝对坐标位置
 * (x, y)为逻辑坐标原点的绝对坐标
 * 默认的原点为(0, 0)
 */
void Console_GetOrigin(int *x, int *y)
{
    if (!_is_console_init)
        Console_Reset();
    *x = _origin.X;
    *y = _origin.Y;
}

/* 获取逻辑x, y坐标轴的单位长度和方向
 * dx, dy分别表示x, y坐标轴的单位长度
 * 如果值为负，表示逻辑坐标的方向与绝对坐标方向相反
 * 默认的值为(1, 1)
 */
void Console_GetAxis(int *dx, int *dy)
{
    if (!_is_console_init)
        Console_Reset();
    *dx = _axis.X;
    *dy = _axis.Y;
}

/* 移动光标到逻辑坐标(x, y)处
 * 此函数调用之后，程序的便从(x, y)处开始输出
 */
void Console_SetCurPos(int x, int y)
{
    COORD coord;

    if (!_is_console_init)
        Console_Reset();
    coord.X = _origin.X + x * _axis.X;
    coord.Y = _origin.Y + y * _axis.Y;
    SetConsoleCursorPosition(hOut, coord);
}

/* 设置当前屏幕所设置的前景色为color
 */
void Console_SetFrontColor(Color color)
{
    CONSOLE_SCREEN_BUFFER_INFO bInfo;

    if (!_is_console_init)
        Console_Reset();
    if (_use_color) {
        GetConsoleScreenBufferInfo(hOut, &bInfo);
        SetConsoleTextAttribute(hOut, (bInfo.wAttributes & 0xf0) | (color & 0x0f));
    }
}

/* 设置当前屏幕所设置的背景色为color
 */
void Console_SetBlackColor(Color color)
{
    CONSOLE_SCREEN_BUFFER_INFO bInfo;

    if (!_is_console_init)
        Console_Reset();
    if (_use_color) {
        GetConsoleScreenBufferInfo(hOut, &bInfo);
        SetConsoleTextAttribute(hOut, (bInfo.wAttributes & 0x0f) | (color << 4));
    }
}

/* 设置逻辑坐标原点的绝对坐标位置
 * (x, y)为逻辑坐标原点的绝对坐标
 * 默认的原点为(0, 0)
 */
void Console_SetOrigin(int x, int y)
{
    if (!_is_console_init)
        Console_Reset();
    _origin.X = x;
    _origin.Y = y;
}

/* 设置逻辑x, y坐标轴的单位长度和方向(可以是负数)
 * dx, dy分别表示x, y坐标轴的单位长度
 * 如果值为负，表示逻辑坐标的方向与绝对坐标方向相反
 * 默认的值为(1, 1)
 */
void Console_SetAxis(int dx, int dy)
{
    if (!_is_console_init)
        Console_Reset();
    _axis.X = dx;
    _axis.Y = dy;
}

/* 判断当前是否有按键按下
 * 如果有，函数返回1，否则返回0
 * 此函数通常与函数get_key()配对使用
 */
int Console_HaveKey(void)
{
    return !!_kbhit();
}

/* 获取当前的按键
 * _case： 是否区分字母的大小写，0表示区分，否则不区分
 * 如果不区分大小写，对于所有字母，得到的都将是大写字母的ascii码
 * 返回字符对应的ascii码
 * 但是对于方向键，返回的是负数(KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT)
 * 对于部分不可打印的可用的ascii码，提供了对应的宏使用(KEY_BACK, KEY_ESC, KEY_TAB, ...)
 * 如果当前没有键按下，此函数会一直等待，直到用户按下有效的键
 * 如果不想等待，你可以先调用函数have_key()判断当前是否有按键，如果有才调用此函数
 * 当调用此函数时，屏幕上不会有任何显示
 */
int Console_GetKey(int _case)
{
    int key;

    if (!_is_console_init)
        Console_Reset();

//  while (1) {
    key = _getch();        /* 获取按键 */
    
    if (key == 224) {      /* 方向键 */
        switch (_getch()) {
        case 72: return KEY_UP;
        case 75: return KEY_LEFT;
        case 77: return KEY_RIGHT;
        case 80: return KEY_DOWN;
        }
    } else if (key) {              /* 其他键 */
        if (_case)
            return toupper(key);  /* 如果忽略大小写，对于字母，都转为大写 */
        else
            return key;
    }
//}                                 /* 如果得到0值，认为是无效按键，重新获取 */
    return key;
}

/* 使程序暂停下来，显示提示字符串str后，按下任意键后继续
 * 如果str为NULL则显示系统默认的字符，一般是： 请按任意键继续. . .
 * 如果什么也不想显示，str可以为空串""
 */
void Console_Pause(const char *prompt)
{
    if (!_is_console_init)
        Console_Reset();
    if (prompt == NULL)
        system("pause");
    else {
        printf(prompt);
        system("pause>nul");
    }
}

/* 清屏
 * 清除屏幕上显示的所有文字，并将光标移动至窗口坐标原点
 */
void Console_Clear(void)
{
    if (!_is_console_init)
        Console_Reset();
    system("cls");
}

/* 从控制台获取密码
 * 将密码字符串存入到buf中，最多允许输入size字节
 * echo_mode:
 *    ECHO_NOTHING: 不回显输入的字符
 *    ECHO_STAR:    将输入的字符全用*回显
 *    ECHO_CHAR:    回显输入的字符
 * 返回buf指针
 */
char *Console_GetPassword(char *buf, char size, int echo_mode)
{
    int top = 0, ch = -1;
    while (1)
    {
        ch = Console_GetKey(MATCH_CASE);
        if (ch >= ' ' && top < size)
        {
            buf[top++] = ch;
            if (echo_mode == ECHO_STAR)
                putchar('*');
            else if (echo_mode == ECHO_CHAR)
                putchar(ch);
        } else if (ch == '\b' && top) {
            top--;
            if (echo_mode == ECHO_CHAR || echo_mode == ECHO_STAR)
                printf("\b \b");
        } else if (ch == '\n' || ch == '\r')
            break;
    }
    putchar('\n');
    buf[top] = '\0';
    return buf;
}

/* 将color对应的颜色代码转化为字符串
 * 如果color非法，返回空串
 */
const char *Console_ColorToString(Color color)
{
    if (color < 16)
        return color_str[color];
    else
        return "";
}

/* 将UTF-8字符串转化为GBK字符串
 * 可用于在控制台中显示UTF-8编码的文件的内容
 */
char *Console_UTF8ToGBK(char *gbkString, const char *utf8String)
{
    wchar_t *unicodeStr = NULL;
    int nRetLen = 0;
    nRetLen = MultiByteToWideChar(CP_UTF8, 0, utf8String, -1, NULL, 0);
    unicodeStr = (wchar_t *)malloc(nRetLen * sizeof(wchar_t));
    nRetLen = MultiByteToWideChar(CP_UTF8, 0, utf8String, -1, unicodeStr, nRetLen);
    nRetLen = WideCharToMultiByte(CP_ACP, 0, unicodeStr, -1, NULL, 0, NULL, 0);
    nRetLen = WideCharToMultiByte(CP_ACP, 0, unicodeStr, -1, gbkString, nRetLen, NULL, 0);
    free(unicodeStr);
    return gbkString;
}

/* 将GBK字符串转化为UTF-8字符串
 * 可用于在控制台中显示UTF-8编码的文件的内容
 */
char *Console_GBKToUTF8(char *utf8String, const char *gbkString)
{
    int len = MultiByteToWideChar(CP_ACP, 0, gbkString, -1, NULL, 0);
    wchar_t* wstr = malloc(sizeof(wchar_t) * (len + 1));
    MultiByteToWideChar(CP_ACP, 0, gbkString, -1, wstr, len);
    len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, utf8String, len, NULL, NULL);
    free(wstr);
    return utf8String;
}


#endif  /* defined(_WIN32) || defined(_WIN64) */
