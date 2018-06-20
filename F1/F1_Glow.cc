#include "stdafx.hh"

#include "CGlow.hh"
#include "F1_Glow.hh"

#include "../SDK/CDrawManager.hh"
#include "Panels.hh"

#include <tier0/memdbgon.h>

ITexture *InitRenderTarget(int w, int h, RenderTargetSizeMode_t sizeMode, ImageFormat fmt, MaterialRenderTargetDepth_t depth, bool bHDR, char *pStrOptionalName = NULL)
{

	int textureFlags = TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT;
	if (depth == MATERIAL_RT_DEPTH_ONLY)
		textureFlags |= TEXTUREFLAGS_POINTSAMPLE;

	int renderTargetFlags = bHDR ? CREATERENDERTARGETFLAGS_HDR : 0;

	// NOTE: Refcount returned by CreateRenderTargetTexture is 1
	return gInts->MatSystem->CreateNamedRenderTargetTextureEx(pStrOptionalName, w, h, sizeMode, fmt, depth, textureFlags, renderTargetFlags);
}

#include "materialsystem/imaterialsystem.h"
#include "materialsystem/imaterialsystemhardwareconfig.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/itexture.h"
#include "model_types.h"
#include "tier1/keyvalues.h"

//#include "view_scene.hh" //Uncomment me if you plan to use multiple screeneffects at once.

#include "F1_Glow.hh"
#include "Panels.hh"

ADD_SCREENSPACE_EFFECT(CEntGlowEffect, ge_entglow);

static float rBlack[4] = {0, 0, 0, 1};
static float rWhite[4] = {1, 1, 1, 1};

#define FULL_FRAME_TEXTURE "_rt_FullFrameFB"

void CEntGlowEffect::Init(void)
{

	// init the textures / materials that we need here rather than doing an
	// expensive lookup each render
	pMatGlowColor = gInts->MatSystem->FindMaterial("dev/glow_color", TEXTURE_GROUP_OTHER, true);

	// Get material and texture pointers
	pRtQuarterSize1 = gInts->MatSystem->FindTexture("_rt_SmallFB1", TEXTURE_GROUP_RENDER_TARGET);

	pRtFullFrame = gInts->MatSystem->FindTexture(FULL_FRAME_TEXTURE, TEXTURE_GROUP_RENDER_TARGET);

	int textureFlags      = TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT | TEXTUREFLAGS_EIGHTBITALPHA;
	int renderTargetFlags = CREATERENDERTARGETFLAGS_HDR;

	// create full size buffers but we might not use the whole size of the
	// texture
	// theoretically if we only use half the buffer for glow the overhead of the
	// bluring *should* be less
	pRenderBuffer1 = gInts->MatSystem->CreateNamedRenderTargetTextureEx("_f1_buffer_1", pRtFullFrame->GetActualWidth(), pRtFullFrame->GetActualHeight(),
	                                                                    RT_SIZE_LITERAL, IMAGE_FORMAT_RGB888, MATERIAL_RT_DEPTH_SHARED, textureFlags, renderTargetFlags);
	pRenderBuffer1->IncrementReferenceCount();

	pRenderBuffer2 = gInts->MatSystem->CreateNamedRenderTargetTextureEx("_f1_buffer_2", pRtFullFrame->GetActualWidth(), pRtFullFrame->GetActualHeight(),
	                                                                    RT_SIZE_LITERAL, IMAGE_FORMAT_RGB888, MATERIAL_RT_DEPTH_SHARED, textureFlags, renderTargetFlags);
	pRenderBuffer2->IncrementReferenceCount();

	{
		// create the blur textures
		KeyValues *kv = new KeyValues("BlurFilterX");
		kv->SetString("$basetexture", "_f1_buffer_1");
		kv->SetInt("$ignorez", 1);
		kv->SetInt("$translucent", 1);
		kv->SetInt("$alphatest", 1);
		pMatBlurX = gInts->MatSystem->CreateMaterial_Internal("_f1_blurx", kv);
		pMatBlurX->Refresh();
	}

	{
		KeyValues *kv = new KeyValues("BlurFilterY");
		kv->SetString("$basetexture", "_f1_buffer_2");
		kv->SetInt("$bloomamount", 10);
		kv->SetInt("$bloomscale", 10);
		kv->SetInt("$ignorez", 1);
		kv->SetInt("$translucent", 1);
		kv->SetInt("$alphatest", 1);
		pMatBlurY = gInts->MatSystem->CreateMaterial_Internal("_f1_blury", kv);
		pMatBlurY->Refresh();
	}

	{
		KeyValues *kv = new KeyValues("UnlitGeneric");
		kv->SetString("$basetexture", "_f1_buffer_1");
		kv->SetInt("$additive", 1);
		pMatHaloAddToScreen = gInts->MatSystem->CreateMaterial_Internal("__f1_glow_blit", kv);
		pMatHaloAddToScreen->Refresh();
	}
}

