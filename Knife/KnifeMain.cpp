//#define NO_KNIFE

#include "KnifeMain.h"
#include "../SDK/WindowsProxy.h"

#ifndef NO_KNIFE
#include <Metahost.h>
#endif

#pragma comment(lib, "mscoree.lib")

#include <codecvt>
#include <locale>
#include <string>

CKnifeState ck;

int CKnifeState::SetupKnife ()
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

	const char *homeDir = getenv ("USERPROFILE");

	knifeDir = converter.from_bytes (std::string (homeDir) + "\\F12017\\Knife\\");

	// ensure that this directory exists
	CreateDirectoryW (knifeDir.c_str (), NULL);

	return 0;
}

int SetupKnife ()
{
	int ret = ck.SetupKnife ();

	if (ret == -1)
		return ret;

	// call clr setup
	ret = ck.CLRSetup ();

	if (ret == -1) {
		ck.CLRShutdown ();
		return -1;
	}

	// call the main function
	ret = ck.KnifeMain ();

	return ret;
}

int CKnifeState::CLRSetup ()
{
#ifndef NO_KNIFE
	HRESULT hr;

	if (metaHost != nullptr) {
		// already setup
		return 0;
	}

	hr = CLRCreateInstance (CLSID_CLRMetaHost, IID_PPV_ARGS (&metaHost));

	if (FAILED (hr)) {
		return -1;
	}

	// Get the ICLRRuntimeInfo corresponding to a particular CLR version. It
	// supersedes CorBindToRuntimeEx with STARTUP_LOADER_SAFEMODE.
	hr = metaHost->GetRuntime (L"v4.0.30319", IID_PPV_ARGS (&runtimeInfo));

	if (FAILED (hr)) {
		return -1;
	}

	// Check if the specified runtime can be loaded into the process. This
	// method will take into account other runtimes that may already be
	// loaded into the process and set pbLoadable to TRUE if this runtime can
	// be loaded in an in-process side-by-side fashion.
	BOOL fLoadable;
	hr = runtimeInfo->IsLoadable (&fLoadable);

	if (FAILED (hr)) {
		return -1;
	}

	if (!fLoadable) {
		return -1;
	}

	runtimeHost;

	// Load the CLR into the current process and return a runtime interface
	// pointer. ICorRuntimeHost and ICLRRuntimeHost are the two CLR hosting
	// interfaces supported by CLR 4.0. Here we demo the ICLRRuntimeHost
	// interface that was provided in .NET v2.0 to support CLR 2.0 new
	// features. ICLRRuntimeHost does not support loading the .NET v1.x
	// runtimes.
	hr = runtimeInfo->GetInterface (CLSID_CLRRuntimeHost, IID_PPV_ARGS (&runtimeHost));

	if (FAILED (hr)) {
		return -1;
	}

	// Start the CLR.
	hr = runtimeHost->Start ();
	if (FAILED (hr)) {
		return -1;
	}

	inited = true;
#endif

	return 0;
}

int CKnifeState::CLRShutdown ()
{
#ifndef NO_KNIFE
	HRESULT hr;

	DWORD dwReturn;

	if (inited) {
		inited = false;

		// if this fails then there is nothing we can do !
		hr = runtimeHost->ExecuteInDefaultAppDomain ((knifeDir + L"Loader.dll").c_str (), L"KnifeLoader.AppLoader", L"Unload", knifeDir.c_str (), &dwReturn);
	}

	if (metaHost) {
		metaHost->Release ();
		metaHost = NULL;
	}
	if (runtimeInfo) {
		runtimeInfo->Release ();
		runtimeInfo = NULL;
	}
	if (runtimeHost) {
		// Please note that after a call to Stop, the CLR cannot be
		// reinitialized into the same process. This step is usually not
		// necessary. You can leave the .NET runtime loaded in your process.
		// wprintf(L"Stop the .NET runtime\n");
		// runtimeHost->Stop();

		runtimeHost->Release ();
		runtimeHost = NULL;
	}
#endif
	return 0;
}

int ShutdownKnife ()
{
	return ck.CLRShutdown ();
}

int CKnifeState::KnifeMain ()
{
	DWORD dwReturn = 0;
#ifndef NO_KNIFE
	HRESULT hr;

	// The invoked method of ExecuteInDefaultAppDomain must have the
	// following signature: static int pwzMethodName (String pwzArgument)
	// where pwzMethodName represents the name of the invoked method, and
	// pwzArgument represents the string value passed as a parameter to that
	// method. If the HRESULT return value of ExecuteInDefaultAppDomain is
	// set to S_OK, pReturnValue is set to the integer value returned by the
	// invoked method. Otherwise, pReturnValue is not set.
	hr = runtimeHost->ExecuteInDefaultAppDomain ((knifeDir + L"Loader.dll").c_str (), L"KnifeLoader.AppLoader", L"Load", knifeDir.c_str (), &dwReturn);

	if (FAILED (hr)) {
		return -1;
	} else
#endif
	{
		return dwReturn;
	}
}

int CKnifeState::CallLoaderFunction (std::wstring &classname, std::wstring &funcname, std::wstring &args)
{
#ifndef NO_KNIFE
	HRESULT hr;

	DWORD dwReturn = 0;

	// The invoked method of ExecuteInDefaultAppDomain must have the
	// following signature: static int pwzMethodName (String pwzArgument)
	// where pwzMethodName represents the name of the invoked method, and
	// pwzArgument represents the string value passed as a parameter to that
	// method. If the HRESULT return value of ExecuteInDefaultAppDomain is
	// set to S_OK, pReturnValue is set to the integer value returned by the
	// invoked method. Otherwise, pReturnValue is not set.
	hr = runtimeHost->ExecuteInDefaultAppDomain ((knifeDir + L"Loader.dll").c_str (), classname.c_str (), funcname.c_str (), knifeDir.c_str (), &dwReturn);

	if (FAILED (hr)) {
		return -1;
	} else {
		return dwReturn;
	}
#endif
}