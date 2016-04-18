// This file is part of MCFCRT.
// See MCFLicense.txt for licensing information.
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_ONCE_FLAG_H_
#define __MCFCRT_ENV_ONCE_FLAG_H_

#include "_crtdef.h"

_MCFCRT_EXTERN_C_BEGIN

// In the case of static initialization, please initialize it with { 0 }.
typedef struct __MCFCRT_tagOnceFlag {
	_MCFCRT_STD uintptr_t __u;
} volatile _MCFCRT_OnceFlag;

typedef enum __MCFCRT_tagOnceResult {
	_MCFCRT_kOnceResultTimedOut = 1,
	_MCFCRT_kOnceResultInitial  = 2,
	_MCFCRT_kOnceResultFinished = 3,
} _MCFCRT_OnceResult;

static inline void _MCFCRT_InitializeOnceFlag(_MCFCRT_OnceFlag *__pOnceFlag) _MCFCRT_NOEXCEPT {
	__atomic_store_n(&(__pOnceFlag->__u), 0, __ATOMIC_RELEASE);
}

extern _MCFCRT_OnceResult _MCFCRT_WaitForOnceFlag(_MCFCRT_OnceFlag *__pOnceFlag, _MCFCRT_STD uint64_t __u64UntilFastMonoClock) _MCFCRT_NOEXCEPT;
extern _MCFCRT_OnceResult _MCFCRT_WaitForOnceFlagForever(_MCFCRT_OnceFlag *__pOnceFlag) _MCFCRT_NOEXCEPT;
extern void _MCFCRT_SignalOnceFlagAsFinished(_MCFCRT_OnceFlag *__pOnceFlag) _MCFCRT_NOEXCEPT;
extern void _MCFCRT_SignalOnceFlagAsAborted(_MCFCRT_OnceFlag *__pOnceFlag) _MCFCRT_NOEXCEPT;

_MCFCRT_EXTERN_C_END

#endif