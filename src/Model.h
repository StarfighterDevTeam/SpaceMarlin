#pragma once
#ifndef _MODEL_H
#define _MODEL_H

#include "SceneObject.h"
#include "GPUProgramManager.h"

struct aiScene;
struct aiMesh;
class Camera;

class ModelResource
{
protected:
	bool	m_bLoaded;
	
	std::vector<unsigned short>	m_indices;
	std::vector<VtxModel>		m_vertices;	// TODO: needs to be made generic

	GLuint						m_vertexArrayId;
	GLuint						m_indexBufferId;
	GLuint						m_vertexBufferId;
	GLuint						m_albedoTexId;

public:
	ModelResource()														{m_bLoaded = false;}
	bool						loadFromFile(const char* fileName);
	bool						loadFromAssImpMesh(const aiMesh* mesh, const aiScene* scene, const char* materialsDir);
	void						unload();

	int							getIndicesCount() const		{return (int)m_indices.size();}
	GLuint						getVertexArrayId() const	{return m_vertexArrayId;}
	GLuint						getIndexBufferId() const	{return m_indexBufferId;}
	GLuint						getVertexBufferId() const	{return m_vertexBufferId;}
	GLuint						getAlbedoTexId() const		{return m_albedoTexId;}
};

class ModelInstance : public SceneObject
{
public:
	void						setModelResourcePtr(const ModelResource* modelResourcePtr) {m_modelResourcePtr = modelResourcePtr;}
	virtual void				draw(const Camera& camera);

protected:
	// Hooks for derived classes
	virtual const GPUProgram*	getProgram() const;
	virtual void				sendUniforms(const GPUProgram* program, const Camera& camera) const;

	const ModelResource*	m_modelResourcePtr;
};

#endif // _MODEL_H
