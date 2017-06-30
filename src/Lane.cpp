#include "Lane.h"
#include "Drawer.h"

void Lane::init()
{
}

void Lane::shut()
{
}

void Lane::draw(const Camera& camera)
{
	static float gfQuadSize = 1.f;
	const glm::vec3 quadPos[] = {
		glm::vec3(-gfQuadSize,0,-gfQuadSize),
		glm::vec3(+gfQuadSize,0,-gfQuadSize),
		glm::vec3(+gfQuadSize,0,+gfQuadSize),
		glm::vec3(-gfQuadSize,0,+gfQuadSize),
	};
	gData.drawer->drawLine(camera, quadPos[0], COLOR_BLUE, quadPos[1], COLOR_BLUE);
	gData.drawer->drawLine(camera, quadPos[1], COLOR_BLUE, quadPos[2], COLOR_BLUE);
	gData.drawer->drawLine(camera, quadPos[2], COLOR_BLUE, quadPos[3], COLOR_BLUE);
	gData.drawer->drawLine(camera, quadPos[3], COLOR_BLUE, quadPos[0], COLOR_BLUE);
}

void Lane::update()
{
}
