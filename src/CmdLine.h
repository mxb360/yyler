/* CmdLine: �����н�������
 *
 * ʹ�÷�ʽ��
 *   1. ��CmdLine_HaveShortOpt(opt, 1)�ж��Ƿ����޲�����ѡ��-opt
 *      �����Լ����߼�����������û��Ƴ���ѡ�
 *   2. ��CmdLine_HaveLongOpt(opt, 1)�ж��Ƿ����޲�����ѡ��--opt
 *      �����Լ����߼�����������û��Ƴ���ѡ�
 *   3. ��CmdLine_GetNextArgOfShortOpt(opt)��ȡ��ѡ��-opt�����в���
 *      �����Լ����߼�����������û��Ƴ���ѡ������еĲ�����
 *   4. ��CmdLine_GetNextArgOfLongOpt(opt)��ȡ��ѡ��--opt�����в���
 *      �����Լ����߼�����������û��Ƴ���ѡ������еĲ�����
 *   5. ��CmdLine_GetNextString()��ȡʣ�µ������ַ���������IS_SHORT_OPT, 
 *      IS_LONG_OPT��IS_NOT_OPT�ж��Ƕ����Ĳ��������ǷǷ���ѡ��
 *
 *  V1.0
 *  By Ma Xiaobo
 *  2018-12
 */

#ifndef __CMDLINE_H__
#define __CMDLINE_H__

#define IS_SHORT_OPT(str)  ((str) && (str)[0] == '-' && (str)[1] && !(str)[2])
#define GET_SHORT_OPT(str) ((str)[1])
#define IS_LONG_OPT(str)   ((str) && (str)[0] == '-' && (str)[1] == '-' && (str)[2])
#define GET_LONG_OPT(str)  ((str) + 2)
#define IS_ARG(str)    ((str) && !IS_SHORT_OPT(str) && !IS_LONG_OPT(str))

int CmdLine_Init(int argc, char *argv[]);
int CmdLine_HaveShortOpt(char opt, int remove);
int CmdLine_HaveLongOpt(const char *opt, int remove);
void CmdLine_Free(void);
const char *CmdLine_GetNextArgOfShortOpt(char opt);
const char *CmdLine_GetNextArgOfLongOpt(const char *opt);
const char *CmdLine_GetNextString(void);
const char *CmdLine_GetAppName(void);
void CmdLine_RemoveAllShortOptArgs(char opt);
void CmdLine_RemoveAllLongOptArgs(const char *opt);

#endif  /* !__CMDLINE_H__ */
