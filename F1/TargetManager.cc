#include "stdafx.hh"

#include "CHack.hh"
#include "TargetManager.hh"

#include <tier0/memdbgon.h>

TargetManagerBase *TargetManagerBase::head = nullptr;

ConVar f1_dumpalltargetmanagerexceptions("f1_dumpalltargetmanagerexceptions", "0", FCVAR_NONE, "create a dump from all target manager exceptions");

TargetManagerBase::TargetManagerBase()
{
	// linked list
	if (head == nullptr) {
		head = this;
	} else {
		this->next = head;
		head       = this;
	}
}

TargetManagerBase::~TargetManagerBase()
{
	if (head == this) {
		head = this->next;
	} else {
		for (auto *manager = head; manager != nullptr; manager = manager->next) {
			if (manager->next == this) {
				manager->next = this->next;
			}
		}
	}
}
