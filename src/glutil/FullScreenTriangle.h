#pragma once
#ifndef _FULLSCREENTRIANGLE_H
#define _FULLSCREENTRIANGLE_H

#include "../Globals.h"

class FullScreenTriangle
{
private:
	GLuint			m_vertexArrayId;
	GLuint			m_vertexBufferId;

public:
	void	init();
	void	shut();
	void	draw();
};

#endif // _FULLSCREENTRIANGLE_H
