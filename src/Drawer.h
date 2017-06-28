#pragma once

#ifndef _DRAWER_H
#define _DRAWER_H

#include "Globals.h"

class Drawer
{
private:
	GLuint	m_lineVertexArrayID;
	GLuint	m_lineBufferID;

public:
	void		init();
	void		shut();

	void		drawLine(const glm::mat4& modelViewProjMtx, glm::vec3 pos0, glm::vec4 col0, glm::vec3 pos1, glm::vec4 col1);
};

#endif // _DRAWER_H
