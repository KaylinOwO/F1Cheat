#pragma once

#include "../SDK/SDK.hh"

#include "F1_VGUIMenu.hh"

#include <tier0/memdbgon.h>

namespace CommonControls {
template <typename T>
class LabeledTextEntry : public vgui::Panel
{

    DECLARE_CLASS_SIMPLE(LabeledTextEntry, vgui::Panel);

    vgui::TextEntry *m_pTextEntry;

    vgui::Label *m_pLabel;

    F1_ConVar<T> *_cv;

    char newName[255];

public:
    LabeledTextEntry(vgui::Panel *parent, const char *panelName, F1_ConVar<T> *convar)
        : BaseClass(parent, panelName), _cv(convar)
    {
        m_pTextEntry = new vgui::TextEntry(this, "TextEntry");
        m_pTextEntry->AddActionSignalTarget(this);

        createNewName();

        m_pLabel = new vgui::Label(this, "Label", newName);

        m_pTextEntry->SendNewLine(true);

        m_pTextEntry->SetText(_cv->print());
    }

    virtual ~LabeledTextEntry()
    {
        delete m_pTextEntry;
        delete m_pLabel;
    }

    void createNewName()
    {
        sprintf_s(newName, "%s <%s - %s>", _cv->name(), std::to_string(_cv->Min()).c_str(), std::to_string(_cv->Max()).c_str());
    }

    virtual void PerformLayout() override
    {
        BaseClass::PerformLayout();

        int wide = 0, tall = 0;
        GetSize(wide, tall);

        int labelWide = 0, labelTall = 0;
        m_pLabel->GetSize(labelWide, labelTall);

        int  minWide = 0, minTall = 0;
        auto font = m_pLabel->GetFont();

        gInts->Surface->GetTextSize(font, newName, minWide, minTall);

        m_pLabel->SetSize(minWide + 4, tall);

        // Log::Console( "m_pLabel->GetMinSize()==%d:%d", minWide, minTall );

        m_pTextEntry->SetBounds(labelWide, 0, wide - labelWide - 8, tall);
    }

    MESSAGE_FUNC(OnTextChanged, "TextKillFocus");
};

class ConvarCheckbutton : public vgui::CheckButton
{

    DECLARE_CLASS_SIMPLE(ConvarCheckbutton, CheckButton);

    F1_ConVar<bool> *_cv;

public:
    ConvarCheckbutton(vgui::Panel *parent, const char *panelName, F1_ConVar<bool> *convar)
        : BaseClass(parent, panelName, convar->name()), _cv(convar)
    {
        AddActionSignalTarget(this);
    }

    virtual void PerformLayout() override
    {
        BaseClass::PerformLayout();

        SetSelected(_cv->Value());

        int wide = 0, tall = 0;

        GetSize(wide, tall);

        SetSize(wide, tall);
    }

    MESSAGE_FUNC_INT(OnButtonToggled, "ButtonToggled", state)
    {
        state ? _cv->increment() : _cv->decrement();
    }
};

class ConvarLabeledComboBox : public vgui::Panel
{
    DECLARE_CLASS_SIMPLE(ConvarLabeledComboBox, vgui::Panel);

    F1_ConVar<Enum<int>> *_cv;

    vgui::Label *m_pLabel;

    vgui::ComboBox *m_pComboBox;

    wchar_t *labelText;

public:
    ConvarLabeledComboBox(vgui::Panel *parent, const char *panelName, F1_ConVar<IEnum> *convar)
        : BaseClass(parent, panelName), _cv(reinterpret_cast<F1_ConVar<Enum<int>> *>(convar))
    {
        // AddActionSignalTarget(this);

        //Log::Console ("Initing for convar %s", _cv->name ());

        m_pLabel = new vgui::Label(this, "Label", _cv->name());

        m_pComboBox = new vgui::ComboBox(this, "ComboBox", 0, false);

        m_pComboBox->AddActionSignalTarget(this);

        // add items from the convar
        int x = 0;
        for (auto &t : _cv->getEnum().translations) {
            m_pComboBox->AddItem(t.t, new KeyValues("Selection", "value", t.v));
            if (t.v == _cv->Value())
                m_pComboBox->ActivateItem(x);
            x++;
        }

        auto size = strlen(_cv->name()) + 1;

        labelText = new wchar_t[size];

        V_strtowcs(_cv->name(), strlen(_cv->name()), labelText, -1);
    }

    virtual ~ConvarLabeledComboBox()
    {
        delete m_pLabel;
        delete m_pComboBox;
        delete[] labelText;
    }

    void PerformLayout() override
    {
        BaseClass::PerformLayout();

        for (int i = 0; i < _cv->getEnum().translations.size(); i++) {
            auto &t = _cv->getEnum().translations[i];
            if (t.v == _cv->Value()) {
                if (m_pComboBox->GetActiveItem() != i)
                    m_pComboBox->SilentActivateItem(i);
            }
        }

        int wide = 0, tall = 0;
        GetSize(wide, tall);

        int labelWide = 0, labelTall = 0;
        m_pLabel->GetSize(labelWide, labelTall);

        int  minWide = 0, minTall = 0;
        auto font = m_pLabel->GetFont();

        gInts->Surface->GetTextSize(font, labelText, minWide, minTall);

        m_pLabel->SetSize(minWide + 4, tall);
        // Log::Console( "m_pLabel->GetMinSize()==%d:%d", minWide, minTall );

        m_pComboBox->SetBounds(labelWide, 0, wide - labelWide - 8, tall);
    }

