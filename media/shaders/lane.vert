#version 400 core
#include "SharedDefines.h"
PROG_VERTEX_SHADER(PROG_LANE)

out vec3 varViewSpaceNormal;
out vec3 varViewSpacePos;
out vec3 varWorldSpaceNormal;
out vec3 varWorldSpaceViewVec;

out vec3 varDebug;

void main()
{
#if 0
	vec3 lPlanePos = pos;
	vec3 lPlaneNormal = normal;

	float waterHeight = textureLod(texHeights, uv, 0).r;
	float heightRight	= textureLod(texHeights, uv + vec2(+gTexelSize.x,0), 0).r;
	float heightTop		= textureLod(texHeights, uv + vec2(0, +gTexelSize.y), 0).r;
	vec3 posRight	= vec3(lPlanePos.x + gDistBetweenTexels.x	, heightRight,	lPlanePos.z);
	vec3 posTop		= vec3(lPlanePos.x							, heightTop,	lPlanePos.z + gDistBetweenTexels.y);
	lPlaneNormal = normalize(cross(posTop-lPlanePos, posRight-lPlanePos));

	float gSideNbVtx = 100;	// GROS PORC
	float gGridSize = 10.f;	// GROS PORC
	//float cylinderRadius = 3 + 2*sin(gTime*20);
	float u = 0.5 * (lPlanePos.x/ ((gGridSize - (gGridSize/gSideNbVtx))/2) ) + 0.5;	// in [0,1]
	float angle = (-3.1415/2 + 2*3.1415*u);	// in [0,pi]
	vec3 lPos;
	lPos.z = lPlanePos.z;
	lPos.y = (1+waterHeight) * sin(angle);
	lPos.x = (1+waterHeight) * cos(angle);

	vec4 worldSpacePos = gLocalToWorldMtx * vec4(lPos, 1);
	worldSpacePos.xyz /= worldSpacePos.w;
	varWorldSpaceViewVec = worldSpacePos.xyz - gWorldSpaceCamPos;
	
	//lPos.x += 3*sin(gTime*1 + 0.3*lPos.z);
	//lPos.z *= 10;

	float rotAngle = angle - 3.1415/2;
	mat3 rotMtx = mat3(
			cos(rotAngle),	sin(rotAngle),	0,
			-sin(rotAngle),	cos(rotAngle),	0,
			0,				0,				1
		);

	// TODO: fix normal with inverse transpose of gLocalToWorldMtx
	vec3 lNormal = rotMtx * lPlaneNormal;

	varWorldSpaceNormal = mat3(gLocalToWorldMtx) * lNormal;
	varViewSpaceNormal = mat3(gLocalToViewMtx) * lNormal;
	varViewSpacePos = vec3( gLocalToViewMtx * vec4(lPos,1) );
	
	gl_Position = gLocalToProjMtx * vec4(lPos, 1);
#else
	varDebug = vec3(1,0,0);
	vec3 lPos = pos;
	vec3 lNormal = normal;

	float waterHeight	= textureLod(texHeights, uv, 0).r;
	float heightRight	= textureLod(texHeights, uv + vec2(+gTexelSize.x,0), 0).r;
	float heightTop		= textureLod(texHeights, uv + vec2(0, +gTexelSize.y), 0).r;
	vec3 posRight	= vec3( 0+ gDistBetweenTexels.x	, heightRight,	0);
	vec3 posTop		= vec3(	0						, heightTop,	0 + gDistBetweenTexels.y);
	vec3 lPlaneNormal = normalize(cross(posTop, posRight));
	// TODO: rotate lPlaneNormal

	const float _2pi = 2*M_PI;
	float posOnCapsule = uv.x*gKeyframeCapsulePerimeter;
	
	lPos.z = pos.z * gKeyframeCapsulePerimeter;

	if(posOnCapsule < gKeyframeThreshold0)
	{
		float curAngleOnC1 = posOnCapsule / gKeyframeR1;
		lPos.x = gKeyframeHalfDist + cos(curAngleOnC1) * gKeyframeR1;
		lPos.y = sin(curAngleOnC1) * gKeyframeR1;
		lNormal = vec3(cos(curAngleOnC1), sin(curAngleOnC1), 0);
	}
	else if(posOnCapsule < gKeyframeThreshold1)
	{
		float t = (posOnCapsule - gKeyframeThreshold0) / gKeyframeThreshold0to1;
		lPos.xy = gKeyframeTopRightPos*(1-t) + t*gKeyframeTopLeftPos;

		vec2 tangentVector = gKeyframeTopTangentVector;
		lNormal = vec3(tangentVector.y, -tangentVector.x, 0);
	}
	else if(posOnCapsule < gKeyframeThreshold2)
	{
		float curAngleOnC0 = gKeyframeTheta + ((posOnCapsule-gKeyframeThreshold1) / gKeyframeR0);
		lPos.x = -gKeyframeHalfDist + cos(curAngleOnC0) * gKeyframeR0;
		lPos.y = sin(curAngleOnC0) * gKeyframeR0;
		lNormal = vec3(cos(curAngleOnC0), sin(curAngleOnC0), 0);
	}
	else if(posOnCapsule < gKeyframeThreshold3)
	{
		float t = (posOnCapsule - gKeyframeThreshold2) / gKeyframeThreshold2to3;
		lPos.xy = gKeyframeBottomLeftPos*(1-t) + t*gKeyframeBottomRightPos;

		vec2 tangentVector = gKeyframeBottomTangentVector;
		lNormal = vec3(tangentVector.y, -tangentVector.x, 0);
	}
	else
	{
		float curAngleOnC1 = _2pi - gKeyframeTheta + ((posOnCapsule-gKeyframeThreshold3) / gKeyframeR1);
		lPos.x = gKeyframeHalfDist + cos(curAngleOnC1) * gKeyframeR1;
		lPos.y = sin(curAngleOnC1) * gKeyframeR1;
		lNormal = vec3(cos(curAngleOnC1), sin(curAngleOnC1), 0);
	}

	varDebug = lNormal;
	
	vec4 worldSpacePos = gKeyframeLocalToWorldMtx * vec4(lPos, 1);
	worldSpacePos.xyz /= worldSpacePos.w;

	vec3 worldSpaceNormal = mat3(gKeyframeLocalToWorldMtx) * lNormal;	// assuming no scaling
	worldSpacePos.xyz += waterHeight * worldSpaceNormal;

	varWorldSpaceViewVec = worldSpacePos.xyz - gWorldSpaceCamPos;
	varWorldSpaceNormal = worldSpaceNormal;
	varViewSpaceNormal = mat3(gWorldToViewMtx) * worldSpaceNormal;
	varViewSpacePos = vec3(gWorldToViewMtx * worldSpacePos);
	
	gl_Position = gWorldToProjMtx * worldSpacePos;
#endif
}
