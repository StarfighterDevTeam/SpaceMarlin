#include "GPUProgramManager.h"
#include "SharedDefines.h"

static const char* gpuProgramNames[] = {
#define HANDLE_GPUPROGRAM_NAME(progId) #progId,
	FOREACH_GPUPROGRAM(HANDLE_GPUPROGRAM_NAME)
};

void GPUProgramManager::init()
{
	for(int progId=0 ; progId < PROG_COUNT ; progId++)
		m_programs[progId] = createProgram((GPUProgramId)progId);

#ifdef _USE_LIVE_SHADERS_UPDATE
	for(int progId=0 ; progId < PROG_COUNT ; progId++)
	{
		WatchedProgram watchedProgram;
		watchedProgram.set((GPUProgramId)progId, m_programs[progId]);
		m_directoryWatchPrograms.push_back(watchedProgram);
	}
	m_directoryWatchNeedToCheck = false;
	m_directoryWatchFinishedEvent = CreateEventA(NULL, TRUE, FALSE, NULL);
	m_directoryWatchThread = new sf::Thread(&directoryWatchFuncWrapper, this);
	m_directoryWatchThread->launch();
#endif
}

void GPUProgramManager::shut()
{
#ifdef _USE_LIVE_SHADERS_UPDATE
	SetEvent(m_directoryWatchFinishedEvent);
	m_directoryWatchThread->wait();
	delete m_directoryWatchThread;
#endif

	for(int i=0 ; i < _countof(m_programs) ; i++)
		SAFE_DELETE(m_programs[i]);
}

void GPUProgramManager::update()
{
#ifdef _USE_LIVE_SHADERS_UPDATE
	if(m_directoryWatchNeedToCheck)
	{
		m_directoryWatchNeedToCheck = false;

		logInfo("Checking updated shaders...");
		for(WatchedProgram& orgWatchedProgram : m_directoryWatchPrograms)
		{
			WatchedProgram watchedProgram;
			watchedProgram.set(orgWatchedProgram.m_programId, orgWatchedProgram.m_program);
			if(memcmp(&watchedProgram, &orgWatchedProgram, sizeof(WatchedProgram)) != 0)
			{
				const GPUProgramId progId = orgWatchedProgram.m_programId;
				logInfo("Updating program ", gpuProgramNames[progId],
					"\n(vertex:", orgWatchedProgram.m_program->getVertexFilename(), ")"
					"\n(fragment:", orgWatchedProgram.m_program->getFragmentFilename(), ")");
				GPUProgram* newProgram = createProgram(progId);
				if(newProgram)
					m_programs[progId] = newProgram;
			}
		}
	}
#endif
}

GPUProgram* GPUProgramManager::createProgram(GPUProgramId id)
{
	GPUProgram* program = NULL;
	bool bOk = false;

	switch(id)
	{
	case PROG_MODEL:
		program = new GPUProgram(
			(gData.shadersPath + SDIR_SEP "model.vert").c_str(),
			(gData.shadersPath + SDIR_SEP "model.frag").c_str());
		if(program->compileAndAttach())
		{
			program->bindAttribLocation(PROG_MODEL_ATTRIB_POSITIONS,	"pos");
			program->bindAttribLocation(PROG_MODEL_ATTRIB_UVS,			"uv");
			program->bindAttribLocation(PROG_MODEL_ATTRIB_NORMALS,		"normal");

			if(program->link())
			{
				program->setUniformNames(
					"gModelViewProjMtx",
					"texAlbedo",
					"gTime",
					NULL);
				bOk = true;
			}
		}
		break;
	case PROG_SIMPLE:
		program = new GPUProgram(
			(gData.shadersPath + SDIR_SEP "simple.vert").c_str(),
			(gData.shadersPath + SDIR_SEP "simple.frag").c_str());
		if(program->compileAndAttach())
		{
			program->bindAttribLocation(PROG_SIMPLE_ATTRIB_POSITIONS,	"pos");
			program->bindAttribLocation(PROG_SIMPLE_ATTRIB_COLORS,		"color");

			if(program->link())
			{
				program->setUniformNames(
					"gModelViewProjMtx",
					NULL);
				bOk = true;
			}
		}
		break;
	case PROG_LANE:
		program = new GPUProgram(
			(gData.shadersPath + SDIR_SEP "lane.vert").c_str(),
			(gData.shadersPath + SDIR_SEP "lane.frag").c_str());
		if(program->compileAndAttach())
		{
			program->bindAttribLocation(PROG_LANE_ATTRIB_POSITIONS,	"pos");

			if(program->link())
			{
				program->setUniformNames(
					"gModelViewProjMtx",
					"texAlbedo",
					"gTime",
					NULL);
				bOk = true;
			}
		}
		break;
	default:
		assert(false && "shouldn't happen");
		return NULL;
	}

	if(!bOk)
		SAFE_DELETE(program);
	return program;
}

void GPUProgramManager::directoryWatchFunc()
{
#ifdef _USE_LIVE_SHADERS_UPDATE
	DWORD dwWaitStatus;
	HANDLE dwChangeHandle = FindFirstChangeNotificationA(
		"media/shaders",
		TRUE,
		FILE_NOTIFY_CHANGE_LAST_WRITE);
	
	if(dwChangeHandle == INVALID_HANDLE_VALUE)
	{
		logError("FindFirstChangeNotificationA FAILED, aborting live shaders update");
		return;
	}

	HANDLE dwWaitedObjects[2] = {
		dwChangeHandle,
		m_directoryWatchFinishedEvent
	};

	bool bFinished = false;
	while(!bFinished)
	{
		dwWaitStatus = WaitForMultipleObjects(_countof(dwWaitedObjects), dwWaitedObjects, FALSE, INFINITE); 
		
		switch(dwWaitStatus)
		{
		case WAIT_OBJECT_0:
			m_directoryWatchNeedToCheck = true;
			if(FindNextChangeNotification(dwWaitedObjects[0]) == FALSE )
			{
				logError("FindNextChangeNotification(dwWaitedObjects[0]) failed, aborting live shaders update");
				bFinished = true;
			}
			break;
		case WAIT_OBJECT_0 + 1:
			bFinished = true;
			break;
		default:
			logError("Unhandled dwWaitStatus: ", dwWaitStatus);
			break;
		}
	}

	FindCloseChangeNotification(dwChangeHandle);
#endif
}

void GPUProgramManager::WatchedProgram::set(GPUProgramId progId, GPUProgram* program)
{
	assert(program);
	m_programId = progId;
	m_program = program;

	for(int i=0 ; i < 2 ; i++)
	{
		const char*	fileName		= (i == 0 ? m_program->getVertexFilename().c_str()	: m_program->getFragmentFilename().c_str());
		FILETIME*	pLastWriteTime	= (i == 0 ? &m_lastWriteTimeVertexShader			: &m_lastWriteTimeFragmentShader);

		HANDLE hFile = CreateFileA(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if(hFile == INVALID_HANDLE_VALUE)
			logError("CreateFileA() failed for ", fileName);
		else
		{
			FILETIME creationTime, lastAccessTime;
			if(!GetFileTime(hFile, &creationTime, &lastAccessTime, pLastWriteTime))
			{
				logError("GetFileTime() failed for ", fileName);
				memset(pLastWriteTime, 0, sizeof(*pLastWriteTime));
			}
			CloseHandle(hFile);
		}
	}
}
