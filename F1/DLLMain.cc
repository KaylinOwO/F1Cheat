#include "stdafx.hh"

#include "../SDK/unlinkpeb.hh"
#include "CHack.hh"
#include "Panels.hh"

#include <tier0/memdbgon.h>

// maybe move these out of here and into their respective files
CHack gHack;

// TODO: make member of CHack
DWORD WINAPI Unhook(LPVOID lpArguments)
{
	while (!GetAsyncKeyState(VK_F11))
		Sleep(200);

	// release hooks
	//gHookManager.~CHookManager();

	mypanel->Destroy();

	// restore the wndproc
	SetWindowLongPtr(gInts->thisWindow, GWLP_WNDPROC, (LONG_PTR)gInts->oldWindowProc);

	FreeLibraryAndExitThread((HMODULE)lpArguments, 0);
}

BOOL APIENTRY DllMain(HMODULE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	// If you manually map, make sure you setup this function properly.
	if (dwReason == DLL_PROCESS_ATTACH) {

		Log::Init(hInstance);

#ifdef _DEBUG
		_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
#endif
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)&CHack::Init, hInstance, 0, 0);
	}
	return true;
}
