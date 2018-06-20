#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "../SDK/baseHeaders.hh"

#include <inputsystem/ButtonCode.h>

#include <tier1/convar.h>

class F1_IConVar;

class F1_ConVarAccessor
{

	static bool isFirst;

public:
	F1_ConVarAccessor();

	virtual ~F1_ConVarAccessor();

public:
	F1_IConVar *next = nullptr;

	static F1_IConVar *staticConVarTail;
	static F1_IConVar *staticConVarHead;
};

class F1_IConVar : public F1_ConVarAccessor
{
public:
	F1_IConVar()
	    : F1_ConVarAccessor()
	{
	}

	virtual ~F1_IConVar()
	{
	}

	virtual void increment() = 0;
	virtual void decrement() = 0;

	virtual void        SetFromString(const char *s) = 0;
	virtual const char *print()                      = 0;

	virtual const char *name()         = 0;
	virtual const char *InternalName() = 0;

	virtual F1_IConVar *parent() = 0;

	virtual vgui::Panel *&getPanel() = 0;
};

// extern F1_IConVar **gConvarArray;
// extern const int gConvarArraySize;

// base convar impl's some basic functionality
class F1_BaseConVar : public F1_IConVar
{
	char displayName[64];
	char internalName[64];

	F1_IConVar *par;

	vgui::Panel *panel;

	void DeletePanel()
	{
		Log::Console("Deleting panel for %s", name());
		__try {
			panel->SetParent((vgui::Panel *)NULL);
			delete panel;
			panel = nullptr;
		} __except (EXCEPTION_EXECUTE_HANDLER) {
			Log::Console("Error removing panel %s", internalName);
			panel = nullptr;
		}
	}

public:
	F1_BaseConVar(const char *name, const char *internalName, F1_IConVar *par);
	~F1_BaseConVar()
	{
		if (this->panel != nullptr)
		// cleanup vgui panels
		{
			DeletePanel();
		}
	}

	virtual const char *name() override
	{
		return displayName;
	}

	virtual const char *InternalName() override
	{
		return internalName;
	}

	F1_IConVar *parent() override
	{
		return par;
	}
	vgui::Panel *&getPanel() override
	{
		return panel;
	}
};

// impl of IConVar for template type T
// add specialisations as needed
template <typename T>
class F1_ConVar : public F1_BaseConVar
{
	T value, min, max, inc;

	bool hasMinMax;

public:
	// default initialiser
	F1_ConVar(const char *n, const char *internalName, T val, T min, T max, T inc, F1_IConVar *p)
	    : F1_BaseConVar(n, internalName, p), value(val), min(min), max(max), inc(inc)
	{
		if (min == max)
			hasMinMax = false;
		// clamp
		if (val > max)
			val = max;
		if (val < min)
			val = min;
	};

	void increment() override
	{
		T newVal = value + inc;
		if (hasMinMax && newVal <= max)
			value = newVal;
	}

	void decrement() override
	{
		T newVal = value - inc;
		if (hasMinMax && newVal >= min)
			value = newVal;
	}

	virtual void SetFromString(const char *s) override;

	virtual void set(T newVal)
	{
		if (hasMinMax && newVal > max)
			newVal = max;
		if (hasMinMax && newVal < min)
			newVal = min;
		value = newVal;
	}

	// use this straight away
	const char *print() override
	{
		static char pString[31];

		memset((void *)pString, 0, 31);

		strcpy_s(pString, std::to_string(value).c_str());

		return pString;
	}

	T Value()
	{
		return value;
	}
	bool HasMinMax()
	{
		return hasMinMax;
	}
	T Min()
	{
		return min;
	}
	T Max()
	{
		return max;
	}
	void SetValue(T v)
	{
		value = v;
	}
	vgui::Panel *&getPanel() override;
};

template <>
class F1_ConVar<bool> : public F1_BaseConVar
{
	bool value;

public:
	F1_ConVar(const char *name, const char *internalName, bool val, F1_IConVar *p)
	    : F1_BaseConVar(name, internalName, p), value(val){};

	const char *print() override
	{
		return value ? "true" : "false";
	}

	void increment() override
	{
		value = true;
	}

	void decrement() override
	{
		value = false;
	}

