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

	GLuint	m_2DQuadVertexArrayId;
	GLuint	m_2DQuadVertexBufferId;

public:
	void		init();
	void		shut();

	void		drawLine(const Camera& camera, glm::vec3 pos0, glm::vec4 col0, glm::vec3 pos1, glm::vec4 col1);
	void		drawCross(const Camera& camera, glm::vec3 pos, glm::vec4 col, float size);
	void		draw2DTexturedQuad(GLuint texId, glm::vec2 pos, glm::vec2 size);
};

#endif // _DRAWER_H
