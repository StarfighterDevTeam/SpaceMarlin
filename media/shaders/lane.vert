#version 400 core
#include "SharedDefines.h"
PROG_VERTEX_SHADER(PROG_LANE)

out vec3 varViewSpaceNormal;
out vec3 varViewSpacePos;
out vec3 varWorldSpaceNormal;
out vec3 varWorldSpaceViewVec;

//out vec3 varDebug;

void main()
{
#if 1
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
	vec3 lPos = pos;
	vec3 lNormal = normal;

	lPos.z *= 3.1415;
	
	float waterHeight	= textureLod(texHeights, uv, 0).r;
	float heightRight	= textureLod(texHeights, uv + vec2(+gTexelSize.x,0), 0).r;
	float heightTop		= textureLod(texHeights, uv + vec2(0, +gTexelSize.y), 0).r;
	vec3 posRight	= vec3( 0+ gDistBetweenTexels.x	, heightRight,	0);
	vec3 posTop		= vec3(	0						, heightTop,	0 + gDistBetweenTexels.y);
	vec3 lPlaneNormal = normalize(cross(posTop, posRight));
	// TODO: rotate lPlaneNormal

	//float waterHeight = 1*(0.5+0.5*sin(10*gTime));
	//lPos += lNormal * waterHeight;
	
	vec4 worldSpacePos = gLocalToWorldMtx * vec4(lPos, 1);
	worldSpacePos.xyz /= worldSpacePos.w;

	vec3 worldSpaceNormal = normalize(gLocalToWorldNormalMtx * lNormal);
	worldSpacePos.xyz += waterHeight * worldSpaceNormal;

	varWorldSpaceViewVec = worldSpacePos.xyz - gWorldSpaceCamPos;
	varWorldSpaceNormal = worldSpaceNormal;
	varViewSpaceNormal = mat3(gWorldToViewMtx) * worldSpaceNormal;
	varViewSpacePos = vec3(gWorldToViewMtx * worldSpacePos);
	
	gl_Position = gWorldToProjMtx * worldSpacePos;

	//varDebug = worldSpaceNormal;
	varDebug = lPlaneNormal;
#endif
}
