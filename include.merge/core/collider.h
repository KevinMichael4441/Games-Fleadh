#ifndef COLLIDER_H
#define COLLIDER_H

#include "cute_c2.h"
#include "spine/spine_manager.h"

//=================================================================
// C linkage (C++ compiler flag)
//=================================================================
#ifdef __cplusplus
extern "C"
{
#endif

	//=================================================================
	// Get c2AABB from Spine Entity
	//=================================================================
	static inline c2AABB GetSpineEntityBounds_c2(SpineEntity *entity)
	{
		Rectangle bounds = GetSpineBounds(entity);
		c2AABB entity_aabb;
		entity_aabb.min = c2V(bounds.x, bounds.y);
		entity_aabb.max = c2V(bounds.x + bounds.width, bounds.y + bounds.height);
		return entity_aabb;
	}

	//=================================================================
	// Check collision using cute_c2 SpineEntity *lhs, SpineEntity *rhs
	//=================================================================
	static inline bool CheckSpineEntityCollision_c2(SpineEntity *lhs, SpineEntity *rhs)
	{
		c2AABB lhs_c2aabb = GetSpineEntityBounds_c2(lhs);
		c2AABB rhs_c2aabb = GetSpineEntityBounds_c2(rhs);
		return c2AABBtoAABB(lhs_c2aabb, rhs_c2aabb);
	}

	//=================================================================
	// Check collision using cute_c2 c2Circle lhs SpineEntity *rhs
	//=================================================================
	static inline bool CheckCircleSpineEntityCollision_c2(c2Circle lhs, SpineEntity *rhs)
	{
		c2AABB rhs_c2aabb = GetSpineEntityBounds_c2(rhs);
		return c2CircletoAABB(lhs, rhs_c2aabb);
	}

#ifdef __cplusplus
}
#endif

#endif // COLLIDER_H