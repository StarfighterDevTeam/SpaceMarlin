#pragma once

#ifndef _DRAWER_H
#define _DRAWER_H

#include "Globals.h"

class Camera;
class Drawer
{
private:
	GLuint	m_lineVertexArrayId;
	GLuint	m_lineVertexBufferId;

public:
	void		init();
	void		shut();

	void		drawLine(const Camera& camera, glm::vec3 pos0, glm::vec4 col0, glm::vec3 pos1, glm::vec4 col1);
};

#endif // _DRAWER_H
