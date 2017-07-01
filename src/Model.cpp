#include "Model.h"

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags
#include "Camera.h"
#include "GPUProgramManager.h"

#include "glutil/glutil.h"

#include "SharedDefines.h"

bool Model::loadFromFile(const char * path)
{
	assert(!m_bLoaded);

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, 0 | aiProcess_Triangulate /*aiProcess_JoinIdenticalVertices | aiProcess_SortByPType*/);
	if(!scene)
	{
		logError(importer.GetErrorString());
		return false;
	}
	const aiMesh* mesh = scene->mMeshes[0]; // In this simple example code we always use the 1rst mesh (in OBJ files there is often only one anyway)

	// Extract directory, including separator
	char materialsDir[1024] = "";
	for(int i=(int)strlen(path)-1 ; i >= 0 ; i--)
	{
		if(path[i] == '/' || path[i] == '\\')
		{
			strncpy_s(materialsDir, path, i+1);
			break;
		}
	}

	return loadFromAssImpMesh(mesh, scene, materialsDir);
}

bool Model::loadFromAssImpMesh(const aiMesh* mesh, const aiScene* scene, const char* materialsDir)
{
	assert(!m_bLoaded);

	// Fill vertices
	m_vertices.resize(mesh->mNumVertices);
	for(unsigned int i=0; i<mesh->mNumVertices; i++)
	{
		aiVector3D pos = mesh->mVertices[i];
		m_vertices[i].pos = glm::vec3(pos.x, pos.y, pos.z);

		aiVector3D UVW = mesh->mTextureCoords[0][i]; // Assume only 1 set of UV coords; AssImp supports 8 UV sets.
		m_vertices[i].uv = glm::vec2(UVW.x, UVW.y);

		aiVector3D n = mesh->mNormals[i];
		m_vertices[i].normal = glm::vec3(n.x, n.y, n.z);

		m_vertices[i].boneIndices = glm::u8vec4(0, 0, 0, 0);
		m_vertices[i].boneWeights = glm::u8vec4(0, 0, 0, 0);
	}

	// Fill face indices
	m_indices.reserve(3*mesh->mNumFaces);
	for (unsigned int i=0; i<mesh->mNumFaces; i++)
	{
		// Assume the model has only triangles.
		m_indices.push_back(mesh->mFaces[i].mIndices[0]);
		m_indices.push_back(mesh->mFaces[i].mIndices[1]);
		m_indices.push_back(mesh->mFaces[i].mIndices[2]);
	}

	// Fill vertex skinning weights
	if(mesh->HasBones())
	{
		for(unsigned int boneId = 0; boneId < mesh->mNumBones; boneId++)
		{
			const aiBone* bone = mesh->mBones[boneId];
			for( unsigned int weightId = 0; weightId < bone->mNumWeights; weightId++)
			{
				const aiVertexWeight& boneWeight = bone->mWeights[weightId];

				glm::u8vec4& vertexBoneIndices = m_vertices[boneWeight.mVertexId].boneIndices;
				glm::u8vec4& vertexBoneWeights = m_vertices[boneWeight.mVertexId].boneWeights;

				bool bFoundEmptySlot = false;
				for(int i=0 ; i < 4 ; i++)
				{
					if(vertexBoneIndices[i] == 0)
					{
						vertexBoneIndices[i] = boneId;
						vertexBoneWeights[i] = (unsigned char)(boneWeight.mWeight * 255.0f);
						bFoundEmptySlot = true;
						break;
					}
				}
				assert(bFoundEmptySlot && "Vertex with more than 4 weights!");
			}
		}
	}
	
	// The "scene" pointer will be deleted automatically by "importer"

	// Send to GPU
	{
		glGenVertexArrays(1, &m_vertexArrayId);
		glBindVertexArray(m_vertexArrayId);

		// Load into the VBO
		glGenBuffers(1, &m_vertexBufferId);
		glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);
		glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(VtxModel), &m_vertices[0], GL_STATIC_DRAW);

		// Generate a buffer for the indices as well
		glGenBuffers(1, &m_indexBufferId);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferId);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned short), &m_indices[0] , GL_STATIC_DRAW);
	}

	// ----------------- Handle material ---------------
	m_albedoTex = INVALID_GL_ID;

	if(mesh->mMaterialIndex >= 0 && mesh->mMaterialIndex < scene->mNumMaterials)
	{
		const aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
		aiString path;
		if(aiReturn_SUCCESS == mat->GetTexture(aiTextureType_DIFFUSE, 0, &path))
		{
			sf::Image img;
			if(img.loadFromFile(std::string(materialsDir) + path.C_Str()))
			{
				img.flipVertically();

				glGenTextures(1, &m_albedoTex);
				glBindTexture(GL_TEXTURE_2D, m_albedoTex);

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
						img.getSize().x, img.getSize().y,
						0,
						GL_RGBA,
						GL_UNSIGNED_BYTE,
						(const GLvoid*)img.getPixelsPtr());
			}
		}
	}

	m_bLoaded = true;

	return m_bLoaded;
}

