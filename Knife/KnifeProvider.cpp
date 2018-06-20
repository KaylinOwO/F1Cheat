#include "KnifeProvider.h"
#include "KnifeMain.h"

template <typename f>
void KnifeFunction<f>::Compile ()
{
	if (ck.Inited ()) {
		// call out to the loader to compile this code
		int ret = ck.CallLoaderFunction (L"KnifeLoader.KnifeProvider", L"Compile", stringdata);

		// if ret is -1 then we failed becuase knife hasnt been inited or the code is bad
		if (ret == -1) {
			// try to get the failure reason
			ret = ck.CallLoaderFunction (L"KnifeLoader.KnifeProvider", L"FailureReason", L"");

			if (ret == -1) {
				// uh oh problem
				compileinfo = L"Error getting compile info...";
			} else {
				compileinfo = std::wstring ((wchar_t *)ret);
			}
		} else {
			// code compiled successfully - the return value is a pointer to the delegate
			assign ((f)ret);
		}
	}
}

template <typename __FunctionType>
std::wstring &KnifeFunction<__FunctionType>::GetCompileInfo ()
{
	return compileinfo;
}
