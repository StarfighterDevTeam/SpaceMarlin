#pragma once
#ifndef _SCORE_H
#define _SCORE_H

#include "Globals.h"
#include "Lane.h"

class Score;

struct LaneTrack
{
	std::vector<LaneKeyframe>	keyframes;
	std::vector<LaneKeyframe>	normalizedKeyframes;

	void						computeNormalizedKeyframes();

private:
	Score*						pScore;
	friend class Score;
};

class Score
{
private:
	int							m_nbBeats;
	sf::Time					m_beatDuration;
	std::vector<LaneTrack>		m_laneTracks;

public:
	bool				load(const char* fileName);
	bool				save(const char* fileName);
	void				unload();

	const int			getNbBeats() const				{return m_nbBeats;}
	const sf::Time&		getBeatDuration() const			{return m_beatDuration;}
	const sf::Time		getScoreDuration() const		{return m_beatDuration * (float)m_nbBeats;}
	int					getLaneTrackCount() const		{return (int)m_laneTracks.size();}
	const LaneTrack&	getLaneTrack(int laneId) const	{return m_laneTracks[laneId];}
	LaneTrack&			getLaneTrack(int laneId)		{return m_laneTracks[laneId];}
};

#endif // _SCORE_H
