#include "Lane.h"
#include "Drawer.h"
#include "GPUProgramManager.h"

void Lane::init()
{
	assert(!m_vertices.size());
	static float gfQuadSize = 1.f;
	VtxLane vtx;
	vtx.pos = glm::vec3(-gfQuadSize,0,-gfQuadSize); m_vertices.push_back(vtx);
	vtx.pos = glm::vec3(+gfQuadSize,0,-gfQuadSize); m_vertices.push_back(vtx);
	vtx.pos = glm::vec3(+gfQuadSize,0,+gfQuadSize); m_vertices.push_back(vtx);
	vtx.pos = glm::vec3(-gfQuadSize,0,+gfQuadSize); m_vertices.push_back(vtx);

	// Send to GPU
#if 0
	{
		glGenVertexArrays(1, &m_vertexArrayId);
		glBindVertexArray(m_vertexArrayId);

		// Load into VBOs

		glGenBuffers(1, &m_vertexBufferId);
		glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);
		glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(VtxLane), &m_vertices[0].pos, GL_STATIC_DRAW);

		glGenBuffers(1, &m_uvsBufferId);
		glBindBuffer(GL_ARRAY_BUFFER, m_uvsBufferId);
		glBufferData(GL_ARRAY_BUFFER, m_uvs.size() * sizeof(glm::vec2), &m_uvs[0], GL_STATIC_DRAW);

		glGenBuffers(1, &m_normalsBufferId);
		glBindBuffer(GL_ARRAY_BUFFER, m_normalsBufferId);
		glBufferData(GL_ARRAY_BUFFER, m_normals.size() * sizeof(glm::vec3), &m_normals[0], GL_STATIC_DRAW);

		assert(	(  m_boneIndices.size() &&  m_boneWeights.size() ) ||
				( !m_boneIndices.size() && !m_boneWeights.size() )
			);
		if(m_boneIndices.size())
		{
			glGenBuffers(1, &m_boneIndicesBufferId);
			glBindBuffer(GL_ARRAY_BUFFER, m_boneIndicesBufferId);
			glBufferData(GL_ARRAY_BUFFER, m_boneIndices.size() * sizeof(glm::u8vec4), &m_boneIndices[0], GL_STATIC_DRAW);
		}

		if(m_boneWeights.size())
		{
			glGenBuffers(1, &m_boneWeightsBufferId);
			glBindBuffer(GL_ARRAY_BUFFER, m_boneWeightsBufferId);
			glBufferData(GL_ARRAY_BUFFER, m_boneWeights.size() * sizeof(glm::u8vec4), &m_boneWeights[0], GL_STATIC_DRAW);
		}

		// Generate a buffer for the indices as well
		glGenBuffers(1, &m_indexBufferId);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferId);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned short), &m_indices[0] , GL_STATIC_DRAW);
	}
#endif
}

void Lane::shut()
{
	m_vertices.clear();
}

void Lane::draw(const Camera& camera)
{
	static float gfQuadSize = 1.f;
	const glm::vec3 quadPos[] = {
		glm::vec3(-gfQuadSize,0,-gfQuadSize),
		glm::vec3(+gfQuadSize,0,-gfQuadSize),
		glm::vec3(+gfQuadSize,0,+gfQuadSize),
		glm::vec3(-gfQuadSize,0,+gfQuadSize),
	};
	gData.drawer->drawLine(camera, quadPos[0], COLOR_BLUE, quadPos[1], COLOR_BLUE);
	gData.drawer->drawLine(camera, quadPos[1], COLOR_BLUE, quadPos[2], COLOR_BLUE);
	gData.drawer->drawLine(camera, quadPos[2], COLOR_BLUE, quadPos[3], COLOR_BLUE);
	gData.drawer->drawLine(camera, quadPos[3], COLOR_BLUE, quadPos[0], COLOR_BLUE);
}

void Lane::update()
{
}
