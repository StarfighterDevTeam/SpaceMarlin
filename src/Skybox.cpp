#include "Skybox.h"
#include "Drawer.h"
#include "Camera.h"
#include "GPUProgramManager.h"
#include "glutil/glutil.h"
#include "SharedDefines.h"

bool Skybox::loadFromFiles(	const char* cubemapFilename0,
							const char* cubemapFilename1,
							const char* cubemapFilename2,
							const char* cubemapFilename3,
							const char* cubemapFilename4,
							const char* cubemapFilename5)
{
	const char* cubemapFilenames[6] = {
		cubemapFilename0,
		cubemapFilename1,
		cubemapFilename2,
		cubemapFilename3,
		cubemapFilename4,
		cubemapFilename5
	};

	sf::Image imgs[6];
	for(int i=0 ; i < 6 ; i++)
	{
		if(!imgs[i].loadFromFile(cubemapFilenames[i]))
			return false;
		imgs[i].flipVertically();
	}

	// from https://learnopengl.com/#!Advanced-OpenGL/Cubemaps
	float skyboxVertices[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};

	// Send to GPU
	{
		glGenVertexArrays(1, &m_vertexArrayId);
		glBindVertexArray(m_vertexArrayId);

		// Load into the VBO
		glGenBuffers(1, &m_vertexBufferId);
		glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);
		glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);

		glEnableVertexAttribArray(PROG_SKYBOX_ATTRIB_POSITIONS);
		glVertexAttribPointer(PROG_SKYBOX_ATTRIB_POSITIONS, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glBindVertexArray(0);
	}

	// ----------------- Load cubemap ---------------
	glGenTextures(1, &m_skyTexId);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_skyTexId);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);  

	for(int i=0 ; i < 6 ; i++)
	{
		sf::Image img;
		if(img.loadFromFile(cubemapFilenames[i]))
		{
			img.flipVertically();

			glTexImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0,
				GL_RGBA8,
				img.getSize().x, img.getSize().y,
				0,
				GL_RGBA,
				GL_UNSIGNED_BYTE,
				(const GLvoid*)img.getPixelsPtr());
		}
	}

	m_loaded = true;
	return true;
}

void Skybox::unload()
{
	if(!m_loaded)
		return;
	m_loaded = false;

	glDeleteBuffers(1, &m_vertexBufferId); m_vertexBufferId = INVALID_GL_ID;
	glDeleteVertexArrays(1, &m_vertexArrayId); m_vertexArrayId = INVALID_GL_ID;

	if(m_skyTexId != INVALID_GL_ID)
	{
		glDeleteTextures(1, &m_skyTexId);
		m_skyTexId = INVALID_GL_ID;
	}
}

void Skybox::draw(const Camera& camera)
{
	glDepthMask(GL_FALSE);
	const GPUProgram* program = gData.gpuProgramMgr->getProgram(PROG_SKYBOX);
	program->use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_skyTexId);

	program->sendUniform("gModelViewProjMtx", camera.getViewProjMtx());
	program->sendUniform("texSky", 0);

	glBindVertexArray(m_vertexArrayId);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glDepthMask(GL_TRUE);
}

void Skybox::update()
{
}
