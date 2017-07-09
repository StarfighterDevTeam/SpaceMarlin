#include "Drawer.h"
#include "GPUProgramManager.h"
#include "Camera.h"

void Drawer::init()
{
	// Init line
	{
		glGenVertexArrays(1, &m_lineVertexArrayId);
		glBindVertexArray(m_lineVertexArrayId);

		glGenBuffers(1, &m_lineVertexBufferId);
		glBindBuffer(GL_ARRAY_BUFFER, m_lineVertexBufferId);

		const GLsizeiptr total_size = 2*sizeof(VtxSimple);	// 2 vertices, each one having (x, y, z) and (r, g, b, a).
		glBufferData(GL_ARRAY_BUFFER, total_size, NULL, GL_STREAM_DRAW);

		// Setup vertex buffer layout
		HANDLE_PROG_SIMPLE(NO_ACTION, NO_ACTION, HANDLE_ATTRIBUTE_SETUP_VERTEX_ATTRIB)

		glBindVertexArray(0);
	}
}

void Drawer::shut()
{
	// Shut line
	glDeleteBuffers(1, &m_lineVertexBufferId);
	glDeleteVertexArrays(1, &m_lineVertexArrayId);
}

void Drawer::drawLine(const Camera& camera, glm::vec3 pos0, glm::vec4 col0, glm::vec3 pos1, glm::vec4 col1)
{
	const GPUProgram* program = gData.gpuProgramMgr->getProgram(PROG_SIMPLE);
	program->use();
	program->sendUniform("gLocalToProjMtx", camera.getWorldToProjMtx());	// no localToWorld as we're giving coordinates in world space
	
	glBindVertexArray(m_lineVertexArrayId);
	
	glBindBuffer(GL_ARRAY_BUFFER, m_lineVertexBufferId);

	VtxSimple vertices[] = {
		{pos0, col0},
		{pos1, col1},
	};
	const size_t nbVertices = _countof(vertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), (const GLvoid*)vertices, GL_STREAM_DRAW);
	
	// - draw
	glDrawArrays(GL_LINES, 0, nbVertices);

	glBindVertexArray(0);
}
