#include "stdafx.hh"

#define CLIENT_DLL

#include "SetupBones.hh"

#include "F1_Vprof.hh"

#include <tier0/memdbgon.h>

CSetupBonesEntity::CSetupBonesEntity(CBaseEntity *ent)
#if defined _MSC_VER
    : thisptr(ent), baseAnimating(ent + 4)
{

	// TODO: SIG ALL THIS!

	//*(_DWORD *)(this + 1460) = v25;               // m_BoneAccessor.SetWritableBones( m_BoneAccessor.GetReadableBones() | boneMask )
	//*(_DWORD *)(this + 1456) = v25;               // m_BoneAccessor.SetReadableBones( m_BoneAccessor.GetWritableBones() );

	// the bone array is before that and is the second class member
	// first class member is a pointer
	// so the class starts at readable bones - 0x8

	m_pBoneAccessor = (CBoneAccessor *)(baseAnimating + 1456 - 0x8);

	// *(_DWORD *)(this + 1440) = *(_DWORD *)(this + 1444);// m_iPrevBoneMask = m_iAccumulatedBoneMask;

	m_iAccumulatedBoneMask = (int *)(baseAnimating + 1444);
	m_iPrevBoneMask        = (int *)(baseAnimating + 1440);

	// memmove_0(a4, *(const void **)(this + 2116), 48 * v38);// memcpy(pBoneToWorldOut, m_CachedBoneData.Base(), sizeof(matrix3x4_t) * m_CachedBoneData.Count());

	m_CachedBoneData = *(CUtlVector<matrix3x4_t> *)(baseAnimating + 2116);
}
#else
    : thisptr(ent), baseAnimating(ent)
{

	// TODO: SIG ALL THIS!

	//*(_DWORD *)(a2 + 1448) = v21; // m_BoneAccessor.SetWritableBones( m_BoneAccessor.GetReadableBones() | boneMask )
	//*(_DWORD *)(a2 + 1444) = v21; // m_BoneAccessor.SetReadableBones( m_BoneAccessor.GetWritableBones() );

	// the bone array is before that and is the second class member
	// first class member is a pointer
	// so the class starts at readable bones - 0x8

	m_pBoneAccessor = (CBoneAccessor *)(baseAnimating + 1444 - 0x8);

	// m_iPrevBoneMask = m_iAccumulatedBoneMask;

	// v18 = *(_DWORD *)( this + 1432 ); // accumulated mask
	//*(_DWORD *)( this + 1432 ) = 0;
	// v10 = *(_BYTE *)( this + 2169 ) == 0;
	//*(_DWORD *)( this + 1428 ) = v18; // prev bone mask

	m_iAccumulatedBoneMask = (int *)(baseAnimating + 1432);
	m_iPrevBoneMask        = (int *)(baseAnimating + 1428);

	// memcpy( dest, *(const void **)( v7 + 2104 ), 48 * v22 );// memcpy(pBoneToWorldOut, m_CachedBoneData.Base(), sizeof(matrix3x4_t) * m_CachedBoneData.Count());

	m_CachedBoneData = *(CUtlVector<matrix3x4_t> *)(baseAnimating + 2104);
}
#endif

inline matrix3x4_t &CSetupBonesEntity::GetBoneForWrite(int iBone)
{
	return m_pBoneAccessor->GetBoneForWrite(iBone);
}

inline const matrix3x4_t &CSetupBonesEntity::GetBone(int iBone)
{
	return m_pBoneAccessor->GetBone(iBone);
}

void CSetupBonesEntity::ApplyBoneMatrixTransform(matrix3x4_t &transform)
{
	NETVAR(m_flModelWidthScale, float, "DT_BaseAnimating", "m_flModelWidthScale");
	float scale = m_flModelWidthScale.GetValue(thisptr);
	if (scale != 1.0f) {
		VectorScale(transform[0], scale, transform[0]);
		VectorScale(transform[1], scale, transform[1]);
	}
}

ConVar f1_usetfbuildtransformations("f1_usetfbuildtransformations", "1", FCVAR_NONE, "Use the tf2 transformations instead of mine");

