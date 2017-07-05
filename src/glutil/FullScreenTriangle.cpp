#pragma once
#include "FullScreenTriangle.h"
#include "../SharedDefines.h"
#include "glutil.h"

void FullScreenTriangle::init()
{
	// Send to GPU

	glGenVertexArrays(1, &m_vertexArrayId);
	glBindVertexArray(m_vertexArrayId);

	// Load into the VBO
	glGenBuffers(1, &m_vertexBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);

	// === Ordering ===
	//  [2]
	//   |    ` __
	//   |         `
	//  [0]----------[1]
	//
	// === Pos: ===
	//(-1,+2)
	//   |    ` __
	//   |         `
	//(-1,-1)-------(+2,-1)
	//
	// === UV: ===
	//(0,2)
	//   |    ` __
	//   |         `
	//(0,0)----------(2,0)

	static const VtxFullScreenTriangle vertices[] = {
		{ vec2(-1,-1), vec2(0, 0) },
		{ vec2(+3,-1), vec2(2, 0) },
		{ vec2(-1,+3), vec2(0, 2) },
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), (const GLvoid*)vertices, GL_STATIC_DRAW);

	// Vertex buffer
	glEnableVertexAttribArray(PROG_FULLSCREENTRIANGLE_ATTRIB_POSITIONS);
	glEnableVertexAttribArray(PROG_FULLSCREENTRIANGLE_ATTRIB_UVS);

	glVertexAttribPointer(PROG_FULLSCREENTRIANGLE_ATTRIB_POSITIONS	, sizeof(VtxFullScreenTriangle::pos)/sizeof(GLfloat),	GL_FLOAT,	GL_FALSE,	sizeof(VtxFullScreenTriangle), (const GLvoid*)offsetof(VtxFullScreenTriangle, pos));
	glVertexAttribPointer(PROG_FULLSCREENTRIANGLE_ATTRIB_UVS		, sizeof(VtxFullScreenTriangle::uv)	/sizeof(GLfloat),	GL_FLOAT,	GL_FALSE,	sizeof(VtxFullScreenTriangle), (const GLvoid*)offsetof(VtxFullScreenTriangle, uv));

	glBindVertexArray(0);
}

void FullScreenTriangle::shut()
{
	glDeleteVertexArrays(1, &m_vertexArrayId);	m_vertexArrayId = INVALID_GL_ID;
	glDeleteBuffers(1, &m_vertexBufferId);		m_vertexBufferId = INVALID_GL_ID;
}

void FullScreenTriangle::draw()
{
	glBindVertexArray(m_vertexArrayId);
	glDrawArrays(GL_TRIANGLES, 0, 3);
}
