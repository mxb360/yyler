/* CmdLine: 命令行解析工具
 *
 * 使用方式：
 *   1. 用CmdLine_HaveShortOpt(opt, 1)判断是否有无参数短选项-opt
 *      按照自己的逻辑处理，这个调用会移除该选项。
 *   2. 用CmdLine_HaveLongOpt(opt, 1)判断是否有无参数长选项--opt
 *      按照自己的逻辑处理，这个调用会移除该选项。
 *   3. 用CmdLine_GetNextArgOfShortOpt(opt)获取短选项-opt的所有参数
 *      按照自己的逻辑处理，这个调用会移除该选项和所有的参数。
 *   4. 用CmdLine_GetNextArgOfLongOpt(opt)获取长选项--opt的所有参数
 *      按照自己的逻辑处理，这个调用会移除该选项和所有的参数。
 *   5. 用CmdLine_GetNextString()获取剩下的所有字符串，并用IS_SHORT_OPT, 
 *      IS_LONG_OPT或IS_NOT_OPT判断是独立的参数或者是非法的选项
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
