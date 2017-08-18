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

	void	computeNormalizedKeyframes();

private:
	Score* pScore;	// TEMP TO BE REMOVED
	friend class Score;
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

	const sf::Time&		getDuration() const				{return m_duration;}
	const sf::Time&		getTimeBetweenKeyframes() const	{return m_timeBetweenKeyframes;}
	int					getLaneTrackCount() const		{return (int)m_laneTracks.size();}
	const LaneTrack&	getLaneTrack(int laneId) const	{return m_laneTracks[laneId];}
	LaneTrack&			getLaneTrack(int laneId)		{return m_laneTracks[laneId];}

private:
	void				createNormalizedKeyframes();
};

#endif // _SCORE_H
