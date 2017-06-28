#include "Drawer.h"
#include "GPUProgramManager.h"
#include "SharedDefines.h"

void Drawer::init()
{
	// Init line
	{
		glGenVertexArrays(1, &m_lineVertexArrayID);
		glBindVertexArray(m_lineVertexArrayID);

		glGenBuffers(1, &m_lineBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, m_lineBufferID);

		const GLsizeiptr total_size = 2*(sizeof(glm::vec3) + sizeof(glm::vec4));	// 2 vertices, each one having (x, y, z) and (r, g, b, a).
		glBufferData(GL_ARRAY_BUFFER, total_size, NULL, GL_STREAM_DRAW);
	}
}

void Drawer::shut()
{
	// Shut line
	glDeleteBuffers(1, &m_lineBufferID);
	glDeleteVertexArrays(1, &m_lineVertexArrayID);
}

void Drawer::drawLine(const glm::mat4& modelViewProjMtx, glm::vec3 pos0, glm::vec4 col0, glm::vec3 pos1, glm::vec4 col1)
{
	const GPUProgram* program = gData.gpuProgramMgr->getProgram(PROG_SIMPLE);
	program->use();
	program->sendUniform("gModelViewProjMtx", modelViewProjMtx);
	
	glBindVertexArray(m_lineVertexArrayID);
	glBindBuffer(GL_ARRAY_BUFFER, m_lineBufferID);
	
	VtxSimple vertices[] = {
		{pos0, col0},
		{pos1, col1},
	};
	const size_t nbVertices = _countof(vertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), (const GLvoid*)vertices, GL_STREAM_DRAW);
	
	glEnableVertexAttribArray(PROG_SIMPLE_ATTRIB_POSITIONS);
	glEnableVertexAttribArray(PROG_SIMPLE_ATTRIB_COLORS);

	GLintptr offset = 0;
	glVertexAttribPointer(PROG_SIMPLE_ATTRIB_POSITIONS,	3, GL_FLOAT, GL_FALSE, sizeof(VtxSimple), (const GLvoid*)offset);	offset += sizeof(glm::vec3);
	glVertexAttribPointer(PROG_SIMPLE_ATTRIB_COLORS,	4, GL_FLOAT, GL_FALSE, sizeof(VtxSimple), (const GLvoid*)offset);	offset += sizeof(glm::vec4);

	// - draw
	glDrawArrays(GL_LINES, 0, nbVertices);

	glDisableVertexAttribArray(PROG_SIMPLE_ATTRIB_POSITIONS);
	glDisableVertexAttribArray(PROG_SIMPLE_ATTRIB_COLORS);
}
