#pragma once
#ifndef _SCORE_H
#define _SCORE_H

#include "Globals.h"
#include "Lane.h"

struct LaneTrack
{
	std::vector<LaneKeyframe>	keyframes;
	std::vector<LaneKeyframe>	normalizedKeyframes;
};

class Score
{
private:
	sf::Time					m_duration;
	sf::Time					m_timeBetweenKeyframes;
	std::vector<LaneTrack>		m_laneTracks;

public:
	bool				load(const char* fileName);
	bool				save(const char* fileName);
	void				unload();

	const sf::Time&		getDuration() const {return m_duration;}
	int					getLaneTrackCount() const {return (int)m_laneTracks.size();}
	const LaneTrack&	getLaneTrack(int laneId) const {return m_laneTracks[laneId];}

private:
	void				createNormalizedKeyframes();
};

#endif // _SCORE_H
