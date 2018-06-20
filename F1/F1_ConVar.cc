#include "stdafx.hh"

#include "F1_ConVar.hh"

#include <tier1/convar.h>
#include <tier2/keybindings.h>

#include "inputsystem/iinputsystem.h"

#include "F1_VGUIMenu.hh"

// windows.h problems
#undef PropertySheet

#include <vgui_controls/PropertyPage.h>
#include <vgui_controls/PropertySheet.h>

#include "CommonControls.hh"

#include <tier0/memdbgon.h>

// TODO: dont hardcode !
// is there a better way to do this?
// F1_IConVar *sConVarArray[1024];
// F1_IConVar **gConvarArray = sConVarArray;
// const int gConvarArraySize = sizeof(sConVarArray);

bool        F1_ConVarAccessor::isFirst          = true;
F1_IConVar *F1_ConVarAccessor::staticConVarTail = nullptr;
F1_IConVar *F1_ConVarAccessor::staticConVarHead = nullptr;

long long oldSerial = 0;
F1_BaseConVar::F1_BaseConVar(const char *n, const char *internalName, F1_IConVar *p)
    : par(p), panel(nullptr)
{
	// Name = new char[ 64 ];
	// memset(Name, 0, 64);
	strcpy_s(displayName, n);
	strcpy_s(this->internalName, internalName);
}

using namespace vgui;

class CMenuCvarChangePanel : public PropertyPage
{
	DECLARE_CLASS_SIMPLE(CMenuCvarChangePanel, PropertyPage);

	void addItem(Panel *p)
	{

		p->SetParent(this);

		// add to items
		items.AddToTail(p);
	}

	void addItem(F1_IConVar *p)
	{
		using namespace CommonControls;
		if (p != NULL) {
			addItem(p->getPanel());
		}
	}

public:
	CMenuCvarChangePanel(F1_IConVar *s, class F1_VGUIMenu *menu, Panel *parent)
	    : BaseClass(parent, s->name()), m_pMenu(menu), thisSwitch(s)
	{
		SetParent(parent);

		SetKeyBoardInputEnabled(true);
		SetMouseInputEnabled(true);

		SetProportional(false);

		ivgui()->AddTickSignal(GetVPanel());
	}

	~CMenuCvarChangePanel()
	{
	}

	virtual void OnTick() override
	{
		SetVisible(visstate);
	}

	virtual void SetVisible(bool state) override
	{
		visstate = state;
		BaseClass::SetVisible(state);
		if (state) {
			SetAlpha(255);
			PerformLayout();
		}
	}

public:
	void addItems(CUtlVector<F1_IConVar *> &p)
	{
		for (auto &c : p) {
			addItem(c);
		}

		PerformLayout();

		onetimeResize = false;
	}

	virtual void PerformLayout() override
	{
		auto *switchConVar = static_cast<F1_ConVar<Switch> *>(thisSwitch);

		if (switchConVar->performLayoutFunc) {
			switchConVar->performLayoutFunc();
		} else {
			// auto arranage elements
			int totalItems = items.Count();

			int cury = 4;

			int wide = 0, tall = 0;

			GetSize(wide, tall);

			for (auto &i : items) {
				i->SetPos(8, cury);
				i->SetSize(wide - 10, 25);

				// perform layout on the child (make sure it is updated!)
				i->PerformLayout();

				cury += 25 + 4;
			}

			if (onetimeResize)
				SetSize(wide, cury + 20);
		}

		BaseClass::PerformLayout();
	}

	void Reset()
	{
		items.PurgeAndDeleteElements();
	}

public:
	CUtlVector<Panel *> items;
	F1_VGUIMenu *       m_pMenu;

	F1_IConVar *thisSwitch = NULL;

	bool visstate = true;

	bool onetimeResize = true;
};

//#define SHOULD_LOG_PANEL
#ifdef SHOULD_LOG_PANEL
#define LOGPANELCREATION() Log::Console("Creating panel %s", InternalName())
#else
#define LOGPANELCREATION()
#endif

