#pragma once

#ifndef _GPU_PROGRAM_MANAGER_H
#define _GPU_PROGRAM_MANAGER_H

#include "glutil\GPUProgram.h"

#ifdef _WIN32
	#define _USE_LIVE_SHADERS_UPDATE
	#include <windows.h>
#endif

#include "../media/shaders/SharedDefines.h"

enum GPUProgramId
{
#define HANDLE_GPUPROGRAM_ENUM(progId)	progId, NO_ACTION

	FOREACH_GPUPROGRAM(HANDLE_GPUPROGRAM_ENUM, NO_ACTION, NO_ACTION, NO_ACTION, NO_ACTION())
	
	PROG_COUNT,
};

#define HANDLE_ATTRIBUTE_SETUP_VERTEX_ATTRIB(vtxStructType, varType, componentType, componentTypeEnum, normalized, varName, loc)	\
	glEnableVertexAttribArray(loc);																				\
	glVertexAttribPointer(loc,									/* attribute location */						\
		sizeof(vtxStructType::varName) / sizeof(componentType), /* number of components (e.g 3 for a vec3) */	\
		componentTypeEnum,										/* e.g GL_FLOAT */								\
		normalized,												/* e.g GL_FALSE */								\
		sizeof(vtxStructType),									/* stride */									\
		(const GLvoid*)offsetof(vtxStructType, varName)			/* "pointer" */									\
	);

#define SETUP_PROGRAM_VERTEX_ATTRIB(progId)	HANDLE_##progId(NO_ACTION, NO_ACTION, HANDLE_ATTRIBUTE_SETUP_VERTEX_ATTRIB)

class Camera;

class GPUProgramManager
{
public:
	void						init();
	void						shut();
	void						update();

	const GPUProgram*			getProgram(GPUProgramId id) const {return m_programs[id];}
	void						sendCommonUniforms(const GPUProgram* program, const Camera& camera, const mat4& localToWorldMtx);

private:
	static GPUProgram*			createProgram(GPUProgramId id);

	GPUProgram*					m_programs[PROG_COUNT];

	// Directory watching
#ifdef _USE_LIVE_SHADERS_UPDATE
	void		directoryWatchFunc();
	static void	directoryWatchFuncWrapper(GPUProgramManager* that) {that->directoryWatchFunc();}

	sf::Thread*					m_directoryWatchThread;
	volatile bool				m_directoryWatchNeedToCheck;
	HANDLE						m_directoryWatchFinishedEvent;
	struct WatchedProgram
	{
		GPUProgramId	m_programId;
		GPUProgram*		m_program;
		FILETIME		m_lastWriteTimeVertexShader;
		FILETIME		m_lastWriteTimeFragmentShader;

		void set(GPUProgramId progId, GPUProgram* program);
	};
	std::vector<WatchedProgram>	m_directoryWatchPrograms;
#endif
};

#endif // _GPU_PROGRAM_MANAGER_H
