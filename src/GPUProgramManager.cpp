#include "GPUProgramManager.h"
#include "Camera.h"
#include "glutil\glutil.h"

struct GPUProgramMetadata
{
	struct VertexAttribute
	{
		GLint		location;
		const char*	name;
	};

	struct Item
	{
		const char*						programName;
		const char*						vertexFileName;
		const char*						fragmentFileName;
		std::vector<VertexAttribute>	vertexAttributes;
		std::list<std::string>			uniformNames;
	};

	Item items[PROG_COUNT];

	GPUProgramMetadata()
	{
		GPUProgramId curId = (GPUProgramId)0;

	#define HANDLE_PROG_GET_METADATA(progId)	items[curId].programName = #progId; HANDLE_##progId

	#define BEGIN_PROGRAM_GET_METADATA(vtxStructType, _vertexFileName, _fragmentFileName)	\
		items[curId].vertexFileName		= _vertexFileName;		\
		items[curId].fragmentFileName	= _fragmentFileName;

	#define HANDLE_UNIFORM_GET_METADATA(type, varName)	\
		items[curId].uniformNames.push_back(#varName);

	#define HANDLE_ATTRIBUTE_GET_METADATA(vtxStructType, varType, componentType, componentTypeEnum, normalized, varName, loc)	\
		items[curId].vertexAttributes.push_back({loc, #varName});

	#define END_PROGRAM_GET_METADATA	\
		curId = (GPUProgramId)(curId+1);

		FOREACH_GPUPROGRAM(HANDLE_PROG_GET_METADATA, BEGIN_PROGRAM_GET_METADATA, HANDLE_UNIFORM_GET_METADATA, HANDLE_ATTRIBUTE_GET_METADATA, END_PROGRAM_GET_METADATA)
	}
} gProgramMetadata;

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
				std::stringstream ss;
				ss << "Updating program " << gProgramMetadata.items[progId].programName
					<< "\n(vertex: " << orgWatchedProgram.m_program->getVertexFilename() << ")"
					<< "\n(fragment: " << orgWatchedProgram.m_program->getFragmentFilename() << ")";

				logInfo(ss.str());
				GPUProgram* newProgram = createProgram(progId);
				if(newProgram)
					m_programs[progId] = newProgram;
				else
				{
			#ifdef WIN32
					MessageBoxA(gData.window->getSystemHandle(), ss.str().c_str(), "Error updating program", MB_ICONERROR|MB_OK|MB_SYSTEMMODAL);
			#endif
				}
			}
		}
	}
#endif
}

void GPUProgramManager::sendCommonUniforms(const GPUProgram* program, const Camera& camera, const mat4& localToWorldMtx)
{
	const mat4& worldToViewMtx = camera.getWorldToViewMtx();
	const mat4& worldToProjMtx = camera.getWorldToProjMtx();
	const mat4& projToWorldRotMtx = camera.getProjToWorldRotMtx();
	const mat4& viewToProjMtx = camera.getViewToProjMtx();
	mat4 localToProjMtx = worldToProjMtx * localToWorldMtx;
	mat4 localToViewMtx = worldToViewMtx * localToWorldMtx;
	mat3 localToWorldNormalMtx = glm::transpose(glm::inverse(mat3(localToWorldMtx)));
	
	program->sendUniform("gTime", gData.curFrameTime.asSeconds());
	program->sendUniform("gLocalToProjMtx", localToProjMtx);
	program->sendUniform("gLocalToViewMtx", localToViewMtx);
	program->sendUniform("gLocalToWorldMtx", localToWorldMtx);
	program->sendUniform("gLocalToWorldNormalMtx", localToWorldNormalMtx, false, Hash::AT_RUNTIME);
	program->sendUniform("gWorldToViewMtx", worldToViewMtx);
	program->sendUniform("gWorldToProjMtx", worldToProjMtx);
	program->sendUniform("gProjToWorldRotMtx", projToWorldRotMtx);
	program->sendUniform("gViewToProjMtx", viewToProjMtx);
	
	GLint curVp[4];
	glGetIntegerv(GL_VIEWPORT, curVp);
	program->sendUniform("gVpSize", vec2(curVp[2], curVp[3]));
}

GPUProgram* GPUProgramManager::createProgram(GPUProgramId id)
{
	bool bOk = false;

	GPUProgram* program = new GPUProgram(
		(gData.shadersPath + SDIR_SEP + gProgramMetadata.items[id].vertexFileName).c_str(),
		(gData.shadersPath + SDIR_SEP + gProgramMetadata.items[id].fragmentFileName).c_str());
	if(program->compileAndAttach())
	{
		for(GPUProgramMetadata::VertexAttribute& attrib : gProgramMetadata.items[id].vertexAttributes)
			program->bindAttribLocation(attrib.location, attrib.name);
		if(program->link())
		{
			program->setUniformNames(gProgramMetadata.items[id].uniformNames);

			bOk = true;
		}
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
