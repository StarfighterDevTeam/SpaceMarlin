#pragma once
#ifndef _MODEL_H
#define _MODEL_H

#include "Globals.h"

struct aiScene;
struct aiMesh;

class Model
{
protected:
	bool	m_bLoaded;
	
	std::vector<unsigned short> m_indices;
	std::vector<glm::vec3> m_positions;
	std::vector<glm::vec2> m_uvs;
	std::vector<glm::vec3> m_normals;
	std::vector<glm::u8vec4> m_boneIndices;
	std::vector<glm::u8vec4> m_boneWeights;

	GLuint m_vertexArrayID;
	GLuint m_indicesBuffer;
	GLuint m_positionsBuffer;
	GLuint m_uvsBuffer;
	GLuint m_normalsBuffer;
	GLuint m_boneIndicesBuffer;
	GLuint m_boneWeightsBuffer;

	GLuint m_albedoTex;
	
public:
	Model() {m_bLoaded = false;}
	bool loadFromFile(const char* fileName);
	bool loadFromAssImpMesh(const aiMesh* mesh, const aiScene* scene, const char* materialsDir);
	void unload();
	void draw();
};

#endif // _MODEL_H
