#pragma once
#ifndef _MODEL_H
#define _MODEL_H

#include "Globals.h"
#include "GPUProgramManager.h"

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

	GLuint						m_albedoTexId;

	glm::mat4					m_localToWorldMtx;
	
public:
	Model()															{m_bLoaded = false;}
	bool						loadFromFile(const char* fileName);
	bool						loadFromAssImpMesh(const aiMesh* mesh, const aiScene* scene, const char* materialsDir);
	void						unload();
	void						draw(const Camera& camera);

	void						setPosition(const glm::vec3 pos)					{m_localToWorldMtx[3].x = pos.x; m_localToWorldMtx[3].y = pos.y; m_localToWorldMtx[3].z = pos.z;}
	glm::vec3					getPosition() const									{return glm::vec3(m_localToWorldMtx[3].x, m_localToWorldMtx[3].y, m_localToWorldMtx[3].z);}
	void						setLocalToWorldMtx(const glm::mat4 modelMtx)		{m_localToWorldMtx = modelMtx;}
	const glm::mat4&			getLocalToWorldMtx() const							{return m_localToWorldMtx;}

protected:
	// Hooks for derived classes
	virtual const GPUProgram*	getProgram() const;
	virtual void				sendUniforms(const GPUProgram* program, const Camera& camera) const;
	void						move(vec3 vector);
};

#endif // _MODEL_H