void CEntGlowEffect::Shutdown(void)
{
	// TODO: shutdown the materials here
}

//-----------------------------------------------------------------------------
// Purpose: Render the effect
//-----------------------------------------------------------------------------

struct ShaderStencilState_t
{
	bool                        m_bEnable;
	StencilOperation_t          m_FailOp;
	StencilOperation_t          m_ZFailOp;
	StencilOperation_t          m_PassOp;
	StencilComparisonFunction_t m_CompareFunc;
	int                         m_nReferenceValue;
	uint32                      m_nTestMask;
	uint32                      m_nWriteMask;

	ShaderStencilState_t()
	{
		m_bEnable = false;
		m_PassOp = m_FailOp = m_ZFailOp = STENCILOPERATION_KEEP;
		m_CompareFunc                   = STENCILCOMPARISONFUNCTION_ALWAYS;
		m_nReferenceValue               = 0;
		m_nTestMask = m_nWriteMask = 0xFFFFFFFF;
	}

	void SetStencilState(CMatRenderContextPtr &pRenderContext)
	{
		pRenderContext->SetStencilEnable(m_bEnable);
		pRenderContext->SetStencilFailOperation(m_FailOp);
		pRenderContext->SetStencilZFailOperation(m_ZFailOp);
		pRenderContext->SetStencilPassOperation(m_PassOp);
		pRenderContext->SetStencilCompareFunction(m_CompareFunc);
		pRenderContext->SetStencilReferenceValue(m_nReferenceValue);
		pRenderContext->SetStencilTestMask(m_nTestMask);
		pRenderContext->SetStencilWriteMask(m_nWriteMask);
	}
};

ConVar f1_glow_outline_effect_enable("f1_glow_outline_effect_enable", "1", FCVAR_ARCHIVE, "Enable entity outline glow effects.");

void CEntGlowEffect::GlowOutlineAmountChangeCallback(IConVar *var, const char *pOldValue, float flOldValue)
{
	Log::Console("Glow outline amount changed!");

	ConVar *        pGlowOutlineWidth = (ConVar *)var;
	CEntGlowEffect *pGlow             = F1_GetGlowEffect();

	IMaterialVar *pBlurYAmmount = pGlow->pMatBlurY->FindVar("$bloomamount", nullptr);
	pBlurYAmmount->SetFloatValue(pGlowOutlineWidth->GetFloat());
}

void CEntGlowEffect::GlowOutlineScaleChangeCallback(IConVar *var, const char *pOldValue, float flOldValue)
{
	Log::Console("Glow outline scale changed!");

	ConVar *        pGlowOutlineWidth = (ConVar *)var;
	CEntGlowEffect *pGlow             = F1_GetGlowEffect();

	IMaterialVar *pBlurYAmmount = pGlow->pMatBlurY->FindVar("$bloomscale", nullptr);
	pBlurYAmmount->SetFloatValue(pGlowOutlineWidth->GetFloat());
}

