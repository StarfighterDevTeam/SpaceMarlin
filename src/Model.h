#pragma once
#ifndef _MODEL_H
#define _MODEL_H

#include "SceneObject.h"
#include "GPUProgramManager.h"

struct aiScene;
struct aiMesh;
class Camera;

class Model : public SceneObject
{
protected:
	bool	m_bLoaded;
	
	std::vector<unsigned short>	m_indices;
	std::vector<VtxModel>		m_vertices;

	GLuint						m_vertexArrayId;
	GLuint						m_indexBufferId;
	GLuint						m_vertexBufferId;

	GLuint						m_albedoTexId;

public:
	Model()															{m_bLoaded = false;}
	bool						loadFromFile(const char* fileName);
	bool						loadFromAssImpMesh(const aiMesh* mesh, const aiScene* scene, const char* materialsDir);
	void						unload();
	void						draw(const Camera& camera);

protected:
	// Hooks for derived classes
	virtual const GPUProgram*	getProgram() const;
	virtual void				sendUniforms(const GPUProgram* program, const Camera& camera) const;
};

#endif // _MODEL_H
