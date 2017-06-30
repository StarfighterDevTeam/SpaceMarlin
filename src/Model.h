#pragma once
#ifndef _MODEL_H
#define _MODEL_H

#include "Globals.h"

struct aiScene;
struct aiMesh;
class Camera;

class Model
{
protected:
	bool	m_bLoaded;
	
	std::vector<unsigned short>	m_indices;
	std::vector<glm::vec3>		m_positions;
	std::vector<glm::vec2>		m_uvs;
	std::vector<glm::vec3>		m_normals;
	std::vector<glm::u8vec4>	m_boneIndices;
	std::vector<glm::u8vec4>	m_boneWeights;

	GLuint						m_vertexArrayID;
	GLuint						m_indicesBuffer;
	GLuint						m_positionsBuffer;
	GLuint						m_uvsBuffer;
	GLuint						m_normalsBuffer;
	GLuint						m_boneIndicesBuffer;
	GLuint						m_boneWeightsBuffer;

	GLuint						m_albedoTex;

	glm::mat4					m_modelMtx;
	
public:
	Model()														{m_bLoaded = false;}
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
