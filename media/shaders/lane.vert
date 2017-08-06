#version 400 core
#include "SharedDefines.h"
PROG_VERTEX_SHADER(PROG_LANE)

#include "GLSLCommon.h"

out vec3 varViewSpaceNormal;
out vec3 varViewSpacePos;
out vec3 varWorldSpaceNormal;
out vec3 varWorldSpaceViewVec;

out vec3 varDebug;

void main()
{
	varDebug = vec3(1,0,0);
	vec3 lPos = pos;
	vec3 lNormal = normal;

	float waterHeight	= textureLod(texHeights, uv, 0).r;
	vec3 waterNormal = textureLod(texNormals, uv, 0).rgb;

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

	// Rotate waterNormal according to lNormal
	vec3 lRight = vec3(lNormal.y, -lNormal.x, 0);
	vec3 lBack = vec3(0,0,1);
	vec3 lNewNormal;

	lNewNormal = waterNormal.x * (-lRight) + waterNormal.y * lNormal + waterNormal.z * lBack;
	//varDebug = lNewNormal;
	lNormal = lNewNormal;
	
	mat4 localToWorldMtx = quatPosToMat4(gKeyframeRot, gKeyframeTrans);

	vec4 worldSpacePos = localToWorldMtx * vec4(lPos, 1);
	worldSpacePos.xyz /= worldSpacePos.w;

	vec3 worldSpaceNormal = mat3(localToWorldMtx) * lNormal;	// assuming no scaling
	worldSpacePos.xyz += waterHeight * worldSpaceNormal;

	varWorldSpaceViewVec = worldSpacePos.xyz - gWorldSpaceCamPos;
	varWorldSpaceNormal = worldSpaceNormal;
	varViewSpaceNormal = mat3(gWorldToViewMtx) * worldSpaceNormal;
	varViewSpacePos = vec3(gWorldToViewMtx * worldSpacePos);
	
	gl_Position = gWorldToProjMtx * worldSpacePos;

	varDebug = vec3(
		texelFetch(texKeyframes, 3*2 + 0).r,
		texelFetch(texKeyframes, 3*2 + 1).r,
		texelFetch(texKeyframes, 3*2 + 2).r
		);
}
