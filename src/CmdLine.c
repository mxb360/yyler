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

/* �����г�ʼ�� */
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

/* ���������Ƿ��ж�ѡ�� -opt
 * ����У�����1�����򷵻�0
 * ���removΪ1���Ὣ��ѡ���Ƴ�
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

/* ���������Ƿ��г�ѡ�� --opt
 * ����У�����1�����򷵻�0
 * ���removΪ1���Ὣ��ѡ���Ƴ�
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

/* �ͷ�cmdline_init����Ŀռ䣬�Ƴ����е�������ѡ��Ͳ���
 */
void CmdLine_Free(void)
{
    free(_cmdline_argv);
    _cmdline_cnt = 0;
    _cmdline_argv = NULL;
    this_cnt = 0;
}

/* ��ȡ��ѡ��-opt֮��Ĳ���
 * ���optΪ0����������֮ǰ�����õ�opt֮��Ĳ�����ֱ��û�в���ʱ���ؿմ�("")
 * ���opt��Ϊ0��������ѡ��opt����һ�����������û�в������ؿմ�("")��ѡ����ڷ���NULL
 * �ú������Ƴ�ѡ����ѷ��صĲ���
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

/* ��ȡ��ѡ��--opt֮��Ĳ���
 * ���optΪ0����������֮ǰ�����õ�opt֮��Ĳ�����ֱ��û�в���ʱ����NULL
 * ���opt��Ϊ0��������ѡ��opt����һ�����������û�в������ؿմ�("")��ѡ����ڷ���NULL
 * �ú������Ƴ�ѡ����ѷ��صĲ���
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

/* ������һ���������ַ�����������argv[0]���Ѿ��Ƴ����ַ���������β���ؿմ�("") */
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

/* ���س������ */
const char *CmdLine_GetAppName(void)
{
    return _name;
}

/* �Ƴ�-optѡ����еĲ��� */
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

/* �Ƴ�--optѡ����еĲ��� */
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
