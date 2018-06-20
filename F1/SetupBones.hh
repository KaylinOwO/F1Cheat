#pragma once

#include "../SDK/SDK.hh"

class CBaseEntity;

class CSetupBonesEntity
{
	CBoneAccessor *m_pBoneAccessor;

	CUtlVector<matrix3x4_t> m_CachedBoneData;

	int *m_iAccumulatedBoneMask = 0;
	int *m_iPrevBoneMask        = 0;

	CIKContext *m_pIk;

	CBaseEntity *thisptr;
	CBaseEntity *baseAnimating;

private:
	inline matrix3x4_t &      GetBoneForWrite (int iBone);
	inline const matrix3x4_t &GetBone (int iBone);
	void                      ApplyBoneMatrixTransform (matrix3x4_t &transform);
	void                      BaseAnimating__BuildTransformations (const CStudioHdr *hdr, Vector *pos, Quaternion *q, const matrix3x4_t &cameraTransform, int boneMask, CBoneBitList &boneComputed);

public:
	CSetupBonesEntity (CBaseEntity *ent);

	bool SetupBones (matrix3x4_t *pBoneToWorldOut, int nMaxBones, int boneMask);

	int  numBonesSetup;
	bool shouldCreatePose;
};