ConVar f1_glow_disable_blur("f1_glow_disable_blur", "0", FCVAR_NONE, "Disable the blur effect (will increase fps)");
ConVar f1_glow_outline_effect_amount("f1_glow_outline_amount", "10.0f", FCVAR_ARCHIVE, "Width of glow outline effect in screen space.", &CEntGlowEffect::GlowOutlineAmountChangeCallback);
ConVar f1_glow_outline_effect_scale("f1_glow_outline_scale", "10.0f", FCVAR_ARCHIVE, "Width of glow outline effect in screen space.", &CEntGlowEffect::GlowOutlineScaleChangeCallback);
ConVar f1_glow_buffer_divisor("f1_glow_buffer_divisor", "1.0f", FCVAR_NONE, "Sets the size of the buffer that glow should render too. 1.0 is normal 2.0 is half size 0.5 is double size.");

static void SetRenderTargetAndViewPort(CMatRenderContextPtr &pRenderContext, ITexture *rt, int w, int h)
{
	pRenderContext->SetRenderTarget(rt);
	pRenderContext->Viewport(0, 0, w, h);
}

void CEntGlowEffect::RenderGlowModels(int nSplitScreenSlot, CMatRenderContextPtr &pRenderContext)
{
	// This function renders the solid glow pixels for each entity

	F1_VPROF_FUNCTION();

	pRenderContext->PushRenderTargetAndViewport();

	// Save modulation color and blend
	Vector vOrigColor;
	gInts->RenderView->GetColorModulation(vOrigColor.Base());
	float flOrigBlend = gInts->RenderView->GetBlend();

	SetRenderTargetAndViewPort(pRenderContext, pRenderBuffer1, gScreenSize.iScreenWidth, gScreenSize.iScreenHeight);

	pRenderContext->ClearColor4ub(0, 0, 0, 0);
	pRenderContext->ClearBuffers(true, false, false);

	// Set override material for glow color

	gInts->ModelRender->ForcedMaterialOverride(pMatGlowColor, OVERRIDE_NORMAL);

	ShaderStencilState_t stencilState;
	stencilState.m_bEnable         = false;
	stencilState.m_nReferenceValue = 0;
	stencilState.m_nTestMask       = 0xFF;
	stencilState.m_CompareFunc     = STENCILCOMPARISONFUNCTION_ALWAYS;
	stencilState.m_PassOp          = STENCILOPERATION_KEEP;
	stencilState.m_FailOp          = STENCILOPERATION_KEEP;
	stencilState.m_ZFailOp         = STENCILOPERATION_KEEP;

	stencilState.SetStencilState(pRenderContext);

	//==================//
	// Draw the objects //
	//==================//
	for (int i = 0; i < m_vGlowEnts.Count(); ++i) {
		const CBaseEntity *pEnt = GetBaseEntity(m_vGlowEnts[i]->m_hEnt);
		if (!pEnt || pEnt->IsDormant()) {
			// We don't exist anymore, remove us!
			delete m_vGlowEnts[i];
			m_vGlowEnts.Remove(i);
			continue;
		}

		GlowEnt *&glowEnt = m_vGlowEnts[i];

		gInts->RenderView->SetBlend(glowEnt->m_fGlowScale);
		Vector vGlowColor = Vector(glowEnt->m_fColor[0], glowEnt->m_fColor[1], glowEnt->m_fColor[2]) * glowEnt->m_fGlowScale;
		gInts->RenderView->SetColorModulation(&vGlowColor[0]); // This only sets rgb, not alpha

		pEnt->DrawModel(STUDIO_RENDER);
	}

	gInts->ModelRender->ForcedMaterialOverride(NULL, OverrideType_t::OVERRIDE_NORMAL);
	gInts->RenderView->SetColorModulation(vOrigColor.Base());
	gInts->RenderView->SetBlend(flOrigBlend);

	ShaderStencilState_t stencilStateDisable;
	stencilStateDisable.m_bEnable = false;
	stencilStateDisable.SetStencilState(pRenderContext);

	pRenderContext->PopRenderTargetAndViewport();
}