    MESSAGE_FUNC(OnMenuItemSelected, "TextChanged")
    {
        _cv->SetValue(m_pComboBox->GetActiveItemUserData()->GetInt("value"));
        // Log::Console("OnActivateItem new value == %s", _cv->print());
    }
};

class LabledBindableConVar : public vgui::Panel
{
    DECLARE_CLASS_SIMPLE(LabledBindableConVar, vgui::Panel);

    vgui::TextEntry *m_pTextEntry;

    vgui::Label *m_pLabel;

    F1_BindableConVar *_cv;

    vgui::ComboBox *m_pComboBox;

public:
    LabledBindableConVar(vgui::Panel *parent, const char *panelName, F1_BindableConVar *convar)
        : BaseClass(parent, panelName), _cv(convar)
    {
        m_pTextEntry = new vgui::TextEntry(this, "TextEntry");
        m_pTextEntry->AddActionSignalTarget(this);

        m_pLabel = new vgui::Label(this, "Label", _cv->name());

        m_pComboBox = new vgui::ComboBox(this, "typeBox", 0, false);

        m_pTextEntry->SendNewLine(true);
        m_pTextEntry->SetText(_cv->lastKey.c_str());

        m_pComboBox->AddItem("Press", new KeyValues("Selection", "value", 0));
        // TODO reimpl
        m_pComboBox->AddItem("Toggle", new KeyValues("Selection", "value", 1));

        m_pComboBox->ActivateItem(int(_cv->md));

        m_pComboBox->AddActionSignalTarget(this);
    }

    virtual ~LabledBindableConVar()
    {
        delete m_pTextEntry;
        delete m_pComboBox;
        delete m_pLabel;
    }

    virtual void PerformLayout() override
    {
        BaseClass::PerformLayout();

        int wide = 0, tall = 0;
        GetSize(wide, tall);

        int labelWide = 0, labelTall = 0;
        m_pLabel->GetSize(labelWide, labelTall);

        int  minWide = 0, minTall = 0;
        auto font = m_pLabel->GetFont();

        gInts->Surface->GetTextSize(font, _cv->name(), minWide, minTall);

        m_pLabel->SetSize(minWide + 4, tall);

        // Log::Console( "m_pLabel->GetMinSize()==%d:%d", minWide, minTall );

        m_pTextEntry->SetBounds(labelWide, 0, wide - labelWide - wide / 2, tall);

        m_pComboBox->SetBounds(wide / 2 + 8, 0, wide - wide / 2 - 16, tall);
    }

    MESSAGE_FUNC(OnTextKillFocus, "TextKillFocus")
    {
        char *text = new char[m_pTextEntry->GetTextLength() + 1];
        m_pTextEntry->GetText(text, m_pTextEntry->GetTextLength() + 1);

        _cv->updateKey(text, F1_BindableConVar::mode(m_pComboBox->GetActiveItemUserData()->GetInt("value")));

        delete[] text;

        // Log::Console("OnActivateItem new value == %s", _cv->print());
    }

    MESSAGE_FUNC(OnMenuItemSelected, "MenuItemSelected")
    {
        char *text = new char[m_pTextEntry->GetTextLength() + 1];
        m_pTextEntry->GetText(text, m_pTextEntry->GetTextLength() + 1);

        _cv->updateKey(text, F1_BindableConVar::mode(m_pComboBox->GetActiveItemUserData()->GetInt("value")));

        delete[] text;
    }
};
} // namespace CommonControls

template <>
inline void CommonControls::LabeledTextEntry<float>::OnTextChanged()
{
    int textlen = m_pTextEntry->GetTextLength();

    char *buf = new char[textlen + 1];

    m_pTextEntry->GetText(buf, textlen + 1);

    float val = std::atof(buf);

    _cv->set(val);

    if (_cv->Value() != val) {
        m_pTextEntry->SetText(_cv->print());
    }

    Log::Console("new value for %s == %s", _cv->name(), _cv->print());

    // clean up
    delete[] buf;
}

template <>
inline void CommonControls::LabeledTextEntry<int>::OnTextChanged()
{
    int textlen = m_pTextEntry->GetTextLength();

    char *buf = new char[textlen + 1];

    m_pTextEntry->GetText(buf, textlen + 1);

    int val = std::atoi(buf);

    _cv->set(val);

    if (_cv->Value() != val) {
        m_pTextEntry->SetText(_cv->print());
    }

    // Log::Console("new value for %s == %s", _cv->name(), _cv->print());

    // clean up
    delete[] buf;
}

template <typename T>
void CommonControls::LabeledTextEntry<T>::OnTextChanged()
{
    extern int NOT_IMPLEMENTED;
    NOT_IMPLEMENTED = 0;
    // static_assert(0);
}

#include <tier0/memdbgoff.h>