vgui::Panel *&F1_ConVar<Switch>::getPanel()
{
	if (auto *&panel = F1_BaseConVar::getPanel()) {
		return panel;
	} else {
		LOGPANELCREATION();
		panel = new CMenuCvarChangePanel(this, mypanel->GetMenu(), (Panel *)mypanel->GetMenu());

		CMenuCvarChangePanel *newPanel = (CMenuCvarChangePanel *)panel;

		newPanel->Reset();

		// find all convars that are its parents
		for (F1_IConVar *pVar = F1_ConVarAccessor::staticConVarTail; pVar != nullptr; pVar = pVar->next) {
			// auto *pVar = gConvarArray[i];

			if (pVar != NULL) {
				if (pVar->parent() == this) {
					newPanel->addItem(pVar);
				}
			}
		}

		// newPanel->addItems(children);

		// newPanel->Activate();

		newPanel->SetVisible(true);

		newPanel->PerformLayout();

		return panel;
	}
}

template <>
vgui::Panel *&F1_ConVar<float>::getPanel()
{
	if (auto *&panel = F1_BaseConVar::getPanel()) {
		return panel;
	} else {
		LOGPANELCREATION();
		panel = new CommonControls::LabeledTextEntry<float>(parent()->getPanel(), name(), this);
		return panel;
	}
}

template <>
vgui::Panel *&F1_ConVar<int>::getPanel()
{
	if (auto *&panel = F1_BaseConVar::getPanel()) {
		return panel;
	} else {
		LOGPANELCREATION();
		panel = new CommonControls::LabeledTextEntry<int>(parent()->getPanel(), name(), this);
		return panel;
	}
}

vgui::Panel *&F1_ConVar<bool>::getPanel()
{
	if (auto *&panel = F1_BaseConVar::getPanel()) {
		return panel;
	} else {
		LOGPANELCREATION();
		panel = new CommonControls::ConvarCheckbutton(parent()->getPanel(), name(), this);
		//((CommonControls::ConvarCheckbutton *)panel)->SetSelected (this->Value ());
		this->SetValue(this->Value());
		return panel;
	}
}

vgui::Panel *&F1_ConVar<IEnum>::getPanel()
{
	if (auto *&panel = F1_BaseConVar::getPanel()) {
		return panel;
	} else {
		LOGPANELCREATION();
		panel = new CommonControls::ConvarLabeledComboBox(parent()->getPanel(), name(), this);
		return panel;
	}
}

vgui::Panel *&F1_BindableConVar::getPanel()
{
	if (auto *&panel = F1_BaseConVar::getPanel()) {
		return panel;
	} else {
		LOGPANELCREATION();
		panel = new CommonControls::LabledBindableConVar(parent()->getPanel(), name(), this);
		return panel;
	}
}

void F1_BindableConVar::UpdateVar(IConVar *var, const char *pOldValue, float flOldValue)
{
	// do we need to check the dynamic_cast here as this will only be called from a convar that is of that type?

	F1_BindableConVar *pBindableConvar = (F1_BindableConVar *)var;

	pBindableConvar->SetValue(((ConVar *)var)->GetBool());
	Log::Console("variable %s (%s) changed from %s to %s", pBindableConvar->name(), pBindableConvar->internalName, pOldValue, pBindableConvar->print());
}

// typedef void ( *FnChangeCallback_t )( IConVar *var, const char *pOldValue, float flOldValue )
F1_BindableConVar::F1_BindableConVar(const char *name, const char *internalName, bool val, F1_IConVar *p)
    : F1_ConVar<bool>(name, internalName, false, p), ConVar(internalName, val ? "1" : "0", FCVAR_NONE, name, &UpdateVar), internalName(internalName), lastKey("<default>"), md(mode::holddown)
{
	//Log::Console ("Creating bindable convar %s", name);
	// ConvarToBindableTable[(DWORD)&internalConVar] = (DWORD)this;
}