void CSetupBonesEntity::BaseAnimating__BuildTransformations(const CStudioHdr *hdr, Vector *pos, Quaternion *q, const matrix3x4_t &cameraTransform, int boneMask, CBoneBitList &boneComputed)
{
	F1_VPROF_FUNCTION();

	if (f1_usetfbuildtransformations.GetBool() == true) {
#ifdef _MSC_VER
		// when this is called from the real setupbones
		//(*(void(__thiscall **)(int, int, char *, char *, char *, int))(*(_DWORD *)thisptr + 648))(
		//	thisptr,
		//	hdr,
		//	&pos,
		//	&m_BoneAccessor,
		//	&parentTransform,
		//	bonesMaskNeedRecalc);

		// build transformations is at index 162
		// we will just call this as it handles all tf2 custom transforms
		typedef void(__thiscall * BuildTransformationsFn)(PVOID, const CStudioHdr *, Vector *, Quaternion *, const matrix3x4_t &, int, CBoneBitList &);
		return getvfunc<BuildTransformationsFn>(thisptr, 162)(thisptr, hdr, pos, q, cameraTransform, boneMask, boneComputed);

#else
		// when this is called from the real setupbones
		//	( *( void( __cdecl ** )( int, int, char *, char *, char *, int, int * ) )( *(_DWORD *)v7
		//		+ 896 ) )(
		//			v7,
		//			v68,
		//			&v87,
		//			&v88,
		//			&v86,
		//			v66,
		//			&v75 );
		//}
		// index 224

		typedef void(__thiscall * BuildTransformationsFn)(PVOID, CStudioHdr *, Vector *, Quaternion *, const matrix3x4_t &, int, CBoneBitList &);
		return getvfunc<BuildTransformationsFn>(thisptr, 224)(thisptr, hdr, pos, q, cameraTransform, boneMask, boneComputed);
#endif
	} else {

		if (!hdr)
			return;

		matrix3x4_t bonematrix;
		bool        boneSimulated[MAXSTUDIOBONES];

		// no bones have been simulated
		memset(boneSimulated, 0, sizeof(boneSimulated));
		mstudiobone_t *pbones = hdr->pBone(0);

		for (int i = 0; i < hdr->numbones(); i++) {
			// Only update bones reference by the bone mask.
			if (!(hdr->boneFlags(i) & boneMask)) {
				continue;
			}

			// animate all non-simulated bones
			if (boneSimulated[i] /*|| CalcProceduralBone(hdr, i, *m_pBoneAccessor)*/) {
				continue;
			}
			// skip bones that the IK has already setup
			else if (boneComputed.IsBoneMarked(i)) {
				// dummy operation, just used to verify in debug that this should have happened
				GetBoneForWrite(i);
			} else {
				QuaternionMatrix(q[i], pos[i], bonematrix);

				if ((hdr->boneFlags(i) & BONE_ALWAYS_PROCEDURAL) && (hdr->pBone(i)->proctype & STUDIO_PROC_JIGGLE)) {
					//
					// Physics-based "jiggle" bone
					// Bone is assumed to be along the Z axis
					// Pitch around X, yaw around Y
					//

					// compute desired bone orientation
					matrix3x4_t goalMX;

					if (pbones[i].parent == -1) {
						ConcatTransforms(cameraTransform, bonematrix, goalMX);
					} else {
						// If the parent bone has been scaled (like with BuildBigHeadTransformations)
						// scale it back down so the jiggly bones show up non-scaled in the correct location.
						matrix3x4_t parentMX = GetBone(pbones[i].parent);

						float fScale = Square(parentMX[0][0]) + Square(parentMX[1][0]) + Square(parentMX[2][0]);
						if (fScale > Square(1.0001f)) {
							fScale = 1.0f / sqrtf(fScale);
							MatrixScaleBy(fScale, parentMX);
						}

						ConcatTransforms(parentMX, bonematrix, goalMX);
					}
				} else if (hdr->boneParent(i) == -1) {
					ConcatTransforms(cameraTransform, bonematrix, GetBoneForWrite(i));
				} else {
					ConcatTransforms(GetBone(hdr->boneParent(i)), bonematrix, GetBoneForWrite(i));
				}
			}

			if (hdr->boneParent(i) == -1) {
				// Apply client-side effects to the transformation matrix
				ApplyBoneMatrixTransform(GetBoneForWrite(i));
			}
		}
	}
}

// ill just leave this here...
// https://hastebin.com/avecaxoziq.php

ConVar f1_setupbones_use_ibonesetup("f1_setupbones_use_ibonesetup", "0", 0, "");

