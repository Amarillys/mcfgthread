// This file is part of MCFCRT.
// See MCFLicense.txt for licensing information.
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_PRE_MODULE_H_
#define __MCFCRT_PRE_MODULE_H_

#include "../env/_crtdef.h"

#ifndef __MCFCRT_MODULE_INLINE_OR_EXTERN
#	define __MCFCRT_MODULE_INLINE_OR_EXTERN     __attribute__((__gnu_inline__)) extern inline
#endif

_MCFCRT_EXTERN_C_BEGIN

extern bool __MCFCRT_ModuleInit(void) _MCFCRT_NOEXCEPT;
extern void __MCFCRT_ModuleUninit(void) _MCFCRT_NOEXCEPT;

typedef void (*_MCFCRT_AtModuleExitCallback)(_MCFCRT_STD intptr_t __nContext);

extern bool _MCFCRT_AtModuleExit(_MCFCRT_AtModuleExitCallback __pfnProc, _MCFCRT_STD intptr_t __nContext) _MCFCRT_NOEXCEPT;

// This symbol should be added implicitly by GNU LD.
extern const char __MCFCRT_ImageBase[]
	__asm__("__image_base__");

__MCFCRT_MODULE_INLINE_OR_EXTERN void *_MCFCRT_GetModuleBase(void) _MCFCRT_NOEXCEPT {
	return (void *)__MCFCRT_ImageBase;
}

_MCFCRT_EXTERN_C_END

#endif