void F1_BindableConVar::SetValue(bool t)
{
	return F1_ConVar<bool>::SetValue(t);
}

void F1_BindableConVar::updateKey(const char *newKey, mode m)
{
	// bind x action
	// alias x_toggletrue "x 1; alias x_toggle x_togglefalse";alias x_togglefalse "x 0; alias x_toggle x_toggletrue";alias x_toggle "x_toggle";bind key x_toggle
	// alias +x_toggle "x 1"
	// alias -x_toggle "x 0"
	// bind k +x_toggle

	ButtonCode_t k = g_pInputSystem->StringToButtonCode(newKey);

	if (k != ButtonCode_t::BUTTON_CODE_INVALID) {
		// Log::Console("binding key: %s", newKey);
		lastKey = newKey;
		md      = m;

		lastKeyCode = k;
	} else {
		Log::Console("Invalid key: %s, unbinding!", newKey);
		lastKeyCode = ButtonCode_t::BUTTON_CODE_INVALID;
	}
}

inline void F1_BindableConVar::SetFromString(const char *s)
{
	gInts->Cvar->ConsoleColorPrintf({255, 0, 0, 255}, "Error: at this time you are unable to set the value of a binable convar from f1_cvar_set\n");
}

void F1_BindableConVar::KeyInputThread()
{

	if (gInts->Engine->Con_IsVisible() == true)
		return;

	// loop through the convars
	// find bindable convars
	// check if the button is up or down
	// set state accordingly

	for (F1_ConVarAccessor *p = F1_ConVarAccessor::staticConVarTail; p != nullptr; p = p->next) {
		if (F1_BindableConVar *pBindable = dynamic_cast<F1_BindableConVar *>(p)) {
			if (pBindable->lastKeyCode == BUTTON_CODE_INVALID)
				continue;

			if (g_pInputSystem->IsButtonDown(pBindable->lastKeyCode)) {
				// button is being pressed
				// if it is a holddown button then it should be set to true
				if (pBindable->md == F1_BindableConVar::mode::holddown) {
					pBindable->SetValue(true);
				}

				// for toggle keybinds we will set the value when the key rises
				// this prevents the problem of the value switching from true to false rapidly
			} else {
				// key is not down
				// for holddown set the value to false
				if (pBindable->md == F1_BindableConVar::mode::holddown) {
					pBindable->SetValue(false);
				} else if (pBindable->md == F1_BindableConVar::mode::toggle) {
					pBindable->SetValue(!pBindable->Value());
				}
			}
		}
	}
}

inline F1_ConVarAccessor::F1_ConVarAccessor()
{
	F1_IConVar *pConVar = (F1_IConVar *)this;
	if (staticConVarTail == nullptr) {
		staticConVarTail = pConVar;
		staticConVarHead = pConVar;
	} else {
		staticConVarHead->next = pConVar;
		staticConVarHead       = pConVar;
		staticConVarHead->next = nullptr;
	}
}

inline F1_ConVarAccessor::~F1_ConVarAccessor()
{
	for (F1_IConVar *p = staticConVarTail; p != nullptr; p = p->next) {
		if (p->next == this) {
			p->next = this->next;
			if (this == staticConVarHead) {
				staticConVarHead = p;
				p->next          = nullptr;
			}
			return;
		}
	}

	// we are the tail
	staticConVarTail = this->next;
}

// TODO: set the text inputs string

template <>
void F1_ConVar<float>::SetFromString(const char *s)
{
	float val = std::atof(s);

	set(val);

	auto panel = (CommonControls::LabeledTextEntry<float> *)getPanel();

	panel->m_pTextEntry->SetText(s);
}

template <>
void F1_ConVar<int>::SetFromString(const char *s)
{
	int val = std::atoi(s);

	set(val);

	auto panel = (CommonControls::LabeledTextEntry<int> *)getPanel();

	panel->m_pTextEntry->SetText(s);
}
