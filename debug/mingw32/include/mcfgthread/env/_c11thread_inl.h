// This file is part of MCFCRT.
// See MCFLicense.txt for licensing information.
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_C11THREAD_INL_H_
#define __MCFCRT_ENV_C11THREAD_INL_H_

#ifndef __MCFCRT_C11THREAD_INLINE_OR_EXTERN
#	error Do not use this header directly.
#endif

#include "../ext/assert.h"
#include "../ext/expect.h"

_MCFCRT_EXTERN_C_BEGIN

//-----------------------------------------------------------------------------
// 7.26.2 Initialization functions
//-----------------------------------------------------------------------------
__MCFCRT_C11THREAD_INLINE_OR_EXTERN void __MCFCRT_call_once(once_flag *__once, void (*__func)(void)) _MCFCRT_NOEXCEPT {
	const _MCFCRT_OnceResult __result = _MCFCRT_WaitForOnceFlagForever(&(__once->__once));
	if(_MCFCRT_EXPECT(__result == _MCFCRT_kOnceResultFinished)){
		return;
	}
	_MCFCRT_ASSERT(__result == _MCFCRT_kOnceResultInitial);
	(*__func)();
	_MCFCRT_SignalOnceFlagAsFinished(&(__once->__once));
}

//-----------------------------------------------------------------------------
// 7.26.3 Condition variable functions
//-----------------------------------------------------------------------------
extern _MCFCRT_STD intptr_t __MCFCRT_C11threadUnlockCallback(_MCFCRT_STD intptr_t __context) _MCFCRT_NOEXCEPT;
extern void __MCFCRT_C11threadRelockCallback(_MCFCRT_STD intptr_t __context, _MCFCRT_STD intptr_t __unlocked) _MCFCRT_NOEXCEPT;

__MCFCRT_C11THREAD_INLINE_OR_EXTERN int __MCFCRT_cnd_init(cnd_t *__cond) _MCFCRT_NOEXCEPT {
	_MCFCRT_InitializeConditionVariable(&(__cond->__cond));
	return thrd_success;
}
__MCFCRT_C11THREAD_INLINE_OR_EXTERN void __MCFCRT_cnd_destroy(cnd_t *__cond) _MCFCRT_NOEXCEPT {
	(void)__cond;
}

__MCFCRT_C11THREAD_INLINE_OR_EXTERN _MCFCRT_STD uint64_t __MCFCRT_C11threadTranslateTimeout(const struct timespec *_MCFCRT_RESTRICT __utc_timeout) _MCFCRT_NOEXCEPT {
	const double __utc_timeout_ms = (double)__utc_timeout->tv_sec * 1.0e3 + (double)__utc_timeout->tv_nsec / 1.0e6;
	const double __utc_now_ms = (double)_MCFCRT_GetUtcClock();
	const double __delta_ms = __utc_timeout_ms - __utc_now_ms;
	if(__delta_ms <= 0){
		return 0;
	}
	const _MCFCRT_STD uint64_t __mono_now_ms = _MCFCRT_GetFastMonoClock();
	// XXX: We assume the value returned by _MCFCRT_GetFastMonoClock() is the number of milliseconds since the system starts, which can hardly be greater than 0x1p48.
	//      All current implementations, especially clock_gettime() on Linux, are specified as that. This isn't guaranteed behavior nevertheless.
	const _MCFCRT_STD uint64_t __complement_ms = (1ull << 48) - 1 - __mono_now_ms;
	// Cast the uint64_t to int64_t for performance reasons. The compiler would otherwise produce more code since unsigned types can't be processed by x87 directly.
	if(__delta_ms >= (double)(_MCFCRT_STD int64_t)__complement_ms){
		return (_MCFCRT_STD uint64_t)-1;
	}
	return __mono_now_ms + (_MCFCRT_STD uint64_t)(_MCFCRT_STD int64_t)(__delta_ms + 0.999999);
}

__MCFCRT_C11THREAD_INLINE_OR_EXTERN int __MCFCRT_cnd_timedwait(cnd_t *_MCFCRT_RESTRICT __cond, mtx_t *_MCFCRT_RESTRICT __mutex, const struct timespec *_MCFCRT_RESTRICT __timeout) _MCFCRT_NOEXCEPT {
	const _MCFCRT_STD uint64_t __mono_timeout_ms = __MCFCRT_C11threadTranslateTimeout(__timeout);
	if(!_MCFCRT_WaitForConditionVariable(&(__cond->__cond), &__MCFCRT_C11threadUnlockCallback, &__MCFCRT_C11threadRelockCallback, (_MCFCRT_STD intptr_t)__mutex, _MCFCRT_CONDITION_VARIABLE_SUGGESTED_SPIN_COUNT, __mono_timeout_ms)){
		return thrd_timedout;
	}
	return thrd_success;
}