void Model::unload()
{
	if(!m_bLoaded)
		return;

	m_indices.clear();
	m_vertices.clear();

	if(m_albedoTex != INVALID_GL_ID)
	{
		glDeleteTextures(1, &m_albedoTex);
		m_albedoTex = INVALID_GL_ID;
	}
	
	glDeleteBuffers(1, &m_vertexBufferId); m_vertexBufferId = INVALID_GL_ID;
	glDeleteVertexArrays(1, &m_vertexArrayId); m_vertexArrayId = INVALID_GL_ID;

	m_bLoaded = false;
}

void Model::draw(const Camera& camera)
{
	// OpenGL
	// X: left
	// Y: top
	// Z: front
	// 
	// Blender:
	// X: front
	// Y: left
	// Z: top

	glm::mat4 patchedModelMtx = glm::rotate(glm::rotate(m_modelMtx, glm::radians(-90.f), glm::vec3(0.f, 1.f, 0.f)), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));

	glm::mat4 modelViewProjMtx = camera.getViewProjMtx() * patchedModelMtx;
	const GPUProgram* modelProgram = gData.gpuProgramMgr->getProgram(PROG_MODEL);
	modelProgram->use();
	modelProgram->sendUniform("gModelViewProjMtx", modelViewProjMtx);
	modelProgram->sendUniform("texAlbedo", 0);
	modelProgram->sendUniform("gTime", gData.frameTime.asSeconds());

	if(m_albedoTex != INVALID_GL_ID)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_albedoTex);
	}

	glBindVertexArray(m_vertexArrayId);
	
	// Vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);
	glEnableVertexAttribArray(PROG_MODEL_ATTRIB_POSITIONS);
	glEnableVertexAttribArray(PROG_MODEL_ATTRIB_UVS);
	glEnableVertexAttribArray(PROG_MODEL_ATTRIB_NORMALS);
	glEnableVertexAttribArray(PROG_MODEL_ATTRIB_BONE_INDICES);
	glEnableVertexAttribArray(PROG_MODEL_ATTRIB_BONE_WEIGHTS);

	glVertexAttribPointer(PROG_MODEL_ATTRIB_POSITIONS	, sizeof(VtxModel::pos)			/sizeof(GLfloat),	GL_FLOAT,			GL_FALSE,	sizeof(VtxModel), (const GLvoid*)offsetof(VtxModel, pos));
	glVertexAttribPointer(PROG_MODEL_ATTRIB_UVS			, sizeof(VtxModel::uv)			/sizeof(GLfloat),	GL_FLOAT,			GL_FALSE,	sizeof(VtxModel), (const GLvoid*)offsetof(VtxModel, uv));
	glVertexAttribPointer(PROG_MODEL_ATTRIB_NORMALS		, sizeof(VtxModel::normal)		/sizeof(GLfloat),	GL_FLOAT,			GL_FALSE,	sizeof(VtxModel), (const GLvoid*)offsetof(VtxModel, normal));
	glVertexAttribPointer(PROG_MODEL_ATTRIB_BONE_INDICES, sizeof(VtxModel::boneIndices)	/sizeof(GLubyte),	GL_UNSIGNED_BYTE,	GL_TRUE,	sizeof(VtxModel), (const GLvoid*)offsetof(VtxModel, boneIndices));
	glVertexAttribPointer(PROG_MODEL_ATTRIB_BONE_WEIGHTS, sizeof(VtxModel::boneWeights)	/sizeof(GLubyte),	GL_UNSIGNED_BYTE,	GL_TRUE,	sizeof(VtxModel), (const GLvoid*)offsetof(VtxModel, boneWeights));

	// Index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferId);

	// Draw the triangles !
	glDrawElements(
		GL_TRIANGLES,				 // mode
		(GLsizei)m_indices.size(),    // count
		GL_UNSIGNED_SHORT,			  // type
		(void*)0					// element array buffer offset
	);

	glDisableVertexAttribArray(PROG_MODEL_ATTRIB_POSITIONS);
	glDisableVertexAttribArray(PROG_MODEL_ATTRIB_UVS);
	glDisableVertexAttribArray(PROG_MODEL_ATTRIB_NORMALS);
	glDisableVertexAttribArray(PROG_MODEL_ATTRIB_BONE_INDICES);
	glDisableVertexAttribArray(PROG_MODEL_ATTRIB_BONE_WEIGHTS);
}
