#include "Background.h"
#include "Drawer.h"
#include "Camera.h"
#include "GPUProgramManager.h"
#include "glutil/glutil.h"
#include "SharedDefines.h"

bool Background::load()
{
	// Load images
	sf::Image imgPerlin;
	if(!imgPerlin.loadFromFile(gData.assetsPath + SDIR_SEP + "textures" SDIR_SEP "perlin.png"))
	{
		imgPerlin.flipVertically();
		return false;
	}

	assert(!m_vertices.size());
	VtxBackground vtx;
	vtx.pos = glm::vec3(-1,-1,0); m_vertices.push_back(vtx);
	vtx.pos = glm::vec3(+1,-1,0); m_vertices.push_back(vtx);
	vtx.pos = glm::vec3(+1,+1,0); m_vertices.push_back(vtx);
	vtx.pos = glm::vec3(-1,+1,0); m_vertices.push_back(vtx);

	m_indices.push_back(0);
	m_indices.push_back(1);
	m_indices.push_back(2);

	m_indices.push_back(0);
	m_indices.push_back(2);
	m_indices.push_back(3);

	// Send to GPU
	{
		glGenVertexArrays(1, &m_vertexArrayId);
		glBindVertexArray(m_vertexArrayId);

		// Load into the VBO
		glGenBuffers(1, &m_vertexBufferId);
		glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);
		glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(VtxBackground), &m_vertices[0], GL_STATIC_DRAW);

		// Generate a buffer for the indices as well
		glGenBuffers(1, &m_indexBufferId);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferId);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned short), &m_indices[0] , GL_STATIC_DRAW);

		glEnableVertexAttribArray(PROG_BACKGROUND_ATTRIB_POSITIONS);
		glVertexAttribPointer(PROG_BACKGROUND_ATTRIB_POSITIONS, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glBindVertexArray(0);
	}

	// ----------------- Load cubemap ---------------
	glGenTextures(1, &m_perlinTexId);
	glBindTexture(GL_TEXTURE_2D, m_perlinTexId);

	// Set the filter
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Create the texture
	glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RGBA8,
			imgPerlin.getSize().x, imgPerlin.getSize().y,
			0,
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			(const GLvoid*)imgPerlin.getPixelsPtr());

	m_loaded = true;
	return true;
}

void Background::unload()
{
	if(!m_loaded)
		return;
	m_loaded = false;

	glDeleteBuffers(1, &m_vertexBufferId); m_vertexBufferId = INVALID_GL_ID;
	glDeleteVertexArrays(1, &m_vertexArrayId); m_vertexArrayId = INVALID_GL_ID;

	if(m_perlinTexId != INVALID_GL_ID)
	{
		glDeleteTextures(1, &m_perlinTexId);
		m_perlinTexId = INVALID_GL_ID;
	}
}

void Background::draw(const Camera& camera)
{
	glDepthMask(GL_FALSE);
	const GPUProgram* program = gData.gpuProgramMgr->getProgram(PROG_BACKGROUND);
	program->use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_perlinTexId);

	mat4 modelViewProjMtx = camera.getViewProjMtx();
	modelViewProjMtx[3] = vec4(0,0,0,1);
	program->sendUniform("gModelViewProjMtx", modelViewProjMtx);
	program->sendUniform("texPerlin", 0);

	glBindVertexArray(m_vertexArrayId);
	// Draw the triangles !
	glDrawElements(
		GL_TRIANGLES,				 // mode
		(GLsizei)m_indices.size(),    // count
		GL_UNSIGNED_SHORT,			  // type
		(void*)0					// element array buffer offset
	);
	glDepthMask(GL_TRUE);
}

void Background::update()
{
}
