#include "Score.h"

bool Score::load(const char* fileName)
{
	// TODO: for now, use a hardcoded score for the lanes
	m_duration = sf::seconds(5.f);

	//const int nbLanes = 1;
	const int nbLanes = 2;
	m_laneTracks.resize(nbLanes);
	for(int i = 0; i < nbLanes ; i++)
	{
		const vec3 debugPos = vec3(10.f*i, 0, 0);

		std::vector<LaneKeyframe>& keyframes = m_laneTracks[i].keyframes;
		LaneKeyframe kf0;
		LaneKeyframe kf1;

		kf0.t = sf::seconds(0.f * m_duration.asSeconds());
		kf0.dist = 4.f;
		kf0.r0 = 2.f;
		kf0.r1 = 0.8f;
		kf0.pos = debugPos;
		keyframes.push_back(kf0);
		
		kf1 = kf0;
		kf1.t = sf::seconds(0.5f * m_duration.asSeconds());
		kf1.dist = 2.f;
		kf1.r0 = 0.8f;
		kf1.r1 = 2.f;
		//kf1.pos = debugPos + vec3(0.f, 1.f, 0.f);
		//kf1.roll = M_PI/2.f;
		keyframes.push_back(kf1);
		
		kf0.t = sf::seconds(1.f * m_duration.asSeconds());
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
	{
		// TODO
	}
}
