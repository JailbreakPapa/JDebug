#pragma once

// Deactivate Doxygen document generation for the following block.
/// \cond

#include <pthread.h>
#include <semaphore.h>

using wdThreadHandle = pthread_t;
using wdThreadID = pthread_t;
using wdMutexHandle = pthread_mutex_t;
using wdOSThreadEntryPoint = void* (*)(void* pThreadParameter);

struct wdSemaphoreHandle
{
  sem_t* m_pNamedOrUnnamed = nullptr;
  sem_t* m_pNamed = nullptr;
  sem_t m_Unnamed;
};

#define WD_THREAD_CLASS_ENTRY_POINT void* wdThreadClassEntryPoint(void* pThreadParameter);

struct wdConditionVariableData
{
  pthread_cond_t m_ConditionVariable;
};


/// \endcond
