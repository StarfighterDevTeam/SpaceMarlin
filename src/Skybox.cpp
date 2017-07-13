#include "Skybox.h"
#include "Drawer.h"
#include "Camera.h"
#include "GPUProgramManager.h"
#include "glutil/glutil.h"

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
		//imgs[i].flipVertically();
		imgs[i].flipHorizontally();
	}

	m_fullScreenTriangle.init();
	
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
		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0,
			GL_RGBA8,
			imgs[i].getSize().x, imgs[i].getSize().y,
			0,
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			(const GLvoid*)imgs[i].getPixelsPtr());
	}

	m_loaded = true;
	return true;
}

void Skybox::unload()
{
	if(!m_loaded)
		return;
	m_loaded = false;

	m_fullScreenTriangle.shut();

	if(m_skyTexId != INVALID_GL_ID)
	{
		glDeleteTextures(1, &m_skyTexId);
		m_skyTexId = INVALID_GL_ID;
	}
}

void Skybox::draw(const Camera& camera)
{
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	const GPUProgram* program = gData.gpuProgramMgr->getProgram(PROG_SKYBOX);
	program->use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_skyTexId);

	mat4 localToProjMtx = camera.getWorldToProjMtx();	// no localToWorldMtx, this is the identity matrix
	localToProjMtx[3] = vec4(0,0,0,1);
	program->sendUniform("gLocalToProjMtx", localToProjMtx);

	mat4 projToViewMtx = glm::inverse(camera.getViewToProjMtx());
	mat4 viewToWorldRotMtx = mat4(glm::transpose(mat3(camera.getWorldToViewMtx())));
	mat4 projToWorldRotMtx = viewToWorldRotMtx * projToViewMtx;
	program->sendUniform("gProjToWorldRotMtx", projToWorldRotMtx);

	program->sendUniform("texSky", 0);

	m_fullScreenTriangle.draw();
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
}

void Skybox::update()
{
}