void CEntGlowEffect::Render(int x, int y, int w, int h)
{
	F1_VPROF_FUNCTION();

	// set up width and height variables
	gScreenSize.iScreenWidth  = w;
	gScreenSize.iScreenHeight = h;

	if (!gGlow.enabled.Value())
		return;

	CMatRenderContextPtr pRenderContext{gInts->MatSystem->GetRenderContext()};

	// Set override shader to the same simple shader we use to render the glow models
	gInts->ModelRender->ForcedMaterialOverride(pMatGlowColor, OVERRIDE_NORMAL);

	ShaderStencilState_t stencilStateDisable;
	stencilStateDisable.m_bEnable = false;
	float flSavedBlend            = gInts->RenderView->GetBlend();

	// Set alpha to 0 so we don't touch any color pixels
	gInts->RenderView->SetBlend(0.0f);
	pRenderContext->OverrideDepthEnable(true, false);

	int iNumGlowObjects = 0;

	// TODO: shouldnt the push / pop render pointer be here instead of in this
	// loop
	for (int i = 0; i < m_vGlowEnts.Count(); ++i) {

		const CBaseEntity *pEnt = GetBaseEntity(m_vGlowEnts[i]->m_hEnt);
		if (!pEnt) {
			// We don't exist anymore, remove us!
			delete m_vGlowEnts[i];
			m_vGlowEnts.Remove(i);
			continue;
		}

		int  removedCond = 0;
		int &cond        = pEnt->GetCond();
		int  oldcond     = cond;
		if (cond & tf_cond::TFCond_Cloaked) {
			cond &= ~tf_cond::TFCond_Cloaked;
		}
		if (cond & tf_cond::TFCond_Disguised) {
			cond &= tf_cond::TFCond_Disguised;
		}

		ShaderStencilState_t stencilState;
		stencilState.m_bEnable         = true;
		stencilState.m_nReferenceValue = 1;
		stencilState.m_CompareFunc     = STENCILCOMPARISONFUNCTION_ALWAYS;
		stencilState.m_PassOp          = STENCILOPERATION_REPLACE;
		stencilState.m_FailOp          = STENCILOPERATION_KEEP;
		stencilState.m_ZFailOp         = STENCILOPERATION_REPLACE;
		stencilState.SetStencilState(pRenderContext);

		pEnt->DrawModel(STUDIO_RENDER);

		// Need to do a 2nd pass to warm stencil for objects which are rendered
		// only when occluded
		stencilState.m_bEnable         = true;
		stencilState.m_nReferenceValue = 2;
		stencilState.m_CompareFunc     = STENCILCOMPARISONFUNCTION_ALWAYS;
		stencilState.m_PassOp          = STENCILOPERATION_REPLACE;
		stencilState.m_FailOp          = STENCILOPERATION_KEEP;
		stencilState.m_ZFailOp         = STENCILOPERATION_KEEP;
		stencilState.SetStencilState(pRenderContext);

		pEnt->DrawModel(STUDIO_RENDER);

		pEnt->GetCond() = oldcond;

		iNumGlowObjects++;
	}

	pRenderContext->OverrideDepthEnable(false, false);
	gInts->RenderView->SetBlend(flSavedBlend);
	stencilStateDisable.SetStencilState(pRenderContext);
	gInts->ModelRender->ForcedMaterialOverride(NULL, OVERRIDE_NORMAL);

	// If there aren't any objects to glow, don't do all this other stuff
	// this fixes a bug where if there are glow objects in the list, but none of them are glowing, the whole screen blooms.
	if (iNumGlowObjects <= 0)
		return;

	RenderGlowModels(-1, pRenderContext);

	// TODO: if we use buffers that arent of a 1:1 size then these need to change

	// src as in from the solid glow objects
	int nSrcWidth  = w;
	int nSrcHeight = h;

	int nViewportX      = x;
	int nViewportY      = y;
	int nViewportWidth  = w;
	int nViewportHeight = h;
	{

		float divisor        = f1_glow_buffer_divisor.GetFloat();
		int   iQuarterWidth  = nSrcWidth / divisor;
		int   iQuarterHeight = nSrcHeight / divisor;

		if (f1_glow_disable_blur.GetBool() == false) {
			pRenderContext->PushRenderTargetAndViewport();

			SetRenderTargetAndViewPort(pRenderContext, pRenderBuffer2, iQuarterWidth, iQuarterHeight);
			pRenderContext->DrawScreenSpaceRectangle(pMatBlurX, 0, 0, iQuarterWidth, iQuarterHeight, 0.0f, 0, iQuarterWidth - 1, iQuarterHeight - 1, iQuarterWidth, iQuarterHeight);

			SetRenderTargetAndViewPort(pRenderContext, pRenderBuffer1, iQuarterWidth, iQuarterHeight);
			pRenderContext->DrawScreenSpaceRectangle(pMatBlurY, 0, 0, iQuarterWidth, iQuarterHeight, 0.0f, 0, iQuarterWidth - 1, iQuarterHeight - 1, iQuarterWidth, iQuarterHeight);

			pRenderContext->PopRenderTargetAndViewport();
		}

		//=======================================================================================================//
		// At this point, pRtQuarterSize0 is filled with the fully colored glow
		// around everything as solid glowy //
		// blobs. Now we need to stencil out the original objects by only
		// writing pixels that have no            //
		// stencil bits set in the range we care about. //
		//=======================================================================================================//

		ShaderStencilState_t stencilState;
		stencilState.m_bEnable         = true;
		stencilState.m_nWriteMask      = 0x0; // We're not changing stencil
		stencilState.m_nTestMask       = 0xFF;
		stencilState.m_nReferenceValue = 0x0;
		stencilState.m_CompareFunc     = STENCILCOMPARISONFUNCTION_EQUAL;
		stencilState.m_PassOp          = STENCILOPERATION_KEEP;
		stencilState.m_FailOp          = STENCILOPERATION_KEEP;
		stencilState.m_ZFailOp         = STENCILOPERATION_KEEP;
		stencilState.SetStencilState(pRenderContext);

		pRenderContext->DrawScreenSpaceRectangle(pMatHaloAddToScreen, 0, 0, nViewportWidth, nViewportHeight,
		                                         0.0f, -0.5f, iQuarterWidth - 1, iQuarterHeight - 1, iQuarterWidth, iQuarterHeight);

		stencilStateDisable.SetStencilState(pRenderContext);
	}
}

