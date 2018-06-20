#pragma once

// Helper macros for vprof

// TODO: move into SDK?

#define __CHEAT_PREFIX "F1"

#define F1_VPROF(name) VPROF_BUDGET (name, __CHEAT_PREFIX)

#define F1_VPROF_FUNCTION() F1_VPROF (__CHEAT_PREFIX "::" __FUNCTION__)
