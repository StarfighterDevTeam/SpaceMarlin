#include "Score.h"

void LaneTrack::computeNormalizedKeyframes()
{
	const size_t prevSize = normalizedKeyframes.size();
	normalizedKeyframes.clear();
	normalizedKeyframes.reserve(prevSize);

	const float fNbBeats = (float)pScore->getNbBeats();
	for(float curBeat=0.f ; curBeat < fNbBeats ; curBeat++)	// using floats this way is ok until 16,777,216 (see https://stackoverflow.com/questions/15094611/behavior-of-float-that-is-used-as-an-integer)
	{
		int idxPrev = 0;
		int idxNext = 0;
		while(keyframes[idxNext].beat < curBeat && idxNext < (int)keyframes.size()-1)
			idxNext++;
		idxPrev = std::max(0, idxNext-1);

		LaneKeyframe kf;
		kf.setFromKeyframes(keyframes[idxPrev], keyframes[idxNext], curBeat);
		normalizedKeyframes.push_back(kf);
	}
}

bool Score::load(const char* fileName)
{
	// TODO: for now, use a hardcoded score for the lanes
	m_nbBeats = 3;
	m_beatDuration = bpmToBeatDuration(135.38f);

	//const int nbLanes = 1;
	const int nbLanes = 2;
	m_laneTracks.resize(nbLanes);
	for(int i = 0; i < nbLanes ; i++)
	{
		const vec2 debugOffset = vec2(10.f*i, 0);

		m_laneTracks[i].pScore = this;

		std::vector<LaneKeyframe>& keyframes = m_laneTracks[i].keyframes;
		LaneKeyframe kf0;
		LaneKeyframe kf1;

		kf0.beat = 0;
		kf0.dist = 4.f;
		kf0.r0 = 2.f;
		kf0.r1 = 0.8f;
		kf0.offset = debugOffset;
		kf0.updatePrecomputedData();
		keyframes.push_back(kf0);
		
		kf1 = kf0;
		kf1.beat = 1;
		kf1.dist = 2.f;
		kf1.r0 = 0.8f;
		kf1.r1 = 2.f;
		//kf1.offset = debugOffset + vec3(0.f, 1.f);
		//kf1.roll = M_PI/2.f;
		kf1.updatePrecomputedData();
		keyframes.push_back(kf1);
		
		kf0.beat = 2;
		kf0.updatePrecomputedData();
		keyframes.push_back(kf0);
	}

	// Compute normalized keyframes:
	// - lane tracks:
	for(LaneTrack& track : m_laneTracks)
		track.computeNormalizedKeyframes();

	// - other tracks to come here
	return true;
}

bool Score::save(const char* fileName)
{
	assert(false && "Score::save() not implemented");
	return false;
}

void Score::unload()
{
	// TODO
}
