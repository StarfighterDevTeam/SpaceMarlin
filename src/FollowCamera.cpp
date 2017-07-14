#include "FollowCamera.h"

void FollowCamera::update()
{
	Camera::update();
	if(isFlyOver())
		return;

	// TODO: follow behavior
}
