#include "Drawer.h"
#include "GPUProgramManager.h"
#include "Camera.h"
#include "glutil/glutil.h"

void Drawer::init()
{
	// Init line
	{
		glGenVertexArrays(1, &m_lineVertexArrayId);
		glBindVertexArray(m_lineVertexArrayId);

		glGenBuffers(1, &m_lineVertexBufferId);
		glBindBuffer(GL_ARRAY_BUFFER, m_lineVertexBufferId);

		const GLsizeiptr totalSize = 2*sizeof(VtxSimple);
		glBufferData(GL_ARRAY_BUFFER, totalSize, NULL, GL_STREAM_DRAW);

		// Setup vertex buffer layout
		SETUP_PROGRAM_VERTEX_ATTRIB(PROG_SIMPLE)

		glBindVertexArray(0);
	}

	// Init 2D quad
	{
		glGenVertexArrays(1, &m_2DQuadVertexArrayId);
		glBindVertexArray(m_2DQuadVertexArrayId);

		glGenBuffers(1, &m_2DQuadVertexBufferId);
		glBindBuffer(GL_ARRAY_BUFFER, m_2DQuadVertexBufferId);

		const GLsizeiptr totalSize = 6*sizeof(VtxTexture2D);
		glBufferData(GL_ARRAY_BUFFER, totalSize, NULL, GL_STREAM_DRAW);

		// Setup vertex buffer layout
		SETUP_PROGRAM_VERTEX_ATTRIB(PROG_TEXTURE_2D)

		glBindVertexArray(0);
	}
}

void Drawer::shut()
{
	// Shut line
	glDeleteBuffers(1, &m_lineVertexBufferId);
	glDeleteVertexArrays(1, &m_lineVertexArrayId);

	// Shut 2D quad
	glDeleteBuffers(1, &m_2DQuadVertexBufferId);
	glDeleteVertexArrays(1, &m_2DQuadVertexArrayId);
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

void Drawer::drawCross(const Camera& camera, glm::vec3 pos, glm::vec4 col, float size)
{
	const GPUProgram* program = gData.gpuProgramMgr->getProgram(PROG_SIMPLE);
	program->use();
	program->sendUniform("gLocalToProjMtx", camera.getWorldToProjMtx());	// no localToWorld as we're giving coordinates in world space

	const mat4& viewToWorldRotMtx = camera.getViewToWorldRotMtx();
	const vec3 worldSpaceViewX = vec3(viewToWorldRotMtx[0]);
	const vec3 worldSpaceViewY = vec3(viewToWorldRotMtx[1]);

	glBindVertexArray(m_lineVertexArrayId);
	
	glBindBuffer(GL_ARRAY_BUFFER, m_lineVertexBufferId);

	const float halfSize = 0.5f * size;
	VtxSimple vertices[] = {
		{pos - worldSpaceViewX * halfSize, col},
		{pos + worldSpaceViewX * halfSize, col},
		{pos - worldSpaceViewY * halfSize, col},
		{pos + worldSpaceViewY * halfSize, col},
	};
	const size_t nbVertices = _countof(vertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), (const GLvoid*)vertices, GL_STREAM_DRAW);
	
	// - draw
	glDrawArrays(GL_LINES, 0, nbVertices);

	glBindVertexArray(0);
}


void Drawer::draw2DTexturedQuad(GLuint texId, glm::vec2 pos, glm::vec2 size)
{
	glutil::Disable<GL_DEPTH_TEST>	depthTestState;
	glutil::Disable<GL_CULL_FACE>	cullFaceState;

	const GPUProgram* program = gData.gpuProgramMgr->getProgram(PROG_TEXTURE_2D);
	program->use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texId);
	program->sendUniform("tex", 0);
	
	glBindVertexArray(m_2DQuadVertexArrayId);
	
	glBindBuffer(GL_ARRAY_BUFFER, m_2DQuadVertexBufferId);

	GLint vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);
	const vec2 vpSize(vp[2], vp[3]);

	const vec2 s = (pos/vpSize) * 2.f - 1.f;
	const vec2 e = ((pos+size)/vpSize) * 2.f - 1.f;
	VtxTexture2D vertices[] = {
		{vec2(s.x, s.y), vec2(0,0)},
		{vec2(e.x, s.y), vec2(1,0)},
		{vec2(e.x, e.y), vec2(1,1)},

		{vec2(s.x, s.y), vec2(0,0)},
		{vec2(e.x, e.y), vec2(1,1)},
		{vec2(s.x, e.y), vec2(0,1)},
	};
	
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), (const GLvoid*)vertices, GL_STREAM_DRAW);
	
	glDrawArrays(GL_TRIANGLES, 0, _countof(vertices));

	glBindVertexArray(0);
}
