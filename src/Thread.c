#include "Thread.h"

#include <stdlib.h>
#include <Windows.h>

struct _Thread {
    int id;
    HANDLE handle;
};

/* 创建一个线程对象
 * func:  线程函数
 * args:  函数的参数
 */
Thread Thread_New(ThreadFunc func, void *args)
{
    Thread thread = (Thread)malloc(sizeof(struct _Thread));
    if (thread) 
        thread->handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)func, (LPVOID)args, CREATE_SUSPENDED, (LPDWORD)&thread->id);
    if (!thread) {
        free(thread);
        thread = NULL;
    }
    return thread;
}

/* 开始运行一个线程
 */
int Thread_Start(Thread thread)
{
    if (!thread)
        return -1;
    return ResumeThread(thread->handle);
}

/* 获取一个线程的id号
 */
int Thread_GetId(Thread thread)
{
    if (thread)
        return thread->id;
    return -1;
}

/* 等待线程thread结束后或秒经过ms毫秒后返回
 * 如果ms为0，直到thread结束才返回
 */
int Thread_WaiteEnd(Thread thread, unsigned int ms)
{
    if (!thread)
        return -1;
    if (ms == 0)
        ms = INFINITE;
    return WaitForSingleObject(thread->handle, ms);
}

/* 判断线程是否还在运行，出错返回-1
 */
int Thread_IsActive(Thread thread)
{
    DWORD code;
    if (!thread)
        return -1;
    if (!GetExitCodeThread(thread->handle, &code))
        return -1;
    return code == STILL_ACTIVE;
}

/* 获取当前线程的id号*/
int Thread_GetCurrentId(void)
{
    return GetCurrentThreadId();
}

/* 延时当前线程ms毫秒 */
void Thread_Sleep(int ms)
{
    Sleep(ms);
}
