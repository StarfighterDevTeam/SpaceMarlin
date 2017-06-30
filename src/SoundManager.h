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

public:
	void		init();
	void		shut();
	void		update();
	
	void		setNextMusic(const MusicDesc& desc) {m_nextMusicDesc = desc;}
	void		setNextMusic(const std::string& filename)
	{
		MusicDesc desc;
		desc.filename = filename;
		setNextMusic(desc);
	}

	sf::Music&	getCurMusic() {return m_curMusic;};
};

#endif // _SOUND_MANAGER_H
