#pragma once
#ifndef _MODEL_H
#define _MODEL_H

#include "Globals.h"
#include "SharedDefines.h"

struct aiScene;
struct aiMesh;
class Camera;

class Model
{
protected:
	bool	m_bLoaded;
	
	std::vector<unsigned short>	m_indices;
	std::vector<VtxModel>		m_vertices;

	GLuint						m_vertexArrayId;
	GLuint						m_indexBufferId;
	GLuint						m_vertexBufferId;

	GLuint						m_albedoTex;

	glm::mat4					m_modelMtx;
	
public:
	Model()															{m_bLoaded = false;}
	bool				loadFromFile(const char* fileName);
	bool				loadFromAssImpMesh(const aiMesh* mesh, const aiScene* scene, const char* materialsDir);
	void				unload();
	void				draw(const Camera& camera);

	void				setPosition(const glm::vec3 pos)			{m_modelMtx[3].x = pos.x; m_modelMtx[3].y = pos.y; m_modelMtx[3].z = pos.z;}
	glm::vec3			getPosition() const							{return glm::vec3(m_modelMtx[3].x, m_modelMtx[3].y, m_modelMtx[3].z);}
	void				setModelMtx(const glm::mat4 modelMtx)		{m_modelMtx = modelMtx;}
	const glm::mat4&	getModelMtx() const							{return m_modelMtx;}
};

#endif // _MODEL_H
