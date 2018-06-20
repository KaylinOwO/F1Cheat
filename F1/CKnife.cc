#include "stdafx.hh"

#include "CKnife.hh"

#include "CHack.hh"

#include <tier0/memdbgon.h>

CKnifeInterface gKnifeInterface;

enum class EventCallbackType
{
	init,
	paint,
	beforePred,
	afterPred,
	keyevent,
	entity,
};

extern "C" EXPOSE void Knife_SubscribeToEvent(EventCallbackType t, void *callback)
{
	auto FindFirstNull = [](void **array, int max) {
		for (int i = 0; i < max; i++)
			if (array[i] == nullptr)
				return i;
		return 0;
	};
	switch (t) {
	case EventCallbackType::init: {
		int i                         = FindFirstNull(gKnifeInterface.initFnPtrs, 255);
		gKnifeInterface.initFnPtrs[i] = callback;
		break;
	}
	case EventCallbackType::paint: {
		int i                          = FindFirstNull(gKnifeInterface.paintFnPtrs, 255);
		gKnifeInterface.paintFnPtrs[i] = callback;
		break;
	}
	case EventCallbackType::beforePred: {
		int i                                             = FindFirstNull(gKnifeInterface.processCommandBeforePredFnPtrs, 255);
		gKnifeInterface.processCommandBeforePredFnPtrs[i] = callback;
		break;
	}
	case EventCallbackType::afterPred: {
		int i                                   = FindFirstNull(gKnifeInterface.processCommandFnPtrs, 255);
		gKnifeInterface.processCommandFnPtrs[i] = callback;
		break;
	}
	case EventCallbackType::keyevent: {
		int i                             = FindFirstNull(gKnifeInterface.keyEventFnPtrs, 255);
		gKnifeInterface.keyEventFnPtrs[i] = callback;
		break;
	}
	case EventCallbackType::entity: {
		int i                                  = FindFirstNull(gKnifeInterface.processEntityFnPtrs, 255);
		gKnifeInterface.processEntityFnPtrs[i] = callback;
		break;
	}
	default:
		break;
	}
}

extern "C" EXPOSE CInterfaces *Knife_GetInterfaces()
{
	return gInts;
}
