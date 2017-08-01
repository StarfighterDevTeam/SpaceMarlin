#pragma once
#ifndef _SCORE_H
#define _SCORE_H

#include "Globals.h"
#include "Lane.h"

struct LaneTrack
{
	std::vector<LaneKeyframe>	keyframes;
};

class Score
{
private:
	std::vector<LaneTrack>		m_laneTracks;

public:
	bool				load(const char* fileName);
	bool				save(const char* fileName);
	void				unload();

	int					getLaneTrackCount() const {return (int)m_laneTracks.size();}
	const LaneTrack&	getLaneTrack(int laneId) const {return m_laneTracks[laneId];}
};

#endif // _SCORE_H