void CEntGlowEffect::RegisterEnt(EHANDLE hEnt, Color glowColor /*=Color(255,255,255,64)*/, float fGlowScale /*=1.0f*/)
{
	// Don't add duplicates
	if (FindGlowEnt(hEnt) != -1 || !GetBaseEntity(hEnt))
		return;

	GlowEnt *newEnt      = new GlowEnt;
	newEnt->m_hEnt       = hEnt;
	newEnt->m_fColor[0]  = glowColor.r() / 255.0f;
	newEnt->m_fColor[1]  = glowColor.g() / 255.0f;
	newEnt->m_fColor[2]  = glowColor.b() / 255.0f;
	newEnt->m_fColor[3]  = glowColor.a() / 255.0f;
	newEnt->m_fGlowScale = fGlowScale;
	m_vGlowEnts.AddToTail(newEnt);
}

void CEntGlowEffect::DeregisterEnt(EHANDLE hEnt)
{
	int idx = FindGlowEnt(hEnt);
	if (idx == -1)
		return;

	delete m_vGlowEnts[idx];
	m_vGlowEnts.Remove(idx);
}

void CEntGlowEffect::SetEntColor(EHANDLE hEnt, Color glowColor)
{
	int idx = FindGlowEnt(hEnt);
	if (idx == -1)
		return;

	m_vGlowEnts[idx]->m_fColor[0] = glowColor.r() / 255.0f;
	m_vGlowEnts[idx]->m_fColor[1] = glowColor.g() / 255.0f;
	m_vGlowEnts[idx]->m_fColor[2] = glowColor.b() / 255.0f;
	m_vGlowEnts[idx]->m_fColor[3] = glowColor.a() / 255.0f;
}

