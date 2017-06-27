#include "Globals.h"

#include "GPUProgramManager.h"

Globals gData;

Globals::Globals()
{
	winSizeX=0;
	winSizeY=0;
	dTime = sf::seconds(1.f/60.f);
	gpuProgramMgr = NULL;
}

#ifdef _WIN32
	// ---------- Assimp ---------
	#pragma comment(lib, "assimp-vc140-mt.lib")

	// ---------- GLEW ---------
	#pragma comment(lib, "glew32.lib")

	// ---------- SFML ---------
	#pragma comment(lib, "flac.lib")
	#pragma comment(lib, "freetype.lib")
	#pragma comment(lib, "jpeg.lib")
	#pragma comment(lib, "ogg.lib")
	#pragma comment(lib, "openal32.lib")
	#pragma comment(lib, "opengl32.lib")
	#pragma comment(lib, "vorbis.lib")
	#pragma comment(lib, "vorbisenc.lib")
	#pragma comment(lib, "vorbisfile.lib")
	#pragma comment(lib, "winmm.lib")

	#ifdef NDEBUG
		#pragma comment(lib, "sfml-audio-s.lib")
		#pragma comment(lib, "sfml-graphics-s.lib")
		#pragma comment(lib, "sfml-graphics.lib")
		#pragma comment(lib, "sfml-main.lib")
		#pragma comment(lib, "sfml-network-s.lib")
		#pragma comment(lib, "sfml-system-s.lib")
		#pragma comment(lib, "sfml-window-s.lib")
	#else
		#pragma comment(lib, "sfml-audio-s-d.lib")
		#pragma comment(lib, "sfml-graphics-s-d.lib")
		#pragma comment(lib, "sfml-main-d.lib")
		#pragma comment(lib, "sfml-network-s-d.lib")
		#pragma comment(lib, "sfml-system-s-d.lib")
		#pragma comment(lib, "sfml-window-s-d.lib")
	#endif
#endif
