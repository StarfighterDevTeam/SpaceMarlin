#pragma once
#ifndef _SCENE_OBJECT_H
#define _SCENE_OBJECT_H

#include "Globals.h"

class SceneObject
{
protected:
	mat4				m_localToWorldMtx;
	
public:
	void				setPosition(const vec3& pos)					{m_localToWorldMtx[3].x = pos.x; m_localToWorldMtx[3].y = pos.y; m_localToWorldMtx[3].z = pos.z;}
	vec3				getPosition() const								{return vec3(m_localToWorldMtx[3].x, m_localToWorldMtx[3].y, m_localToWorldMtx[3].z);}
	void				setLocalToWorldMtx(const mat4& modelMtx)		{m_localToWorldMtx = modelMtx;}
	const mat4&			getLocalToWorldMtx() const						{return m_localToWorldMtx;}
	void				move(const vec3& vector)						{setPosition(getPosition() + vector);}
};

#endif // _SCENE_OBJECT_H