void CEntGlowEffect::SetEntGlowScale(EHANDLE hEnt, float fGlowScale)
{
	int idx = FindGlowEnt(hEnt);
	if (idx == -1)
		return;

	m_vGlowEnts[idx]->m_fGlowScale = fGlowScale;
}

int CEntGlowEffect::FindGlowEnt(EHANDLE hEnt)
{
	for (int i = 0; i < m_vGlowEnts.Count(); i++) {
		if (GetBaseEntity(m_vGlowEnts[i]->m_hEnt) == GetBaseEntity(hEnt))
			return i;
	}

	return -1;
}

CEntGlowEffect *F1_GetGlowEffect()
{
	return &ge_entglow_effect;
}

//
//
//
//
//
// L4D implementation for cross reference
//
//
//
//
//
//

/*


//-----------------------------------------------------------------------------
// Purpose: Render the effect
//-----------------------------------------------------------------------------
ConVar cl_ge_glowscale( "cl_ge_glowscale", "0.4", FCVAR_CLIENTDLL );
ConVar cl_ge_glowstencil( "cl_ge_glowstencil", "1", FCVAR_CLIENTDLL );

void L4DRender( int x, int y, int w, int h )
{
    // Don't bother rendering if we have nothing to render!
    if( !m_vGlowEnts.Count() || ( IsEnabled() == false ) )
        return;

    // Grab the render context
    CMatRenderContextPtr pRenderContext( materials );

    // Apply our glow buffers as the base textures for our blurring operators
    IMaterialVar *var;
    // Set our effect material to have a base texture of our primary glow buffer
    var = m_BlurX->FindVar( "$basetexture", NULL );
    var->SetTextureValue( m_GlowBuff1 );
    var = m_BlurY->FindVar( "$basetexture", NULL );
    var->SetTextureValue( m_GlowBuff2 );
    var = m_EffectMaterial->FindVar( "$basetexture", NULL );
    var->SetTextureValue( m_GlowBuff1 );

    var = m_BlurX->FindVar( "$bloomscale", NULL );
    var->SetFloatValue( 10 * cl_ge_glowscale.GetFloat() );
    var = m_BlurY->FindVar( "$bloomamount", NULL );
    var->SetFloatValue( 10 * cl_ge_glowscale.GetFloat() );

    // Clear the glow buffer from the previous iteration
    pRenderContext->ClearColor4ub( 0, 0, 0, 255 );
    pRenderContext->PushRenderTargetAndViewport( m_GlowBuff1 );
    pRenderContext->ClearBuffers( true, true );
    pRenderContext->PopRenderTargetAndViewport();

    pRenderContext->PushRenderTargetAndViewport( m_GlowBuff2 );
    pRenderContext->ClearBuffers( true, true );
    pRenderContext->PopRenderTargetAndViewport();

    // Clear the stencil buffer in case someone dirtied it this frame
    pRenderContext->ClearStencilBufferRectangle( 0, 0, ScreenWidth(),
ScreenHeight(), 0 );

    // Iterate over our registered entities and add them to the cut-out stencil
and the glow buffer
    for( int i = 0; i < m_vGlowEnts.Count(); i++ )
    {
        if( cl_ge_glowstencil.GetInt() )
            RenderToStencil( i, pRenderContext );
        RenderToGlowTexture( i, pRenderContext );
    }

    // Now we take the built up glow buffer (m_GlowBuff1) and blur it two ways
    // the intermediate buffer (m_GlowBuff2) allows us to do this properly
    pRenderContext->PushRenderTargetAndViewport( m_GlowBuff2 );
    pRenderContext->DrawScreenSpaceQuad( m_BlurX );
    pRenderContext->PopRenderTargetAndViewport();

    pRenderContext->PushRenderTargetAndViewport( m_GlowBuff1 );
    pRenderContext->DrawScreenSpaceQuad( m_BlurY );
    pRenderContext->PopRenderTargetAndViewport();

    if( cl_ge_glowstencil.GetInt() )
    {
        // Setup the renderer to only draw where the stencil is not 1
        pRenderContext->SetStencilEnable( true );
        pRenderContext->SetStencilReferenceValue( 0 );
        pRenderContext->SetStencilTestMask( 1 );
        pRenderContext->SetStencilCompareFunction(
STENCILCOMPARISONFUNCTION_EQUAL );
        pRenderContext->SetStencilPassOperation( STENCILOPERATION_ZERO );
    }

    // Finally draw our blurred result onto the screen
    pRenderContext->DrawScreenSpaceQuad( m_EffectMaterial );
    // DrawScreenEffectMaterial( m_EffectMaterial, x, y, w, h ); //Uncomment me
and comment the above line if you plan to use multiple screeneffects at once.

    pRenderContext->SetStencilEnable( false );
}

void CEntGlowEffect::RenderToStencil( int idx, IMatRenderContext *pRenderContext
)
{
    if( idx < 0 || idx >= m_vGlowEnts.Count() )
        return;

    C_BaseEntity *pEnt = m_vGlowEnts[ idx ]->m_hEnt.Get();
    if( !pEnt )
    {
        // We don't exist anymore, remove us!
        delete m_vGlowEnts[ idx ];
        m_vGlowEnts.Remove( idx );
        return;
    }

    pRenderContext->SetStencilEnable( true );
    pRenderContext->SetStencilFailOperation( STENCILOPERATION_KEEP );
    pRenderContext->SetStencilZFailOperation( STENCILOPERATION_KEEP );
    pRenderContext->SetStencilPassOperation( STENCILOPERATION_REPLACE );
    pRenderContext->SetStencilCompareFunction( STENCILCOMPARISONFUNCTION_ALWAYS
);
    pRenderContext->SetStencilWriteMask( 1 );
    pRenderContext->SetStencilReferenceValue( 1 );

    pRenderContext->DepthRange( 0.0f, 0.01f );
    render->SetBlend( 0 );

    modelrender->ForcedMaterialOverride( m_WhiteMaterial );
    pEnt->DrawModel( STUDIO_RENDER );
    modelrender->ForcedMaterialOverride( NULL );

    render->SetBlend( 1 );
    pRenderContext->DepthRange( 0.0f, 1.0f );

    pRenderContext->SetStencilEnable( false );
}

void CEntGlowEffect::RenderToGlowTexture( int idx, IMatRenderContext
*pRenderContext )
{
    if( idx < 0 || idx >= m_vGlowEnts.Count() )
        return;

    C_BaseEntity *pEnt = m_vGlowEnts[ idx ]->m_hEnt.Get();
    if( !pEnt )
    {
        // We don't exist anymore, remove us!
        delete m_vGlowEnts[ idx ];
        m_vGlowEnts.Remove( idx );
        return;
    }

    pRenderContext->PushRenderTargetAndViewport( m_GlowBuff1 );

    modelrender->SuppressEngineLighting( true );
    render->SetColorModulation( m_vGlowEnts[ idx ]->m_fColor );

    modelrender->ForcedMaterialOverride( m_WhiteMaterial );
    pEnt->DrawModel( STUDIO_RENDER );
    modelrender->ForcedMaterialOverride( NULL );

    render->SetColorModulation( rWhite );
    modelrender->SuppressEngineLighting( false );

    pRenderContext->PopRenderTargetAndViewport();
}
*/
