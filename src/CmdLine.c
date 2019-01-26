#include "CmdLine.h"

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int _argc;
static char **_argv;
static const char *_name;
static char **_cmdline_argv;
static int _cmdline_cnt;

static int this_cnt;
static char long_opt_buf[256] = "--";

/* 命令行初始化 */
int CmdLine_Init(int argc, char *argv[])
{
    _argc = argc;
    _argv = argv;
    _name = argv[0];
    _cmdline_cnt = argc - 1;
    _cmdline_argv = (char **)malloc(_cmdline_cnt * sizeof(char *));
    if (_cmdline_argv == NULL)
        return -1;
    this_cnt = 0;
    memcpy(_cmdline_argv, argv + 1, _cmdline_cnt * sizeof(char *));

    return 0;
}

/* 命令行中是否有短选项 -opt
 * 如果有，返回1，否则返回0
 * 如果remov为1，会将该选项移除
 */
int CmdLine_HaveShortOpt(char opt, int remove)
{
    int i, have_opt = 0;

    for (i = 0; i < _cmdline_cnt; i++)
        if (IS_SHORT_OPT(_cmdline_argv[i]) && GET_SHORT_OPT(_cmdline_argv[i]) == opt) {
            have_opt = 1;
            if (remove)
                _cmdline_argv[i] = NULL;
        }
    return have_opt;
}

/* 命令行中是否有长选项 --opt
 * 如果有，返回1，否则返回0
 * 如果remov为1，会将该选项移除
 */
int CmdLine_HaveLongOpt(const char *opt, int remove)
{
    int i, have_opt = 0;

    for (i = 0; i < _cmdline_cnt; i++)
        if (IS_LONG_OPT(_cmdline_argv[i]) && !strcmp(GET_LONG_OPT(_cmdline_argv[i]), opt)) {
            have_opt = 1;
            if (remove)
                _cmdline_argv[i] = NULL;
        }
    return have_opt;
}

/* 释放cmdline_init申请的空间，移除所有的命令行选项和参数
 */
void CmdLine_Free(void)
{
    free(_cmdline_argv);
    _cmdline_cnt = 0;
    _cmdline_argv = NULL;
    this_cnt = 0;
}

/* 获取短选项-opt之后的参数
 * 如果opt为0，继续返回之前所设置的opt之后的参数，直到没有参数时返回空串("")
 * 如果opt不为0，返回新选项opt的下一个参数，如果没有参数返回空串("")，选项不存在返回NULL
 * 该函数会移除选项和已返回的参数
 */
const char *CmdLine_GetNextArgOfShortOpt(char opt)
{
    int i;
    static int next = -1;

    if (opt) {
        next = -1;
        for (i = 0; i < _cmdline_cnt; i++) {
            if (IS_SHORT_OPT(_cmdline_argv[i]) && GET_SHORT_OPT(_cmdline_argv[i]) == opt) {
                if (next == -1)
                    next = i + 1;
                _cmdline_argv[i] = NULL;
            }
        }
    }

    if (next == -1)
        return NULL;
    if (next < _cmdline_cnt && IS_ARG(_cmdline_argv[next])) {
        const char *arg = _cmdline_argv[next];
        _cmdline_argv[next++] = NULL;
        return arg;
    } else
        return "";
}

/* 获取长选项--opt之后的参数
 * 如果opt为0，继续返回之前所设置的opt之后的参数，直到没有参数时返回NULL
 * 如果opt不为0，返回新选项opt的下一个参数，如果没有参数返回空串("")，选项不存在返回NULL
 * 该函数会移除选项和已返回的参数
 */
const char *CmdLine_GetNextArgOfLongOpt(const char *opt)
{
    int i;
    static int next = -1;

    if (opt) {
        next = -1;
        for (i = 0; i < _cmdline_cnt; i++) {
            if (IS_LONG_OPT(_cmdline_argv[i]) && !strcmp(GET_LONG_OPT(_cmdline_argv[i]), opt)) {
                if (next == -1)
                    next = i + 1;
                _cmdline_argv[i] = NULL;
            }
        }
    }

    if (next == -1)
        return NULL;
    if (next < _cmdline_cnt && IS_ARG(_cmdline_argv[next])) {
        const char *arg = _cmdline_argv[next];
        _cmdline_argv[next++] = NULL;
        return arg;
    } else
        return "";
}

/* 返回下一个命令行字符串，不包括argv[0]和已经移除的字符串，到结尾返回空串("") */
const char *CmdLine_GetNextString(void)
{
    static int n;
    while (n < _cmdline_cnt && !_cmdline_argv[n]) 
        n++;
    if (n < _cmdline_cnt)
        return _cmdline_argv[n++];
    else
        return "";
}

/* 返回程序标题 */
const char *CmdLine_GetAppName(void)
{
    return _name;
}

/* 移除-opt选项及所有的参数 */
void CmdLine_RemoveAllShortOptArgs(char opt)
{
    int i, have_opt = 0;

    for (i = 0; i < _cmdline_cnt; i++) {
        if (have_opt)
            if (IS_ARG(_cmdline_argv[i]))
                _cmdline_argv[i] = NULL;
            else
                have_opt = 0;
        if (IS_SHORT_OPT(_cmdline_argv[i]) && GET_SHORT_OPT(_cmdline_argv[i]) == opt) {
            have_opt = 1;
            _cmdline_argv[i] = NULL;
        }
    }
}

/* 移除--opt选项及所有的参数 */
void CmdLine_RemoveAllLongOptArgs(const char *opt)
{
    int i, have_opt = 0;

    for (i = 0; i < _cmdline_cnt; i++) {
        if (have_opt)
            if (IS_ARG(_cmdline_argv[i]))
                _cmdline_argv[i] = NULL;
            else
                have_opt = 0;
            if (IS_LONG_OPT(_cmdline_argv[i]) && !strcmp(GET_LONG_OPT(_cmdline_argv[i]), opt)) {
                have_opt = 1;
                _cmdline_argv[i] = NULL;
            }
        }
}
