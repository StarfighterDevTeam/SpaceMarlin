#ifndef _SOUND_MANAGER_H
#define _SOUND_MANAGER_H

#include "Globals.h"

struct MusicDesc
{
	std::string	filename;
	bool looping;

	MusicDesc() {reset();}
	void reset() {filename = ""; looping=true;}
};

class SoundManager
{
	sf::Music				m_curMusic;
	MusicDesc				m_curMusicDesc;
	MusicDesc				m_nextMusicDesc;
	bool					m_bFadingOut;
	float					m_fadeAmount;
	bool					m_bMute;
	float					m_bPM;

public:
	void		init();
	void		shut();
	void		update();
	
	void		setNextMusic(const MusicDesc& desc, float bPM) { m_nextMusicDesc = desc; m_bPM = bPM; }
	void		setNextMusic(const std::string& filename, float bPM)
	{
		MusicDesc desc;
		desc.filename = filename;
		setNextMusic(desc, bPM);
		m_bPM = bPM;
	};

	bool toggleMuteMusic();

	sf::Music&	getCurMusic() { return m_curMusic; };
	float getBPM() const { return m_bPM; };
};

#endif // _SOUND_MANAGER_H
