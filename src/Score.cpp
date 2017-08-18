#include "Score.h"

void LaneTrack::computeNormalizedKeyframes()
{
	const size_t prevSize = normalizedKeyframes.size();
	normalizedKeyframes.clear();
	normalizedKeyframes.reserve(prevSize);

	const sf::Time duration = pScore->getDuration();
	const sf::Time timeBetweenKeyframes = pScore->getTimeBetweenKeyframes();
	for(sf::Time t=sf::microseconds(0ll) ; t < duration ; t+=timeBetweenKeyframes)
	{
		int idxPrev = 0;
		int idxNext = 0;
		while(keyframes[idxNext].t < t && idxNext < (int)keyframes.size()-1)
			idxNext++;
		idxPrev = std::max(0, idxNext-1);

		LaneKeyframe kf;
		kf.setFromKeyframes(keyframes[idxPrev], keyframes[idxNext], t);
		normalizedKeyframes.push_back(kf);
	}
}

bool Score::load(const char* fileName)
{
	// TODO: for now, use a hardcoded score for the lanes
	m_duration = sf::seconds(5.f);
	m_timeBetweenKeyframes = sf::seconds(0.25f);

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

		kf0.t = sf::seconds(0.f * m_duration.asSeconds());
		kf0.dist = 4.f;
		kf0.r0 = 2.f;
		kf0.r1 = 0.8f;
		kf0.offset = debugOffset;
		kf0.updatePrecomputedData();
		keyframes.push_back(kf0);
		
		kf1 = kf0;
		kf1.t = sf::seconds(0.5f * m_duration.asSeconds());
		kf1.dist = 2.f;
		kf1.r0 = 0.8f;
		kf1.r1 = 2.f;
		//kf1.offset = debugOffset + vec3(0.f, 1.f);
		//kf1.roll = M_PI/2.f;
		kf1.updatePrecomputedData();
		keyframes.push_back(kf1);
		
		kf0.t = sf::seconds(1.f * m_duration.asSeconds());
		kf0.updatePrecomputedData();
		keyframes.push_back(kf0);
	}

	createNormalizedKeyframes();
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

void Score::createNormalizedKeyframes()
{
	// Lane track
	for(LaneTrack& track : m_laneTracks)
		track.computeNormalizedKeyframes();

	// TODO: other tracks come here
}
