#pragma once

#include "../SDK/SDK.hh"

#include "F1_ConVar.hh"

static ConVar f1_show_menu_panel ("f1_show_menu_panel", "0", FCVAR_NONE, "Sets the state of the MenuPanel <state>");

// IMyPanel.h
class IMyFrame
{
public:
	virtual void               Create (vgui::VPANEL parent) = 0;
	virtual void               Destroy (void)               = 0;
	virtual void               Activate (void)              = 0;
	virtual void               AddCvar (F1_IConVar *)       = 0;
	virtual void               ClearCvars (void)            = 0;
	virtual class F1_VGUIMenu *GetMenu ()                   = 0;
};

extern IMyFrame *mypanel;

// CMyPanel class: Tutorial example class
class F1_VGUIMenu : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE (F1_VGUIMenu, vgui::Frame)
	// CMyPanel : This Class / Frame : BaseClass

	F1_VGUIMenu (vgui::VPANEL parent); // Constructor
	~F1_VGUIMenu (){};                 // Destructor
public:
	// VGUI overrides:
	virtual void OnTick () override;
	virtual void OnCommand (const char *pcCommand) override;

	virtual void Activate (void) override;

	virtual void PerformLayout (void) override;

	virtual void SetVisible (bool state) override
	{
		BaseClass::SetVisible (state);
		if (state)
			SetAlpha (255);
		else
			SetAlpha (0);
	}

	void AddSwitch (F1_IConVar *pConVar);

	// void ClearConVars();

	virtual void OnCloseFrameButtonPressed () override
	{
		BaseClass::OnCloseFrameButtonPressed ();
		OnCommand ("turnoff");
	}

public:
	// Other used VGUI control Elements:

	// Our Code Defined Control
	// Button *m_pCloseButton;

	bool m_bDirty = true;

	class CMenuSheet *m_pSheet;

	// F1_IConVar *menuArray[255];
	F1_ConVar<Switch> *switchArray[127];
	int                iSwitchItems = 0;
	int                iMenuItems   = 0;
};