__MCFCRT_C11THREAD_INLINE_OR_EXTERN int __MCFCRT_cnd_wait(cnd_t *_MCFCRT_RESTRICT __cond, mtx_t *_MCFCRT_RESTRICT __mutex) _MCFCRT_NOEXCEPT {
	_MCFCRT_WaitForConditionVariableForever(&(__cond->__cond), &__MCFCRT_C11threadUnlockCallback, &__MCFCRT_C11threadRelockCallback, (_MCFCRT_STD intptr_t)__mutex, _MCFCRT_CONDITION_VARIABLE_SUGGESTED_SPIN_COUNT);
	return thrd_success;
}
__MCFCRT_C11THREAD_INLINE_OR_EXTERN int __MCFCRT_cnd_signal(cnd_t *__cond) _MCFCRT_NOEXCEPT {
	_MCFCRT_SignalConditionVariable(&(__cond->__cond), 1);
	return thrd_success;
}
__MCFCRT_C11THREAD_INLINE_OR_EXTERN int __MCFCRT_cnd_broadcast(cnd_t *__cond) _MCFCRT_NOEXCEPT {
	_MCFCRT_BroadcastConditionVariable(&(__cond->__cond));
	return thrd_success;
}

//-----------------------------------------------------------------------------
// 7.26.4 Mutex functions
//-----------------------------------------------------------------------------
__MCFCRT_C11THREAD_INLINE_OR_EXTERN int __MCFCRT_mtx_init(mtx_t *__mutex, int __mask) _MCFCRT_NOEXCEPT {
	__mutex->__mask  = __mask;
	__mutex->__owner = 0;
	__mutex->__count = 0;
	_MCFCRT_InitializeMutex(&(__mutex->__mutex));
	return thrd_success;
}
__MCFCRT_C11THREAD_INLINE_OR_EXTERN void __MCFCRT_mtx_destroy(mtx_t *__mutex) _MCFCRT_NOEXCEPT {
	(void)__mutex;
}

