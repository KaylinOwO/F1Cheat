#pragma once

#include <string>

class CKnifeState
{

    friend class CKnifeProvider;

    std::wstring knifeDir;

    class ICLRMetaHost *metaHost;
    class ICLRRuntimeInfo *runtimeInfo;
    class ICLRRuntimeHost *runtimeHost;

    bool inited = false;

public:

	bool Inited()
	{
		return inited;
	}

    int SetupKnife();

    int CLRSetup();

    int CLRShutdown();

    int KnifeMain();

    // calls out to a function in the loader dll in the default namespace
    int CallLoaderFunction( std::wstring &classname, std::wstring &funcname, std::wstring &args );
};

extern CKnifeState ck;

int SetupKnife();

int ShutdownKnife();