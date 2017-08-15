#version 400 core
#include "SharedDefines.h"
PROG_VERTEX_SHADER(PROG_LANE)

#include "GLSLCommon.h"

out vec3 varViewSpaceNormal;
out vec3 varViewSpacePos;
out vec3 varWorldSpaceNormal;
out vec3 varWorldSpaceViewVec;

out vec3 varDebug;

/*
GPULaneKeyframe setFromKeyframes(GPULaneKeyframe kf0, GPULaneKeyframe kf1, float u)
{
	GPULaneKeyframe kf;
	// TODO
	//kf.gKeyframeR0					=
	//kf.gKeyframeR1					=
	//kf.gKeyframeHalfDist			=
	//kf.gKeyframeTheta				=
	//kf.gKeyframeCapsulePerimeter	=
	//kf.gKeyframeThreshold0			=
	//kf.gKeyframeThreshold1			=
	//kf.gKeyframeThreshold2			=
	//kf.gKeyframeThreshold3			=
	//kf.gKeyframeThreshold0to1		=
	//kf.gKeyframeThreshold2to3		=
	//kf.gKeyframeTopRightPos			=
	//kf.gKeyframeTopLeftPos			=
	//kf.gKeyframeBottomLeftPos		=
	//kf.gKeyframeBottomRightPos		=
	//kf.gKeyframeTopTangentVector	=
	//kf.gKeyframeBottomTangentVector	=
	//kf.gKeyframeRot					=
	//kf.gKeyframeTrans				=
	
	return kf;
}
*/

#pragma custom_preprocessor_off
float texelFetch_float(samplerBuffer tex, inout int offset)	{	int off=offset;
																offset+=1;
																return			texelFetch(tex,off+0).r;}

vec2 texelFetch_vec2(samplerBuffer tex, inout int offset)	{	int off=offset;
																offset+=2;
																return vec2(	texelFetch(tex,off+0).r,
																				texelFetch(tex,off+1).r);}

vec3 texelFetch_vec3(samplerBuffer tex, inout int offset)	{	int off=offset;
																offset+=3;
																return vec3(	texelFetch(tex,off+0).r,
																				texelFetch(tex,off+1).r,
																				texelFetch(tex,off+2).r);}

vec4 texelFetch_vec4(samplerBuffer tex, inout int offset)	{	int off=offset;
																offset+=4;
																return vec4(	texelFetch(tex,off+0).r,
																				texelFetch(tex,off+1).r,
																				texelFetch(tex,off+2).r,
																				texelFetch(tex,off+3).r);}


GPULaneKeyframe readKeyframeAtOffset(samplerBuffer tex, int offset)
{
	GPULaneKeyframe kf;
	int curOffset = offset;
#define HANDLE_UNIFORM_TEXEL_FETCH(type, varName)	\
		kf.varName = texelFetch_##type(tex, curOffset);
		
	FOREACH_LANE_KEYFRAME_MEMBER(HANDLE_UNIFORM_TEXEL_FETCH)
	
	return kf;
}
#pragma custom_preprocessor_on

void main()
{
	varDebug = vec3(1,0,0);
	vec3 lPos = pos;
	vec3 lNormal = normal;

	float waterHeight	= textureLod(texHeights, uv, 0).r;
	vec3 waterNormal = textureLod(texNormals, uv, 0).rgb;

	GPULaneKeyframe kf = readKeyframeAtOffset(texKeyframes, 10 * dimension_GPULaneKeyframe);

	const float _2pi = 2*M_PI;
	float posOnCapsule = uv.x*kf.gKeyframeCapsulePerimeter;
	
	//lPos.z = pos.z * kf.gKeyframeCapsulePerimeter;
	lPos.z = -uv.y * gLaneLength;

	if(posOnCapsule < kf.gKeyframeThreshold0)
	{
		float curAngleOnC1 = posOnCapsule / kf.gKeyframeR1;
		lPos.x = kf.gKeyframeHalfDist + cos(curAngleOnC1) * kf.gKeyframeR1;
		lPos.y = sin(curAngleOnC1) * kf.gKeyframeR1;
		lNormal = vec3(cos(curAngleOnC1), sin(curAngleOnC1), 0);
	}
	else if(posOnCapsule < kf.gKeyframeThreshold1)
	{
		float t = (posOnCapsule - kf.gKeyframeThreshold0) / kf.gKeyframeThreshold0to1;
		lPos.xy = kf.gKeyframeTopRightPos*(1-t) + t*kf.gKeyframeTopLeftPos;

		vec2 tangentVector = kf.gKeyframeTopTangentVector;
		lNormal = vec3(tangentVector.y, -tangentVector.x, 0);
	}
	else if(posOnCapsule < kf.gKeyframeThreshold2)
	{
		float curAngleOnC0 = kf.gKeyframeTheta + ((posOnCapsule-kf.gKeyframeThreshold1) / kf.gKeyframeR0);
		lPos.x = -kf.gKeyframeHalfDist + cos(curAngleOnC0) * kf.gKeyframeR0;
		lPos.y = sin(curAngleOnC0) * kf.gKeyframeR0;
		lNormal = vec3(cos(curAngleOnC0), sin(curAngleOnC0), 0);
	}
	else if(posOnCapsule < kf.gKeyframeThreshold3)
	{
		float t = (posOnCapsule - kf.gKeyframeThreshold2) / kf.gKeyframeThreshold2to3;
		lPos.xy = kf.gKeyframeBottomLeftPos*(1-t) + t*kf.gKeyframeBottomRightPos;

		vec2 tangentVector = kf.gKeyframeBottomTangentVector;
		lNormal = vec3(tangentVector.y, -tangentVector.x, 0);
	}
	else
	{
		float curAngleOnC1 = _2pi - kf.gKeyframeTheta + ((posOnCapsule-kf.gKeyframeThreshold3) / kf.gKeyframeR1);
		lPos.x = kf.gKeyframeHalfDist + cos(curAngleOnC1) * kf.gKeyframeR1;
		lPos.y = sin(curAngleOnC1) * kf.gKeyframeR1;
		lNormal = vec3(cos(curAngleOnC1), sin(curAngleOnC1), 0);
	}

	//lPos.y += uv.y*uv.y*10;

	// Rotate waterNormal according to lNormal
	vec3 lRight = vec3(lNormal.y, -lNormal.x, 0);
	vec3 lBack = vec3(0,0,1);
	vec3 lNewNormal;

	lNewNormal = waterNormal.x * (-lRight) + waterNormal.y * lNormal + waterNormal.z * lBack;
	//varDebug = lNewNormal;
	lNormal = lNewNormal;
	
	mat4 localToWorldMtx = quatPosToMat4(kf.gKeyframeRot, kf.gKeyframeTrans);

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
