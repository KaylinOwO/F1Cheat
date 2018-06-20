#pragma once

#include "../SDK/IHack.hh"
#include "F1_ConVar.hh"

#include "../SDK/F1_Gui.hh"

class CRadar : public IHack< CRadar >, public F1_CWindow
{
    F1_ConVar< Switch > pRadarSwitch = F1_ConVar< Switch >( "Radar", "f1_radar_switch", false );
    F1_ConVar< bool > pEnabledRadar = F1_ConVar< bool >( " - Enabled", "f1_radar_enabled", false, &pRadarSwitch );
    F1_ConVar< int > pSize = F1_ConVar< int >( " - Size", "f1_radar_size", 130, 0, 200, 5, &pRadarSwitch );
    F1_ConVar< float > pRange = F1_ConVar< float >( " - Range", "f1_radar_range", 2000, 0, 10000, 100, &pRadarSwitch );
    F1_ConVar< int > pAlpha = F1_ConVar< int >( " - Alpha", "f1_radar_alpha", 125, 0, 255, 1, &pRadarSwitch );

    using BaseClass = F1_CWindow;

public:
    CRadar();

    void render() override;

    void init();

    void processEntity( CBaseEntity *pBaseEntity );

    F1_Point getWidthHeight() override;

    F1_Rect getBounds() override;
};

extern CRadar gRadar;
