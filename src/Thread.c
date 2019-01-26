#include "Thread.h"

#include <stdlib.h>
#include <Windows.h>

struct _Thread {
    int id;
    HANDLE handle;
};

/* ����һ���̶߳���
 * func:  �̺߳���
 * args:  �����Ĳ���
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

/* ��ʼ����һ���߳�
 */
int Thread_Start(Thread thread)
{
    if (!thread)
        return -1;
    return ResumeThread(thread->handle);
}

/* ��ȡһ���̵߳�id��
 */
int Thread_GetId(Thread thread)
{
    if (thread)
        return thread->id;
    return -1;
}

/* �ȴ��߳�thread��������뾭��ms����󷵻�
 * ���msΪ0��ֱ��thread�����ŷ���
 */
int Thread_WaiteEnd(Thread thread, unsigned int ms)
{
    if (!thread)
        return -1;
    if (ms == 0)
        ms = INFINITE;
    return WaitForSingleObject(thread->handle, ms);
}

/* �ж��߳��Ƿ������У�������-1
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

/* ��ȡ��ǰ�̵߳�id��*/
int Thread_GetCurrentId(void)
{
    return GetCurrentThreadId();
}

/* ��ʱ��ǰ�߳�ms���� */
void Thread_Sleep(int ms)
{
    Sleep(ms);
}
