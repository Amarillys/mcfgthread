// This file is part of MCFCRT.
// See MCFLicense.txt for licensing information.
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#define __MCFCRT_GTHREAD_INLINE_OR_EXTERN     extern inline
#include "gthread.h"
#include "_seh_top.h"

void __MCFCRT_GthreadTlsDestructor(intptr_t context, void *storage){
	void (*const destructor)(void *) = (void (*)(void *))context;

	void *const value = *(void **)storage;
	if(!value){
		return;
	}
	*(void **)storage = _MCFCRT_NULLPTR;

	(*destructor)(value);
}

intptr_t __MCFCRT_GthreadUnlockCallbackMutex(intptr_t context){
	__gthread_mutex_t *const mutex = (__gthread_mutex_t *)context;

	__gthread_mutex_unlock(mutex);
	return 1;
}
void __MCFCRT_GthreadRelockCallbackMutex(intptr_t context, intptr_t unlocked){
	__gthread_mutex_t *const mutex = (__gthread_mutex_t *)context;

	_MCFCRT_ASSERT((size_t)unlocked == 1);
	__gthread_mutex_lock(mutex);
}

intptr_t __MCFCRT_GthreadUnlockCallbackRecursiveMutex(intptr_t context){
	__gthread_recursive_mutex_t *const recur_mutex = (__gthread_recursive_mutex_t *)context;
	_MCFCRT_ASSERT(_MCFCRT_GetCurrentThreadId() == __atomic_load_n(&(recur_mutex->__owner), __ATOMIC_RELAXED));

	const size_t old_count = recur_mutex->__count;
	recur_mutex->__count = 0;
	__atomic_store_n(&(recur_mutex->__owner), 0, __ATOMIC_RELAXED);

	__gthread_mutex_unlock(&(recur_mutex->__mutex));
	return (intptr_t)old_count;
}
void __MCFCRT_GthreadRelockCallbackRecursiveMutex(intptr_t context, intptr_t unlocked){
	__gthread_recursive_mutex_t *const recur_mutex = (__gthread_recursive_mutex_t *)context;

	_MCFCRT_ASSERT((size_t)unlocked >= 1);
	__gthread_mutex_lock(&(recur_mutex->__mutex));

	const uintptr_t self = _MCFCRT_GetCurrentThreadId();
	__atomic_store_n(&(recur_mutex->__owner), self, __ATOMIC_RELAXED);
	recur_mutex->__count = (size_t)unlocked;
}

void __MCFCRT_GthreadMopWrapper(void *params){
	__MCFCRT_GthreadControlBlock *const control = params;

	void *exit_code;

	__MCFCRT_SEH_TOP_BEGIN
	{
		exit_code = (*(control->__proc))(control->__param);
	}
	__MCFCRT_SEH_TOP_END

	control->__exit_code = exit_code;
}
