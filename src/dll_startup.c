// This file is part of MCFCRT.
// See MCFLicense.txt for licensing information.
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "env/mcfwin.h"
#include "env/thread_env.h"

static bool g_bInitialized = false;

__MCFCRT_C_STDCALL
BOOL DllMain(HINSTANCE hDll, DWORD dwReason, LPVOID pReserved){
	(void)hDll;
	(void)pReserved;

	bool bRet = true;

	switch(dwReason){
	case DLL_PROCESS_ATTACH:
		if(g_bInitialized){
			break;
		}
		bRet = __MCFCRT_ThreadEnvInit();
		if(!bRet){
			break;
		}
		g_bInitialized = true;
		break;

	case DLL_THREAD_ATTACH:
		break;

	case DLL_THREAD_DETACH:
		__MCFCRT_TlsCleanup();
		break;

	case DLL_PROCESS_DETACH:
		if(!g_bInitialized){
			break;
		}
		g_bInitialized = false;
		__MCFCRT_ThreadEnvUninit();
		break;
	}

	return bRet;
}
