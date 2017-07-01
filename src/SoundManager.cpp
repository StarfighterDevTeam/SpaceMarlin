#include "SoundManager.h"

void SoundManager::init()
{
	m_bFadingOut = false;
	m_fadeAmount = 0.0f;
}

void SoundManager::shut()
{
	m_curMusic.stop();
}

void SoundManager::update()
{
	if(m_curMusicDesc.filename != m_nextMusicDesc.filename)
	{
		if(m_fadeAmount > 0.01f)
		{
			m_bFadingOut = true;
		}
		else
		{
			bool bOk = m_curMusic.openFromFile(m_nextMusicDesc.filename);
			assert(bOk);

			m_curMusic.setVolume(0.0f);
			m_curMusic.setLoop(m_nextMusicDesc.looping);
			m_curMusic.play();
			m_bFadingOut = false;

			m_curMusicDesc = m_nextMusicDesc;
		}
	}

	static float fadeSpeed = 1.0f;
	m_fadeAmount += (m_bFadingOut ? -1.0f : 1.0f) * fadeSpeed * gData.dTime.asSeconds();
	m_fadeAmount = clamp(m_fadeAmount, 0.0f, 1.0f);
	m_curMusic.setVolume(m_fadeAmount * 100.0f);
}