#include "Model.h"

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

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
			strncpy(materialsDir, path, i+1);
	}

	return loadFromAssImpMesh(mesh, scene, materialsDir);
}

bool Model::loadFromAssImpMesh(const aiMesh* mesh, const aiScene* scene, const char* materialsDir)
{
	assert(!m_bLoaded);

	// Fill vertices positions
	m_positions.reserve(mesh->mNumVertices);
	for(unsigned int i=0; i<mesh->mNumVertices; i++){
		aiVector3D pos = mesh->mVertices[i];
		m_positions.push_back(glm::vec3(pos.x, pos.y, pos.z));
	}

	// Fill vertices texture coordinates
	m_uvs.reserve(mesh->mNumVertices);
	for(unsigned int i=0; i<mesh->mNumVertices; i++){
		aiVector3D UVW = mesh->mTextureCoords[0][i]; // Assume only 1 set of UV coords; AssImp supports 8 UV sets.
		m_uvs.push_back(glm::vec2(UVW.x, UVW.y));
	}

	// Fill vertices normals
	m_normals.reserve(mesh->mNumVertices);
	for(unsigned int i=0; i<mesh->mNumVertices; i++){
		aiVector3D n = mesh->mNormals[i];
		m_normals.push_back(glm::vec3(n.x, n.y, n.z));
	}


	// Fill face indices
	m_indices.reserve(3*mesh->mNumFaces);
	for (unsigned int i=0; i<mesh->mNumFaces; i++){
		// Assume the model has only triangles.
		m_indices.push_back(mesh->mFaces[i].mIndices[0]);
		m_indices.push_back(mesh->mFaces[i].mIndices[1]);
		m_indices.push_back(mesh->mFaces[i].mIndices[2]);
	}

	// Fill vertex skinning weights
	if(mesh->HasBones())
	{
		m_boneIndices.resize(mesh->mNumVertices);
		m_boneWeights.resize(mesh->mNumVertices);

		memset(&m_boneIndices[0], 0, m_boneIndices.size() * sizeof(m_boneIndices[0]));
		memset(&m_boneWeights[0], 0, m_boneWeights.size() * sizeof(m_boneWeights[0]));

		for(unsigned int boneId = 0; boneId < mesh->mNumBones; boneId++)
		{
			const aiBone* bone = mesh->mBones[boneId];
			for( unsigned int weightId = 0; weightId < bone->mNumWeights; weightId++)
			{
				const aiVertexWeight& boneWeight = bone->mWeights[weightId];

				glm::u8vec4& vertexBoneIndices = m_boneIndices[boneWeight.mVertexId];
				glm::u8vec4& vertexBoneWeights = m_boneWeights[boneWeight.mVertexId];

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
		glGenVertexArrays(1, &m_vertexArrayID);
		glBindVertexArray(m_vertexArrayID);

		// Load into VBOs

		glGenBuffers(1, &m_positionsBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_positionsBuffer);
		glBufferData(GL_ARRAY_BUFFER, m_positions.size() * sizeof(glm::vec3), &m_positions[0], GL_STATIC_DRAW);

		glGenBuffers(1, &m_uvsBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_uvsBuffer);
		glBufferData(GL_ARRAY_BUFFER, m_uvs.size() * sizeof(glm::vec2), &m_uvs[0], GL_STATIC_DRAW);

		glGenBuffers(1, &m_normalsBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_normalsBuffer);
		glBufferData(GL_ARRAY_BUFFER, m_normals.size() * sizeof(glm::vec3), &m_normals[0], GL_STATIC_DRAW);

		assert(	(  m_boneIndices.size() &&  m_boneWeights.size() ) ||
				( !m_boneIndices.size() && !m_boneWeights.size() )
			);
		if(m_boneIndices.size())
		{
			glGenBuffers(1, &m_boneIndicesBuffer);
			glBindBuffer(GL_ARRAY_BUFFER, m_boneIndicesBuffer);
			glBufferData(GL_ARRAY_BUFFER, m_boneIndices.size() * sizeof(glm::u8vec4), &m_boneIndices[0], GL_STATIC_DRAW);
		}

		if(m_boneWeights.size())
		{
			glGenBuffers(1, &m_boneWeightsBuffer);
			glBindBuffer(GL_ARRAY_BUFFER, m_boneWeightsBuffer);
			glBufferData(GL_ARRAY_BUFFER, m_boneWeights.size() * sizeof(glm::u8vec4), &m_boneWeights[0], GL_STATIC_DRAW);
		}

		// Generate a buffer for the indices as well
		glGenBuffers(1, &m_indicesBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indicesBuffer);
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
	m_positions.clear();
	m_uvs.clear();
	m_normals.clear();
	m_boneIndices.clear();
	m_boneWeights.clear();

	if(m_albedoTex != INVALID_GL_ID)
	{
		glDeleteTextures(1, &m_albedoTex);
		m_albedoTex = INVALID_GL_ID;
	}
	
#define RELEASE_BUFFER(bufferName) if(bufferName) { glDeleteBuffers(1, &bufferName); bufferName = INVALID_GL_ID; }
	RELEASE_BUFFER(m_positionsBuffer);
	RELEASE_BUFFER(m_uvsBuffer);
	RELEASE_BUFFER(m_normalsBuffer);
	RELEASE_BUFFER(m_indicesBuffer);
	RELEASE_BUFFER(m_boneIndicesBuffer);
	RELEASE_BUFFER(m_boneWeightsBuffer);
#undef RELEASE_BUFFER

	glDeleteVertexArrays(1, &m_vertexArrayID); m_vertexArrayID = INVALID_GL_ID;

	m_bLoaded = false;
}

void Model::draw()
{
	if(m_albedoTex != INVALID_GL_ID)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_albedoTex);
	}

	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(MODEL_ATTRIB_POSITIONS);
	glBindBuffer(GL_ARRAY_BUFFER, m_positionsBuffer);
	glVertexAttribPointer(
		0,                  // attribute
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);

	// 2nd attribute buffer : UVs
	glEnableVertexAttribArray(MODEL_ATTRIB_UVS);
	glBindBuffer(GL_ARRAY_BUFFER, m_uvsBuffer);
	glVertexAttribPointer(
		1,                                // attribute
		2,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);

	// 3rd attribute buffer : normals
	glEnableVertexAttribArray(MODEL_ATTRIB_NORMALS);
	glBindBuffer(GL_ARRAY_BUFFER, m_normalsBuffer);
	glVertexAttribPointer(
		2,                                // attribute
		3,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);

	// Index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indicesBuffer);

	// Draw the triangles !
	glDrawElements(
		GL_TRIANGLES,      // mode
		(GLsizei)m_indices.size(),    // count
		GL_UNSIGNED_SHORT,   // type
		(void*)0           // element array buffer offset
	);

	glDisableVertexAttribArray(MODEL_ATTRIB_POSITIONS);
	glDisableVertexAttribArray(MODEL_ATTRIB_UVS);
	glDisableVertexAttribArray(MODEL_ATTRIB_NORMALS);
}
