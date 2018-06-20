#include "stdafx.hh"

#include "CHack.hh"
#include "F1_VGUIMenu.hh"
#include "modules.hh"

#include "CommonControls.hh"

#define RAD_TELEMETRY_DISABLED
#include <tier0/vprof.h>

#include <tier0/memdbgon.h>

using namespace vgui;

class CMenuSheet;

// TODO: tooltips or a help page

class CMenuSheet : public PropertySheet
{
	DECLARE_CLASS_SIMPLE(CMenuSheet, PropertySheet);

public:
	CMenuSheet(Panel *panel, const char *panelName)
	    : BaseClass(panel, panelName)
	{
		// m_pCvarListPage = new CMenuCvarListPage(this, (F1_VGUIMenu *)panel, "CvarList");

		// AddPage(m_pCvarListPage, "Switch List");
		// AddPage(m_pCvarChangePage, "Cvar Change");
	}

	void addSwitch(F1_IConVar *pConVar)
	{
		auto *newPage = pConVar->getPanel();
		newPage->SetParent(this);
		AddPage(newPage, pConVar->name());
		// SetActivePage(newPage);
	}

public:
	CUtlVector<Panel *> panels;
	// CMenuCvarListPage *m_pCvarListPage;
};

float activateTime = 0.0f;

void UpdateActivateTime(IConVar *pVar, const char *oldv, float foldv)
{
	if (foldv == 0.0f)
		activateTime = gInts->Globals->curtime;
}

// Constuctor: Initializes the Panel
F1_VGUIMenu::F1_VGUIMenu(VPANEL parent)
    : BaseClass(NULL, "MyPanel", false)
{
	SetParent(parent);

	SetKeyBoardInputEnabled(true);
	SetMouseInputEnabled(true);

	SetProportional(false);
	SetTitleBarVisible(true);
	SetMinimizeButtonVisible(false);
	SetMaximizeButtonVisible(false);
	SetCloseButtonVisible(true);
	SetSizeable(true);
	SetMoveable(true);
	SetVisible(true);

	const wchar_t *title = L"F1 Menu";

	SetTitle(title, false);

	SetSize(800, 700);

	SetPos(100, 100);

	ivgui()->AddTickSignal(GetVPanel());

	m_pSheet = new CMenuSheet(this, "MenuSheet");

	f1_show_menu_panel.InstallChangeCallback(&UpdateActivateTime);
}

// Class: CMyPanelInterface Class. Used for construction.
class CMyPanelInterface : public IMyFrame
{
private:
	F1_VGUIMenu *MyFrame;

public:
	CMyPanelInterface()
	{
		MyFrame = NULL;
	}
	void Create(VPANEL parent)
	{
		MyFrame = new F1_VGUIMenu(parent);
	}
	void Destroy()
	{
		if (MyFrame) {
			MyFrame->SetParent((Panel *)NULL);
			delete MyFrame;
		}
	}

	void Activate(void) override
	{
		if (MyFrame) {
			MyFrame->Activate();
		}
	}

	void AddCvar(F1_IConVar *p)
	{
		if (MyFrame) {
			MyFrame->AddSwitch(p);
		}
	}

	void ClearCvars(void)
	{
		// static_assert(0, "bad");
		// if(MyFrame)
		//{
		//  MyFrame->ClearConVars();
		//}
	}

	F1_VGUIMenu *GetMenu()
	{
		return MyFrame;
	}
};

static CMyPanelInterface g_MyPanel;
IMyFrame *               mypanel = (IMyFrame *)&g_MyPanel;

CON_COMMAND(activate_menu_panel, "Activates the menu panel")
{
	mypanel->Activate();
};

DEFINE_RECURSE_CALL_FUNCTION_2_ARG(_menuUpdate, F1_IConVar **, int &);

int  forceUpdateTick = 100;
int  curUpdateTick   = 0;
bool oneTimeLoad     = true;
void F1_VGUIMenu::OnTick()
{
	BaseClass::OnTick();

	SetVisible(f1_show_menu_panel.GetBool()); // CL_SHOWMYPANEL / 1 BY DEFAULT

	int currIndex = 0;

	// TODO we need to do this to ensure that when switches close, the items are deleted
	// memset is a fast operation, so this should not be a problem
	// memset(gConvarArray, 0, sizeof(gConvarArray));

	// RecurseCall_menuUpdate(gConvarArray, currIndex, ACTIVE_HACKS);

	// static int oldItems = 0;

	// iMenuItems = currIndex;

	if (m_bDirty) {
		// ClearConVars(); // reset displayed convars

		iSwitchItems = 0;

		memset(switchArray, 0, sizeof(switchArray));

		F1_ConVarAccessor *prev = nullptr;
		for (auto *pConVar = F1_ConVarAccessor::staticConVarTail; pConVar != nullptr; pConVar = pConVar->next) {
			// auto pConVar = gConvarArray[i];

			if (pConVar == prev)
				break;

			if (pConVar != NULL) {
				// AddConVar(pConVar);

				if (auto pSwitchConVar = dynamic_cast<F1_ConVar<Switch> *>(pConVar)) {
					AddSwitch(pConVar);
				}
			}
			prev = pConVar;
		}

		m_bDirty = false;
	}

	// oldItems = iMenuItems;

	if (curUpdateTick >= forceUpdateTick) {
		curUpdateTick = 0;
		// m_bDirty = true;
	} else
		curUpdateTick++;

	if (activateTime + 1.0f < gInts->Globals->curtime) {
		if (f1_show_menu_panel.GetBool() && GetAsyncKeyState(VK_INSERT) & 0x8000) {
			// close the menu
			f1_show_menu_panel.SetValue(false);
		}
	}
}

void F1_VGUIMenu::OnCommand(const char *pcCommand)
{
	BaseClass::OnCommand(pcCommand);

	if (!stricmp(pcCommand, "turnoff")) {
		f1_show_menu_panel.SetValue(0);
	}
}

void F1_VGUIMenu::Activate(void)
{
	BaseClass::Activate();

	//SetAlpha (255);
}

void F1_VGUIMenu::PerformLayout(void)
{
	BaseClass::PerformLayout();

	int wide, tall;

	GetSize(wide, tall);

	const int inset     = 8;
	const int topHeight = 28;

	IScheme *pScheme = scheme()->GetIScheme(GetScheme());

	m_pSheet->SetBorder(pScheme->GetBorder("DepressedButtonBorder"));

	m_pSheet->SetPos(inset, inset + topHeight);
	m_pSheet->SetSize(wide - inset * 2, tall - inset * 2 + topHeight * 2);
	// pListPanel->SetBounds( inset, 0, wide, tall );
}

void F1_VGUIMenu::AddSwitch(F1_IConVar *pConVar)
{
	m_pSheet->addSwitch(pConVar);
}