__MCFCRT_C11THREAD_INLINE_OR_EXTERN int __MCFCRT_mtx_lock(mtx_t *__mutex) _MCFCRT_NOEXCEPT {
	if(__mutex->__mask & mtx_recursive){
		const _MCFCRT_STD uintptr_t __self = _MCFCRT_GetCurrentThreadId();
		const _MCFCRT_STD uintptr_t __old_owner = __atomic_load_n(&(__mutex->__owner), __ATOMIC_RELAXED);
		if(_MCFCRT_EXPECT_NOT(__old_owner != __self)){
			_MCFCRT_WaitForMutexForever(&(__mutex->__mutex), _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
			__atomic_store_n(&(__mutex->__owner), __self, __ATOMIC_RELAXED);
		}
		const _MCFCRT_STD size_t __new_count = ++__mutex->__count;
		_MCFCRT_ASSERT(__new_count != 0);
	} else {
		_MCFCRT_WaitForMutexForever(&(__mutex->__mutex), _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
	}
	return thrd_success;
}
__MCFCRT_C11THREAD_INLINE_OR_EXTERN int __MCFCRT_mtx_timedlock(mtx_t *_MCFCRT_RESTRICT __mutex, const struct timespec *_MCFCRT_RESTRICT __timeout) _MCFCRT_NOEXCEPT  {
	if(__mutex->__mask & mtx_recursive){
		const _MCFCRT_STD uintptr_t __self = _MCFCRT_GetCurrentThreadId();
		const _MCFCRT_STD uintptr_t __old_owner = __atomic_load_n(&(__mutex->__owner), __ATOMIC_RELAXED);
		if(_MCFCRT_EXPECT_NOT(__old_owner != __self)){
			const _MCFCRT_STD uint64_t __mono_timeout_ms = __MCFCRT_C11threadTranslateTimeout(__timeout);
			if(!_MCFCRT_WaitForMutex(&(__mutex->__mutex), _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT, __mono_timeout_ms)){
				return thrd_timedout;
			}
			__atomic_store_n(&(__mutex->__owner), __self, __ATOMIC_RELAXED);
		}
		const _MCFCRT_STD size_t __new_count = ++__mutex->__count;
		_MCFCRT_ASSERT(__new_count != 0);
	} else {
		const _MCFCRT_STD uint64_t __mono_timeout_ms = __MCFCRT_C11threadTranslateTimeout(__timeout);
		if(!_MCFCRT_WaitForMutex(&(__mutex->__mutex), _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT, __mono_timeout_ms)){
			return thrd_timedout;
		}
	}
	return thrd_success;
}
__MCFCRT_C11THREAD_INLINE_OR_EXTERN int __MCFCRT_mtx_trylock(mtx_t *__mutex) _MCFCRT_NOEXCEPT {
	if(__mutex->__mask & mtx_recursive){
		const _MCFCRT_STD uintptr_t __self = _MCFCRT_GetCurrentThreadId();
		const _MCFCRT_STD uintptr_t __old_owner = __atomic_load_n(&(__mutex->__owner), __ATOMIC_RELAXED);
		if(_MCFCRT_EXPECT_NOT(__old_owner != __self)){
			if(_MCFCRT_EXPECT_NOT(!_MCFCRT_WaitForMutex(&(__mutex->__mutex), _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT, 0))){
				return thrd_busy;
			}
			__atomic_store_n(&(__mutex->__owner), __self, __ATOMIC_RELAXED);
		}
		const _MCFCRT_STD size_t __new_count = ++__mutex->__count;
		_MCFCRT_ASSERT(__new_count != 0);
	} else {
		if(_MCFCRT_EXPECT_NOT(!_MCFCRT_WaitForMutex(&(__mutex->__mutex), _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT, 0))){
			return thrd_busy;
		}
	}
	return thrd_success;
}
__MCFCRT_C11THREAD_INLINE_OR_EXTERN int __MCFCRT_mtx_unlock(mtx_t *__mutex) _MCFCRT_NOEXCEPT {
	if(__mutex->__mask & mtx_recursive){
		_MCFCRT_DEBUG_CHECK(_MCFCRT_GetCurrentThreadId() == __atomic_load_n(&(__mutex->__owner), __ATOMIC_RELAXED));
		const _MCFCRT_STD size_t __new_count = --__mutex->__count;
		if(_MCFCRT_EXPECT_NOT(__new_count == 0)){
			__atomic_store_n(&(__mutex->__owner), 0, __ATOMIC_RELAXED);
			_MCFCRT_SignalMutex(&(__mutex->__mutex));
		}
	} else {
		_MCFCRT_SignalMutex(&(__mutex->__mutex));
	}
	return thrd_success;
}

//-----------------------------------------------------------------------------
// 7.26.5 Thread functions
//-----------------------------------------------------------------------------
typedef struct __MCFCRT_tagC11threadControlBlock {
	int (*__proc)(void *);
	void *__param;
	int __exit_code;
} __MCFCRT_C11threadControlBlock;

extern void __MCFCRT_C11threadMopWrapper(void *__params) _MCFCRT_NOEXCEPT;
extern void __MCFCRT_C11threadMopExitModifier(void *__params, _MCFCRT_STD intptr_t __context) _MCFCRT_NOEXCEPT;

__MCFCRT_C11THREAD_INLINE_OR_EXTERN int __MCFCRT_thrd_create(thrd_t *__tid_ret, thrd_start_t __proc, void *__param) _MCFCRT_NOEXCEPT {
	__MCFCRT_C11threadControlBlock __control = { __proc, __param, (int)0xDEADBEEF };
	const _MCFCRT_STD uintptr_t __tid = __MCFCRT_MopthreadCreate(&__MCFCRT_C11threadMopWrapper, &__control, sizeof(__control));
	if(__tid == 0){
		return thrd_nomem; // XXX: We should have returned `EAGAIN` as what POSIX has specified but there isn't `thrd_again` in ISO C.
	}
	*__tid_ret = __tid;
	return thrd_success;
}
__MCFCRT_C11THREAD_INLINE_OR_EXTERN void __MCFCRT_thrd_exit(int __exit_code) _MCFCRT_NOEXCEPT {
	__MCFCRT_MopthreadExit(&__MCFCRT_C11threadMopExitModifier, __exit_code);
}
__MCFCRT_C11THREAD_INLINE_OR_EXTERN int __MCFCRT_thrd_join(thrd_t __tid, int *__exit_code_ret) _MCFCRT_NOEXCEPT {
	if(__tid == _MCFCRT_GetCurrentThreadId()){
		return thrd_error; // XXX: EDEADLK
	}
	if(__exit_code_ret){
		__MCFCRT_C11threadControlBlock __control;
		if(!__MCFCRT_MopthreadJoin(__tid, &__control)){
			return thrd_error; // XXX: ESRCH
		}
		*__exit_code_ret = __control.__exit_code;
	} else {
		if(!__MCFCRT_MopthreadJoin(__tid, _MCFCRT_NULLPTR)){
			return thrd_error; // XXX: ESRCH
		}
	}
	return thrd_success;
}
__MCFCRT_C11THREAD_INLINE_OR_EXTERN int __MCFCRT_thrd_detach(thrd_t __tid) _MCFCRT_NOEXCEPT {
	if(__tid == _MCFCRT_GetCurrentThreadId()){
		return thrd_error; // XXX: EDEADLK
	}
	if(!__MCFCRT_MopthreadDetach(__tid)){
		return thrd_error; // XXX: ESRCH
	}
	return thrd_success;
}

__MCFCRT_C11THREAD_INLINE_OR_EXTERN thrd_t __MCFCRT_thrd_current(void) _MCFCRT_NOEXCEPT {
	return _MCFCRT_GetCurrentThreadId();
}
__MCFCRT_C11THREAD_INLINE_OR_EXTERN int __MCFCRT_thrd_equal(thrd_t __tid1, thrd_t __tid2) _MCFCRT_NOEXCEPT {
	return __tid1 == __tid2;
}

__MCFCRT_C11THREAD_INLINE_OR_EXTERN int __MCFCRT_thrd_sleep(const struct timespec *__duration, struct timespec *__remaining) _MCFCRT_NOEXCEPT {
	const double __delta_ms = (double)__duration->tv_sec * 1.0e3 + (double)__duration->tv_nsec / 1.0e6;
	const _MCFCRT_STD uint64_t __mono_begin_ms = _MCFCRT_GetFastMonoClock();
	const _MCFCRT_STD uint64_t __complement_ms = (1ull << 48) - 1 - __mono_begin_ms;
	_MCFCRT_STD uint64_t __mono_timeout_ms;
	if(__delta_ms >= (double)__complement_ms){
		__mono_timeout_ms = (_MCFCRT_STD uint64_t)-1;
	} else {
		__mono_timeout_ms = __mono_begin_ms + (_MCFCRT_STD uint64_t)__delta_ms;
	}
	_MCFCRT_Sleep(__mono_timeout_ms);

	if(__remaining){
		const _MCFCRT_STD uint64_t __mono_end_ms = _MCFCRT_GetFastMonoClock();
		const double __slept_ms = (double)__mono_end_ms - (double)__mono_begin_ms;
		const double __remaining_s = (__delta_ms - __slept_ms) / 1.0e3;

		long long __sec = 0;
		long __nsec = 0;
		if(__remaining_s > 0){
			__sec = (long long)__remaining_s;
			__nsec = (long)((__remaining_s - (double)__sec) * 1.0e9);
		}
		__remaining->tv_sec  = (time_t)__sec;
		__remaining->tv_nsec = __nsec;
	}
	return 0; // On Windows we don't support interruptable sleep.
}
__MCFCRT_C11THREAD_INLINE_OR_EXTERN void __MCFCRT_thrd_yield(void) _MCFCRT_NOEXCEPT {
	_MCFCRT_YieldThread();
}

//-----------------------------------------------------------------------------
// 7.26.6 Thread-specific storage functions
//-----------------------------------------------------------------------------
extern void __MCFCRT_C11threadTlsDestructor(_MCFCRT_STD intptr_t __context, void *__storage) _MCFCRT_NOEXCEPT;

__MCFCRT_C11THREAD_INLINE_OR_EXTERN int __MCFCRT_tss_create(tss_t *__key_ret, tss_dtor_t __destructor) _MCFCRT_NOEXCEPT {
	const tss_t __key = _MCFCRT_TlsAllocKey(sizeof(void *), _MCFCRT_NULLPTR, __destructor ? &__MCFCRT_C11threadTlsDestructor : _MCFCRT_NULLPTR, (_MCFCRT_STD intptr_t)__destructor);
	if(!__key){
		return thrd_nomem;
	}
	*__key_ret = __key;
	return thrd_success;
}
__MCFCRT_C11THREAD_INLINE_OR_EXTERN void __MCFCRT_tss_delete(tss_t __key) _MCFCRT_NOEXCEPT {
	_MCFCRT_TlsFreeKey(__key);
}

__MCFCRT_C11THREAD_INLINE_OR_EXTERN void *__MCFCRT_tss_get(tss_t __key) _MCFCRT_NOEXCEPT {
	void *__storage;
	const bool __success = _MCFCRT_TlsGet(__key, &__storage);
	if(!__success){
		return _MCFCRT_NULLPTR;
	}
	if(!__storage){
		return _MCFCRT_NULLPTR;
	}
	return *(void **)__storage;
}
__MCFCRT_C11THREAD_INLINE_OR_EXTERN int __MCFCRT_tss_set(tss_t __key, void *__value) _MCFCRT_NOEXCEPT {
	void *__storage;
	const bool __success = _MCFCRT_TlsRequire(__key, &__storage);
	if(!__success){
		return thrd_error; // XXX: ISO C does not seem to allow `thrd_nomem` here.
	}
	_MCFCRT_ASSERT(__storage);
	*(void **)__storage = (void *)__value;
	return thrd_success;
}

_MCFCRT_EXTERN_C_END

#endif