	bool Value()
	{
		return value;
	}
	void SetValue(bool v)
	{
		value = v;
	}

	void SetFromString(const char *s) override
	{
		if (strcmp(s, "true") == 0) {
			increment();
		} else {
			decrement();
		}
	}

	vgui::Panel *&getPanel() override;
};

struct Switch
{
};

template <>
class F1_ConVar<Switch> : public F1_ConVar<bool>
{
public:
	F1_ConVar(const char *name, const char *internalName, bool val)
	    : F1_ConVar<bool>(name, internalName, val, nullptr){};
	vgui::Panel *&        getPanel() override;
	std::function<void()> performLayoutFunc;
	// void setLayoutFunc(std::function<void()> f);
};

template <typename T>
struct translation_t
{
	T           v;
	const char *t;
};

struct IEnum
{
};

template <typename T>
struct Enum : public IEnum
{
	T value;

	// we use vector here to make things much easier
	std::vector<translation_t<T>> translations;

	Enum(T val, std::vector<translation_t<T>> trans)
	    : value(val), translations(trans)
	{
		static_assert(sizeof(T) == sizeof(int), "Sizeof T must be sizeof int becuase of reasons");
	}

	// TODO improve efficiency
	const char *findTranslation()
	{
		for (auto &tr : translations) {
			if (tr.v == value)
				return tr.t;
		}

		return "<null>";
	}
};

template <>
class F1_ConVar<IEnum> : public F1_ConVar<int>
{
public:
	F1_ConVar(const char *name, const char *internalName, int def, int min, int max, F1_IConVar *p)
	    : F1_ConVar<int>(name, internalName, def, (int)min, (int)max, 1, p)
	{
	}

	vgui::Panel *&getPanel() override;
};

template <typename T /*, typename = std::enable_if < std::is_same<typename std::underlying_type<T>::type, int>::value >*/>
class F1_ConVar<Enum<T>> : public F1_ConVar<IEnum>
{
	Enum<T> value;

public:
	F1_ConVar(const char *name, const char *internalName, Enum<T> val, T min, T max, F1_IConVar *p)
	    : F1_ConVar<IEnum>(name, internalName, (int)val.value, (int)min, (int)max, p), value(val)
	{
	}

	const char *print() override
	{
		value.value = (T)F1_ConVar<int>::Value();
		return value.findTranslation();
	}

	void increment() override
	{
		F1_ConVar<int>::increment();
		value.value = (T)F1_ConVar<IEnum>::Value();
	}

	void decrement() override
	{
		F1_ConVar<int>::decrement();
		value.value = (T)F1_ConVar<IEnum>::Value();
	}

	T Value()
	{
		return (T)F1_ConVar<int>::Value();
	}
	void SetValue(T val)
	{
		value.value = val;
		F1_ConVar<int>::SetValue(val);
	}

	void SetFromString(const char *s)
	{
		if (!isalpha(s[0])) {
			// use the int baseclass to set the value
			F1_ConVar<int>::SetFromString(s);
			value.value = (T)F1_ConVar<int>::Value();
		} else {
			// string compare until we find a matching value
			for (auto &t : value.translations) {
				if (strcmp(s, t.t) == 0) {
					F1_ConVar<int>::SetValue((int)t.v);
					value.value = (T)F1_ConVar<int>::Value();

					return;
				}
			}
			// TODO: do something if we dont find a match
		}
	}

	const Enum<T> &getEnum()
	{
		return value;
	}
};

class F1_BindableConVar : public ConVar, public F1_ConVar<bool>
{
public:
	enum class mode
	{
		holddown,
		toggle,
	};

private:
	const char *internalName;

public:
	// TODO: func these
	std::string  lastKey;
	std::string  oldBindString; // this is the bind string before we bind
	ButtonCode_t lastKeyCode;
	mode         md;

	F1_BindableConVar(const char *name, const char *internalName, bool val, F1_IConVar *p);
	static void   UpdateVar(IConVar *var, const char *pOldVal, float flOldVal);
	void          updateKey(const char *newKey, F1_BindableConVar::mode);
	void          SetFromString(const char *s);
	vgui::Panel *&getPanel() override;
	static void   KeyInputThread();
	void          SetValue(bool t);

	void Init()
	{
		ConVar::Init();
	}
};
