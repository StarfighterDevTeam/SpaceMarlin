#pragma once

#ifndef _GPU_PROGRAM_MANAGER_H
#define _GPU_PROGRAM_MANAGER_H

#include "glutil\GPUProgram.h"

#ifdef _WIN32
	#define _USE_LIVE_SHADERS_UPDATE
	#include <windows.h>
#endif

#define FOREACH_GPUPROGRAM(HANDLE_GPUPROGRAM)	\
	HANDLE_GPUPROGRAM(PROG_MODEL)				\
	HANDLE_GPUPROGRAM(PROG_SIMPLE)				\
	HANDLE_GPUPROGRAM(PROG_LANE)				\
	HANDLE_GPUPROGRAM(PROG_SKYBOX)				\
	/* next comes here */

enum GPUProgramId
{
#define HANDLE_GPUPROGRAM_ENUM(progId) progId,

	FOREACH_GPUPROGRAM(HANDLE_GPUPROGRAM_ENUM)
	
	PROG_COUNT,
};

class GPUProgramManager
{
public:
	void						init();
	void						shut();
	void						update();

	const GPUProgram*			getProgram(GPUProgramId id) const {return m_programs[id];}

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
