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

	lPos.z *= 2*3.1415;
	
	float waterHeight	= textureLod(texHeights, uv, 0).r;
	float heightRight	= textureLod(texHeights, uv + vec2(+gTexelSize.x,0), 0).r;
	float heightTop		= textureLod(texHeights, uv + vec2(0, +gTexelSize.y), 0).r;
	vec3 posRight	= vec3( 0+ gDistBetweenTexels.x	, heightRight,	0);
	vec3 posTop		= vec3(	0						, heightTop,	0 + gDistBetweenTexels.y);
	vec3 lPlaneNormal = normalize(cross(posTop, posRight));
	// TODO: rotate lPlaneNormal

	//float waterHeight = 1*(0.5+0.5*sin(10*gTime));
	//lPos += lNormal * waterHeight;

	// TODO: lPos.x, lPos.y
	//varDebug.x = gl_VertexID/100.f;
	//varDebug = vec3(gKeyframeDist/4.f,0,0);
	const float _2pi = 2*M_PI;
	//float circlePerimeter0 = _2pi*
	float theta = acos( (gKeyframeR0 - gKeyframeR1) / gKeyframeDist );
	//float lengthOnC0 = theta * gKeyframeR0;
	float lengthOnC1 = theta * gKeyframeR1;
	float lengthOnC0 = (M_PI - theta) * gKeyframeR0;
	float lengthTangent = tan(theta) * (gKeyframeR0 - gKeyframeR1);
	//float capsulePerimeter = _2pi*gKeyframeR0 + 2*gKeyframeDist;
	float capsulePerimeter = 2 * (lengthOnC1 + lengthTangent + lengthOnC0);
	float posOnCapsule = uv.x*capsulePerimeter;
	varDebug=vec3(0);
	float thresholds[4];
	thresholds[0] = lengthOnC1;
	thresholds[1] = thresholds[0] + lengthTangent;
	thresholds[2] = thresholds[1] + 2*lengthOnC0;
	thresholds[3] = thresholds[2] + lengthTangent;

	lPos.z = pos.z * capsulePerimeter;

	// TEMP TEST
	{
		float angle = uv.x*_2pi;
		lPos.x = gKeyframeR0*cos(angle);
		lPos.y = gKeyframeR0*sin(angle);
		lPos.y -= 3;
	}

	float xOffsetOnC0 = (gKeyframeR0 - gKeyframeR1) * gKeyframeR0 / gKeyframeDist;
	float xOffsetOnC1 = (gKeyframeR0 - gKeyframeR1) * gKeyframeR1 / gKeyframeDist;

	if(posOnCapsule < thresholds[0])
	{
		varDebug = vec3(1,0,0);
		float curAngleOnC1 = posOnCapsule / gKeyframeR1;
		lPos.x = 0.5*gKeyframeDist + cos(curAngleOnC1) * gKeyframeR1;
		lPos.y = sin(curAngleOnC1) * gKeyframeR1;
		lNormal = vec3(cos(curAngleOnC1), sin(curAngleOnC1), 0);
	}
	else if(posOnCapsule < thresholds[1])
	{
		varDebug = vec3(0,1,0);
		float t = (posOnCapsule - thresholds[0]) / (thresholds[1]-thresholds[0]);
		vec2 startPos = vec2(0.5*gKeyframeDist + xOffsetOnC1, xOffsetOnC1 * tan(theta));
		vec2 endPos = vec2(-0.5*gKeyframeDist + xOffsetOnC0, xOffsetOnC0 * tan(theta));
		lPos.xy = startPos*(1-t) + t*endPos;

		vec2 tangentVector = normalize(endPos - startPos);
		lNormal = vec3(tangentVector.y, -tangentVector.x, 0);
	}
	else if(posOnCapsule < thresholds[2])
	{
		varDebug = vec3(0,0,1);

		float curAngleOnC0 = theta + ((posOnCapsule-thresholds[1]) / gKeyframeR0);
		lPos.x = -0.5*gKeyframeDist + cos(curAngleOnC0) * gKeyframeR0;
		lPos.y = sin(curAngleOnC0) * gKeyframeR0;
		lNormal = vec3(cos(curAngleOnC0), sin(curAngleOnC0), 0);
	}
	else if(posOnCapsule < thresholds[3])
	{
		varDebug = vec3(0,1,1);
		float t = (posOnCapsule - thresholds[2]) / (thresholds[3]-thresholds[2]);
		vec2 startPos = vec2(-0.5*gKeyframeDist + xOffsetOnC0, -xOffsetOnC0 * tan(theta));
		vec2 endPos = vec2(0.5*gKeyframeDist + xOffsetOnC1, -xOffsetOnC1 * tan(theta));
		lPos.xy = startPos*(1-t) + t*endPos;

		vec2 tangentVector = normalize(endPos - startPos);
		lNormal = vec3(tangentVector.y, -tangentVector.x, 0);
	}
	else
	{
		varDebug = vec3(1,1,1);

		float curAngleOnC1 = _2pi - theta + ((posOnCapsule-thresholds[3]) / gKeyframeR1);
		lPos.x = 0.5*gKeyframeDist + cos(curAngleOnC1) * gKeyframeR1;
		lPos.y = sin(curAngleOnC1) * gKeyframeR1;
		lNormal = vec3(cos(curAngleOnC1), sin(curAngleOnC1), 0);
	}

	varDebug = lNormal;
	
	// TODO: lNormal
	
	vec4 worldSpacePos = gKeyframeLocalToWorldMtx * vec4(lPos, 1);
	worldSpacePos.xyz /= worldSpacePos.w;

	//vec3 worldSpaceNormal = normalize(gLocalToWorldNormalMtx * lNormal);
	vec3 worldSpaceNormal = mat3(gKeyframeLocalToWorldMtx) * lNormal;	// assuming no scaling
	worldSpacePos.xyz += waterHeight * worldSpaceNormal;

	varWorldSpaceViewVec = worldSpacePos.xyz - gWorldSpaceCamPos;
	varWorldSpaceNormal = worldSpaceNormal;
	varViewSpaceNormal = mat3(gWorldToViewMtx) * worldSpaceNormal;
	varViewSpacePos = vec3(gWorldToViewMtx * worldSpacePos);
	
	gl_Position = gWorldToProjMtx * worldSpacePos;

	//varDebug = worldSpaceNormal;
	//varDebug = lPlaneNormal;
#endif
}
