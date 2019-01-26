#ifndef __THREAD_H__
#define __THREAD_H__

#ifdef __cplusplus
extern "C" {
#endif 

typedef struct _Thread *Thread;
typedef int(*ThreadFunc)(void *args);

Thread Thread_New(ThreadFunc func, void *args);
int    Thread_Start(Thread thread);
int    Thread_GetId(Thread thread);
int    Thread_WaiteEnd(Thread thread, unsigned int ms);
int    Thread_IsActive(Thread thread);
int    Thread_GetCurrentId(void);
void   Thread_Sleep(int ms);

#ifdef __cplusplus
}
#endif 

#endif
