#include "Lane.h"
#include "Drawer.h"
#include "GPUProgramManager.h"
#include "Camera.h"
#include "glutil/glutil.h"
#include "SharedDefines.h"

void Lane::init()
{
	assert(!m_vertices.size());
	const float fHalfSize = 10.f;
	VtxLane vtx;
	vtx.pos.y = 0.f;
	const int sz = 100;
	const int sx = 100;
	for(int z=0 ; z < sz ; z++)
	{
		vtx.pos.z = (z-sz/2) * fHalfSize / sz;
		for(int x=0 ; x < sx ; x++)
		{
			vtx.pos.x = (x-sx/2) * fHalfSize / sx;
			m_vertices.push_back(vtx);
		}
	}

	for(int z = 0 ; z < sz-1 ; z++)
	{
		for(int x = 0 ; x < sx-1 ; x++)
		{
			m_indices.push_back((x+0) + (z+0)*sx);
			m_indices.push_back((x+0) + (z+1)*sx);
			m_indices.push_back((x+1) + (z+1)*sx);

			m_indices.push_back((x+0) + (z+0)*sx);
			m_indices.push_back((x+1) + (z+1)*sx);
			m_indices.push_back((x+1) + (z+0)*sx);
		}
	}

	// Send to GPU
	{
		glGenVertexArrays(1, &m_vertexArrayId);
		glBindVertexArray(m_vertexArrayId);

		// Load into the VBO
		glGenBuffers(1, &m_vertexBufferId);
		glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);
		glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(VtxLane), &m_vertices[0], GL_STATIC_DRAW);

		// Generate a buffer for the indices as well
		glGenBuffers(1, &m_indexBufferId);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferId);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned short), &m_indices[0] , GL_STATIC_DRAW);

		// Vertex buffer
		glEnableVertexAttribArray(PROG_LANE_ATTRIB_POSITIONS);

		glVertexAttribPointer(PROG_LANE_ATTRIB_POSITIONS	, sizeof(VtxLane::pos)			/sizeof(GLfloat),	GL_FLOAT,			GL_FALSE,	sizeof(VtxLane), (const GLvoid*)offsetof(VtxLane, pos));

		glBindVertexArray(0);
	}
}

void Lane::shut()
{
	assert(m_indices.size());
	assert(m_vertices.size());

	m_indices.clear();
	m_vertices.clear();

	glDeleteBuffers(1, &m_vertexBufferId); m_vertexBufferId = INVALID_GL_ID;
	glDeleteBuffers(1, &m_indexBufferId); m_indexBufferId = INVALID_GL_ID;
	glDeleteVertexArrays(1, &m_vertexArrayId); m_vertexArrayId = INVALID_GL_ID;
}

void Lane::draw(const Camera& camera)
{
	static float gfQuadSize = 1.f;
	const vec3 quadPos[] = {
		vec3(-gfQuadSize,0,-gfQuadSize),
		vec3(+gfQuadSize,0,-gfQuadSize),
		vec3(+gfQuadSize,0,+gfQuadSize),
		vec3(-gfQuadSize,0,+gfQuadSize),
	};
	gData.drawer->drawLine(camera, quadPos[0], COLOR_BLUE, quadPos[1], COLOR_BLUE);
	gData.drawer->drawLine(camera, quadPos[1], COLOR_BLUE, quadPos[2], COLOR_BLUE);
	gData.drawer->drawLine(camera, quadPos[2], COLOR_BLUE, quadPos[3], COLOR_BLUE);
	gData.drawer->drawLine(camera, quadPos[3], COLOR_BLUE, quadPos[0], COLOR_BLUE);

	mat4 modelMtx;
	mat4 modelViewProjMtx = camera.getViewProjMtx() * modelMtx;
	const GPUProgram* laneProgram = gData.gpuProgramMgr->getProgram(PROG_LANE);
	laneProgram->use();
	laneProgram->sendUniform("gModelViewProjMtx", modelViewProjMtx);
	//laneProgram->sendUniform("texAlbedo", 0);
	laneProgram->sendUniform("gTime", gData.frameTime.asSeconds());

	//if(m_albedoTex != INVALID_GL_ID)
	//{
	//	glActiveTexture(GL_TEXTURE0);
	//	glBindTexture(GL_TEXTURE_2D, m_albedoTex);
	//}

	glBindVertexArray(m_vertexArrayId);
	
	glDisable(GL_CULL_FACE);

	// Draw the triangles !
	glDrawElements(
		GL_TRIANGLES,				 // mode
		(GLsizei)m_indices.size(),    // count
		GL_UNSIGNED_SHORT,			  // type
		(void*)0					// element array buffer offset
	);

	glEnable(GL_CULL_FACE);
}

void Lane::update()
{
}
