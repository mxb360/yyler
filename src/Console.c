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

/* ���ÿ���̨�ĸ�������
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

/* ���ÿ���̨����ɫΪԭʼ��ɫ
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

/* ��ȡ��ǰ��������
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

/* ��ȡ��ǰ��Ļ�����õ�ǰ��ɫ
 */
Color Console_GetBlackColor(void)
{
    CONSOLE_SCREEN_BUFFER_INFO bInfo;

    if (!_is_console_init)
        Console_Reset();
    GetConsoleScreenBufferInfo(hOut, &bInfo);
    return bInfo.wAttributes >> 4;
}

/* ��ȡ��ǰ��Ļ�����õ�ǰ��ɫ
 */
Color Console_GetFrontColor(void)
{
    CONSOLE_SCREEN_BUFFER_INFO bInfo;

    if (!_is_console_init)
        Console_Reset();
    GetConsoleScreenBufferInfo(hOut, &bInfo);
    return bInfo.wAttributes & 0xF;
}

/* ��ȡ����̨��Ĭ�ϱ���ɫ
 */
Color Console_GetDefaultBlackColor(void)
{
    if (!_is_console_init)
        Console_Reset();
    return _raw_color >> 4;
}

/* ��ȡ����̨��Ĭ��ǰ��ɫ
 */
Color Console_GetDefaultFrontColor(void)
{
    if (!_is_console_init)
        Console_Reset();
    return _raw_color & 0x0f;
}

/* ��ȡ�߼�����ԭ��ľ�������λ��
 * (x, y)Ϊ�߼�����ԭ��ľ�������
 * Ĭ�ϵ�ԭ��Ϊ(0, 0)
 */
void Console_GetOrigin(int *x, int *y)
{
    if (!_is_console_init)
        Console_Reset();
    *x = _origin.X;
    *y = _origin.Y;
}

/* ��ȡ�߼�x, y������ĵ�λ���Ⱥͷ���
 * dx, dy�ֱ��ʾx, y������ĵ�λ����
 * ���ֵΪ������ʾ�߼�����ķ�����������귽���෴
 * Ĭ�ϵ�ֵΪ(1, 1)
 */
void Console_GetAxis(int *dx, int *dy)
{
    if (!_is_console_init)
        Console_Reset();
    *dx = _axis.X;
    *dy = _axis.Y;
}

/* �ƶ���굽�߼�����(x, y)��
 * �˺�������֮�󣬳���ı��(x, y)����ʼ���
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

/* ���õ�ǰ��Ļ�����õ�ǰ��ɫΪcolor
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

/* ���õ�ǰ��Ļ�����õı���ɫΪcolor
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

/* �����߼�����ԭ��ľ�������λ��
 * (x, y)Ϊ�߼�����ԭ��ľ�������
 * Ĭ�ϵ�ԭ��Ϊ(0, 0)
 */
void Console_SetOrigin(int x, int y)
{
    if (!_is_console_init)
        Console_Reset();
    _origin.X = x;
    _origin.Y = y;
}

/* �����߼�x, y������ĵ�λ���Ⱥͷ���(�����Ǹ���)
 * dx, dy�ֱ��ʾx, y������ĵ�λ����
 * ���ֵΪ������ʾ�߼�����ķ�����������귽���෴
 * Ĭ�ϵ�ֵΪ(1, 1)
 */
void Console_SetAxis(int dx, int dy)
{
    if (!_is_console_init)
        Console_Reset();
    _axis.X = dx;
    _axis.Y = dy;
}

/* �жϵ�ǰ�Ƿ��а�������
 * ����У���������1�����򷵻�0
 * �˺���ͨ���뺯��get_key()���ʹ��
 */
int Console_HaveKey(void)
{
    return !!_kbhit();
}

/* ��ȡ��ǰ�İ���
 * _case�� �Ƿ�������ĸ�Ĵ�Сд��0��ʾ���֣���������
 * ��������ִ�Сд������������ĸ���õ��Ķ����Ǵ�д��ĸ��ascii��
 * �����ַ���Ӧ��ascii��
 * ���Ƕ��ڷ���������ص��Ǹ���(KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT)
 * ���ڲ��ֲ��ɴ�ӡ�Ŀ��õ�ascii�룬�ṩ�˶�Ӧ�ĺ�ʹ��(KEY_BACK, KEY_ESC, KEY_TAB, ...)
 * �����ǰû�м����£��˺�����һֱ�ȴ���ֱ���û�������Ч�ļ�
 * �������ȴ���������ȵ��ú���have_key()�жϵ�ǰ�Ƿ��а���������вŵ��ô˺���
 * �����ô˺���ʱ����Ļ�ϲ������κ���ʾ
 */
int Console_GetKey(int _case)
{
    int key;

    if (!_is_console_init)
        Console_Reset();

//  while (1) {
    key = _getch();        /* ��ȡ���� */
    
    if (key == 224) {      /* ����� */
        switch (_getch()) {
        case 72: return KEY_UP;
        case 75: return KEY_LEFT;
        case 77: return KEY_RIGHT;
        case 80: return KEY_DOWN;
        }
    } else if (key) {              /* ������ */
        if (_case)
            return toupper(key);  /* ������Դ�Сд��������ĸ����תΪ��д */
        else
            return key;
    }
//}                                 /* ����õ�0ֵ����Ϊ����Ч���������»�ȡ */
    return key;
}

/* ʹ������ͣ��������ʾ��ʾ�ַ���str�󣬰�������������
 * ���strΪNULL����ʾϵͳĬ�ϵ��ַ���һ���ǣ� �밴���������. . .
 * ���ʲôҲ������ʾ��str����Ϊ�մ�""
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

/* ����
 * �����Ļ����ʾ���������֣���������ƶ�����������ԭ��
 */
void Console_Clear(void)
{
    if (!_is_console_init)
        Console_Reset();
    system("cls");
}

/* �ӿ���̨��ȡ����
 * �������ַ������뵽buf�У������������size�ֽ�
 * echo_mode:
 *    ECHO_NOTHING: ������������ַ�
 *    ECHO_STAR:    ��������ַ�ȫ��*����
 *    ECHO_CHAR:    ����������ַ�
 * ����bufָ��
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

/* ��color��Ӧ����ɫ����ת��Ϊ�ַ���
 * ���color�Ƿ������ؿմ�
 */
const char *Console_ColorToString(Color color)
{
    if (color < 16)
        return color_str[color];
    else
        return "";
}

/* ��UTF-8�ַ���ת��ΪGBK�ַ���
 * �������ڿ���̨����ʾUTF-8������ļ�������
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

/* ��GBK�ַ���ת��ΪUTF-8�ַ���
 * �������ڿ���̨����ʾUTF-8������ļ�������
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
