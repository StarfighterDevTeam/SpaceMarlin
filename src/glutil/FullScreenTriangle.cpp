#include "FullScreenTriangle.h"
#include "../GPUProgramManager.h"
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

	// Setup vertex buffer layout
	SETUP_PROGRAM_VERTEX_ATTRIB(PROG_TONEMAPPING)

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