bool CSetupBonesEntity::SetupBones(matrix3x4_t *pBoneToWorldOut, int nMaxBones, int boneMask)
{
	F1_VPROF_FUNCTION();

	// Keep track of everthing asked for over the entire frame
	(*m_iAccumulatedBoneMask) |= boneMask;

	// Have we cached off all bones meeting the flag set?
	if ((m_pBoneAccessor->GetReadableBones() & boneMask) != boneMask) {
		MDLCACHE_CRITICAL_SECTION();

		const CStudioHdr *hdr = thisptr->GetStudioHdr();
		if (!hdr)
			return false;

		// Setup our transform based on render angles and origin.
		matrix3x4_t parentTransform;

		// v22 = (*(int(__thiscall **)(int))(*(_DWORD *)this + 4))(this);// GetRenderOrigin()
		// v23 = (*(int(__thiscall **)(int))(*(_DWORD *)this + 8))(this);// GetRenderAngles()

		// Vector backupOrigin = thisptr->GetAbsOrigin();
		// thisptr->GetAbsOrigin() = thisptr->GetRenderOrigin();

		AngleMatrix(thisptr->GetEyeAngles(), thisptr->GetAbsOrigin(), parentTransform);

		// Load the boneMask with the total of what was asked for last frame.
		boneMask |= (*m_iPrevBoneMask);

		// Allow access to the bones we're setting up so we don't get asserts in here.
		int oldReadableBones = m_pBoneAccessor->GetReadableBones();
		m_pBoneAccessor->SetWritableBones(m_pBoneAccessor->GetReadableBones() | boneMask);
		m_pBoneAccessor->SetReadableBones(m_pBoneAccessor->GetWritableBones());

		if (hdr->flags() & STUDIOHDR_FLAGS_STATIC_PROP) {
			MatrixCopy(parentTransform, GetBoneForWrite(0));
		} else {
			Vector     pos[MAXSTUDIOBONES];
			Quaternion q[MAXSTUDIOBONES];

			int bonesMaskNeedRecalc = boneMask | oldReadableBones; // Hack to always recalc bones, to fix the arm jitter in the new CS player anims until Ken makes the real fix

			CBoneBitList boneComputed;

			// call standardblendingrules
			// TODO: linxu
			// look for "%8.4f : %30s : %5.3f : %4.2f\n"
			//typedef void(__thiscall * StandardBlendingRulesFn)(PVOID, const CStudioHdr *, Vector[], Quaternion[], float, int);
			//getvfunc<StandardBlendingRulesFn>(baseAnimating, 176)(baseAnimating, hdr, pos, q, gInts->Globals->curtime, bonesMaskNeedRecalc);

			if (f1_setupbones_use_ibonesetup.GetBool() == true && shouldCreatePose) {
				//const CStudioHdr *hdr = thisptr->GetStudioHdr ();

				IBoneSetup boneSetup(hdr, boneMask, ((DT_BaseAnimating *)baseAnimating)->flPoseParameter());

				// use curtime to setup our bones
				boneSetup.InitPose(pos, q);

				int maxLayers = thisptr->GetNumAnimOverlays();
				for (int i = 0; i < maxLayers; i++) {
					auto *layer = thisptr->GetAnimOverlay(i);
					if (layer->m_flWeight != 0.0f) {
						// if the weight is 0 then there is no reason to accumulate the pose as it will have no effect
						boneSetup.AccumulatePose(pos, q, layer->m_nSequence, layer->m_flCycle, layer->m_flWeight, gInts->Globals->curtime, nullptr);
					}
				}
			}

			BaseAnimating__BuildTransformations(hdr, pos, q, parentTransform, bonesMaskNeedRecalc, boneComputed);

			// RemoveFlag(EFL_SETTING_UP_BONES);
			// ControlMouth(hdr);
		}

		if (!(oldReadableBones & BONE_USED_BY_ATTACHMENT) && (boneMask & BONE_USED_BY_ATTACHMENT)) {
			typedef bool(__thiscall * SetupBones_AttachmentHelperFn)(const CBaseEntity *, const CStudioHdr *);
#ifdef _MSC_VER
			static DWORD dwSetupBonesAttachmentHelper = gSignatures.GetClientSignature("55 8B EC 83 EC 4C 53 8B 5D 08");
#else
			// just string search for attachment helper
			static DWORD dwSetupBonesAttachmentHelper = gSignatures.GetClientSignature("55 89 E5 57 56 53 83 EC 6C 8B 5D 0C 8B 7D 08 85 DB");
#endif
			if (!((SetupBones_AttachmentHelperFn)dwSetupBonesAttachmentHelper)(baseAnimating, hdr)) {
				Log::Console("SetupBones: SetupBones_AttachmentHelper failed");
				return false;
			}
		}

		// reset origin
		// thisptr->GetAbsOrigin() = backupOrigin;
	}

	// Do they want to get at the bone transforms? If it's just making sure an aiment has
	// its bones setup, it doesn't need the transforms yet.
	if (pBoneToWorldOut) {
		if (nMaxBones >= m_CachedBoneData.Count()) {
			numBonesSetup = m_CachedBoneData.Count();
			memcpy(pBoneToWorldOut, m_CachedBoneData.Base(), sizeof(matrix3x4_t) * m_CachedBoneData.Count());
		} else {
			// ExecuteNTimes(25, Warning("SetupBones: invalid bone array size (%d - needs %d)\n", nMaxBones, m_CachedBoneData.Count()));
			Log::Console("SetupBones: invalid bone array size (%d - needs %d)\n", nMaxBones, m_CachedBoneData.Count());
			return false;
		}
	}

	return true;
}
