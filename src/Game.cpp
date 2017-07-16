#include "Game.h"
#include "MainScene.h"
#include "TestScene.h"
#include "GPUProgramManager.h"
#include "Drawer.h"
#include "SoundManager.h"
#include "InputManager.h"

#ifdef _WIN32
	#include <Windows.h>
#else
	#include <unistd.h>
#endif

#define WIN_TITLE	"SpaceMarlin"
#define WIN_SIZE_X	1280
#define WIN_SIZE_Y	720

//#define _USE_FULLSCREEN
#ifndef NDEBUG
	#define _USE_KHR_DEBUG
	//#define _USE_KHR_DEBUG_VERBOSE
	#define _USE_DEBUG_CONTEXT
#endif

// Follow GLDEBUGPROC signature
void GLAPIENTRY debugCallback(GLenum source, GLenum type, GLuint id,
   GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

bool Game::init(sf::RenderWindow* window)
{
#ifdef NDEBUG
	logInfo("Release version");
#else
	logInfo("Debug version");
#endif

	// Init SFML RenderWindow
	gData.window = window;
	gData.window->setVerticalSyncEnabled(true);
	gData.winSizeX = (int)gData.window->getSize().x;
	gData.winSizeY = (int)gData.window->getSize().y;
	
	
	// Init GLEW
	GLenum err = glewInit();
	if(err != GLEW_OK)
	{
		fprintf(stderr, "Error: failed to initialize GLEW\n");
		return false;
	}

#ifdef _USE_KHR_DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(&debugCallback, NULL);
#endif

	logInfo("GPU vendor: ", glGetString(GL_VENDOR));
	//logInfo("OpenGL extensions: ", glGetString(GL_EXTENSIONS));	// TODO: unavailable in Core profile...

	m_wireframe	= false;
	m_slowMode	= false;

	// Init GPUProgramManager
#define INIT_MGR(mgr) do {mgr = new (std::remove_reference<decltype(*mgr)>::type); mgr->init(); } while(0)
	INIT_MGR(gData.gpuProgramMgr);
	INIT_MGR(gData.drawer);
	INIT_MGR(gData.soundMgr);
	INIT_MGR(gData.inputMgr);

	// Init Assimp logging
	// get a handle to the predefined STDOUT log stream and attach
	// it to the logging system. It remains active for all further
	// calls to aiImportFile(Ex) and aiApplyPostProcessing. */
	m_aiLogStream = aiGetPredefinedLogStream(aiDefaultLogStream_STDOUT, NULL);
	aiAttachLogStream(&m_aiLogStream);

	// ... same procedure, but this stream now writes the
	// log messages to assimp_log.txt
	m_aiLogStream = aiGetPredefinedLogStream(aiDefaultLogStream_FILE,"assimp_log.txt");
	aiAttachLogStream(&m_aiLogStream);

	m_scenes[SCENE_MAIN] = new MainScene();
	m_scenes[SCENE_TEST] = new TestScene();

	m_curScene = SCENE_MAIN;

	if(!m_scenes[m_curScene]->init())
		return false;
	return true;
}

void Game::shut()
{
	m_scenes[m_curScene]->shut();

	// We added a log stream to the library, it's our job to disable it
	// again. This will definitely release the last resources allocated
	// by Assimp.
	aiDetachAllLogStreams();

	// Shut managers
#define SHUT_MGR(mgr) do {mgr->shut(); delete mgr; mgr = NULL;} while(0)
	SHUT_MGR(gData.drawer);
	SHUT_MGR(gData.gpuProgramMgr);
	SHUT_MGR(gData.soundMgr);
	SHUT_MGR(gData.inputMgr);
}

void Game::run()
{
	bool running = true;
	while (running)
	{
		sf::Event event;
		while (gData.window->pollEvent(event))
		{
			if(event.type == sf::Event::Closed)
			{
				running = false;
			}
			else if(event.type == sf::Event::Resized)
			{
				gData.winSizeX = event.size.width;
				gData.winSizeY = event.size.height;
				glViewport(0, 0, event.size.width, event.size.height);
			}
			else if(gData.inputMgr->eventIsDebugSlowModeReleased(event))
			{
				m_slowMode = !m_slowMode;
				logDebug("slow mode: ", m_slowMode ? "on" : "off");
			}
			else if(gData.inputMgr->eventIsDebugWireframeReleased(event))
			{
				m_wireframe = !m_wireframe;
				logDebug("wireframe: ", m_wireframe ? "on" : "off");
			}

			m_scenes[m_curScene]->onEvent(event);
		}

		update();
		draw();
	}
}

void Game::update()
{
	sf::Time prevFrameTime = gData.curFrameTime;
	gData.curFrameTime = gData.clock.getElapsedTime();
	gData.dTime = sf::microseconds(gData.curFrameTime.asMicroseconds() - prevFrameTime.asMicroseconds());

	if(m_slowMode)
	{
		static float sNbSeconds = 0.5f;
		sf::sleep(sf::seconds(sNbSeconds));
	}

	gData.gpuProgramMgr->update();
	gData.soundMgr->update();
	gData.inputMgr->update();
	m_scenes[m_curScene]->update();
}

void Game::draw()
{
	if(m_wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	m_scenes[m_curScene]->draw();

	if(m_wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	m_scenes[m_curScene]->drawAfter();

	gData.window->display();
}

struct LogRAII
{
	~LogRAII()
	{
		Log::close();
	}
} gLogRAII;

std::string getExePath()
{
#ifdef _WIN32
	char buff[MAX_PATH] = "";
	GetModuleFileNameA(GetModuleHandle(NULL), buff, sizeof(buff));
#else
    char buff[PATH_MAX] = "";
    ssize_t len = ::readlink("/proc/self/exe", buff, sizeof(buff)-1);
    if(len != -1)
		buff[len] = '\0';
#endif
	for(int i=(int)strlen(buff)-1 ; i >= 0 ; i--)
	{
		if(buff[i] == '/' || buff[i] == '\\')
		{
			buff[i] = '\0';
			break;
		}
	}
	return buff;
}

int main(int argc, char* argv[])
{
	Log::open(argc, argv);

	gData.exePath = getExePath();
	gData.assetsPath = gData.exePath + SDIR_SEP "media";
	gData.shadersPath = gData.exePath + SDIR_SEP "media" SDIR_SEP "shaders";

	logInfo("gData.exePath = ", gData.exePath);
	logInfo("gData.assetsPath = ", gData.assetsPath);
	logInfo("gData.shadersPath = ", gData.shadersPath);

	sf::Uint32 style = sf::Style::Default;
#ifdef _USE_FULLSCREEN
	style = sf::Style::Fullscreen;
#endif

	sf::ContextSettings contextSettings;
	contextSettings.attributeFlags = sf::ContextSettings::Core;
#ifdef _USE_DEBUG_CONTEXT
	contextSettings.attributeFlags |= sf::ContextSettings::Debug;
#endif
	contextSettings.majorVersion = 4;
	contextSettings.minorVersion = 0;
	contextSettings.depthBits = 24;
	contextSettings.stencilBits = 8;
	sf::RenderWindow window(sf::VideoMode(WIN_SIZE_X, WIN_SIZE_Y), WIN_TITLE, style, contextSettings);

	Game game;
	if(!game.init(&window))
	{
#ifdef _WIN32
		system("pause");
#endif
		return EXIT_FAILURE;
	}
	game.run();
	game.shut();
	return 0;
}

#define FOREACH_GL_DEBUG_SOURCE(HANDLE_GL_DEBUG_SOURCE)	\
	HANDLE_GL_DEBUG_SOURCE(GL_DEBUG_SOURCE_API, "Calls to the OpenGL API")	\
	HANDLE_GL_DEBUG_SOURCE(GL_DEBUG_SOURCE_WINDOW_SYSTEM, "Calls to a window-system API")	\
	HANDLE_GL_DEBUG_SOURCE(GL_DEBUG_SOURCE_SHADER_COMPILER, "A compiler for a shading language")	\
	HANDLE_GL_DEBUG_SOURCE(GL_DEBUG_SOURCE_THIRD_PARTY, "An application associated with OpenGL")	\
	HANDLE_GL_DEBUG_SOURCE(GL_DEBUG_SOURCE_APPLICATION, "Generated by the user of this application")	\
	HANDLE_GL_DEBUG_SOURCE(GL_DEBUG_SOURCE_OTHER, "Some source that isn't one of these")	\
		
#define FOREACH_GL_DEBUG_TYPE(HANDLE_GL_DEBUG_TYPE)	\
	HANDLE_GL_DEBUG_TYPE(GL_DEBUG_TYPE_ERROR, "An error, typically from the API")	\
	HANDLE_GL_DEBUG_TYPE(GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR, "Some behavior marked deprecated has been used")	\
	HANDLE_GL_DEBUG_TYPE(GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR, "Something has invoked undefined behavior")	\
	HANDLE_GL_DEBUG_TYPE(GL_DEBUG_TYPE_PORTABILITY, "Some functionality the user relies upon is not portable")	\
	HANDLE_GL_DEBUG_TYPE(GL_DEBUG_TYPE_PERFORMANCE, "Code has triggered possible performance issues")	\
	HANDLE_GL_DEBUG_TYPE(GL_DEBUG_TYPE_MARKER, "Command stream annotation")	\
	HANDLE_GL_DEBUG_TYPE(GL_DEBUG_TYPE_PUSH_GROUP, "Group pushing")	\
	HANDLE_GL_DEBUG_TYPE(GL_DEBUG_TYPE_POP_GROUP, "foo")	\
	HANDLE_GL_DEBUG_TYPE(GL_DEBUG_TYPE_OTHER, "Some type that isn't one of these")	\

void GLAPIENTRY debugCallback(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam)
{
	std::string str;

	switch(source)
	{
#define HANDLE_GL_DEBUG_SOURCE(glEnum, msg)	\
	case glEnum: str += " "; str += msg; str += " "; break;	\

	FOREACH_GL_DEBUG_SOURCE(HANDLE_GL_DEBUG_SOURCE)
	default:
		str += "UNKNOWN SOURCE!";
		break;
	}

	switch(type)
	{
#define HANDLE_GL_DEBUG_TYPE(glEnum, msg)	\
	case glEnum: str += " "; str += msg; str += " "; break;	\

	FOREACH_GL_DEBUG_TYPE(HANDLE_GL_DEBUG_TYPE)
	default:
		str += " UNKNOWN TYPE! ";
		break;
	}

	switch(severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:		// All OpenGL Errors, shader compilation/linking errors, or highly-dangerous undefined behavior
		logError("KHR_Debug [HIGH]: [", str.c_str(), "]", message);
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		logWarn("KHR_Debug [MEDIUM]: [", str.c_str(), "]", message);
		break;
	case GL_DEBUG_SEVERITY_LOW:
		logWarn("KHR_Debug [LOW]: [", str.c_str(), "]", message);
		break;
	case GL_DEBUG_SEVERITY_NOTIFICATION:
	#ifdef _USE_KHR_DEBUG_VERBOSE
		logInfo("KHR_Debug [NOTIF]: [", str.c_str(), "]", message);
	#endif
		break;
	}
}
