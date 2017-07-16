#include "FollowCamera.h"
#include "SceneObject.h"

void FollowCamera::init()
{
	Camera::init();
	m_followedObject = NULL;
}

void FollowCamera::shut()
{
	Camera::shut();
	m_followedObject = NULL;
}

void FollowCamera::update()
{
	Camera::update();
	if(isFlyOver() || !m_followedObject)
		return;

	const mat4& objectLocalToWorldMtx = m_followedObject->getLocalToWorldMtx();
	const vec3& objectWorldSpaceFront = vec3(-objectLocalToWorldMtx[2]);
	const vec3& objectWorldSpaceUp = vec3(objectLocalToWorldMtx[1]);
	vec3 objectPos = m_followedObject->getPosition();

	static float distToObject = 7.f;
	static float cameraHeight = 3.f;
	vec3 targetPos		= objectPos - distToObject*objectWorldSpaceFront + cameraHeight*objectWorldSpaceUp;
	vec3 targetUp		= objectWorldSpaceUp;
	vec3 targetFront	= glm::normalize(objectPos-targetPos);

	setPosition(targetPos);
	setUp(targetUp);
	setFront(targetFront);
}
