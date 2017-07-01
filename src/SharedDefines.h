#ifndef _SHARED_DEFINES_H
#define _SHARED_DEFINES_H

#define PROG_MODEL_ATTRIB_POSITIONS		0
#define PROG_MODEL_ATTRIB_UVS			1
#define PROG_MODEL_ATTRIB_NORMALS		2
#define PROG_MODEL_ATTRIB_BONE_INDICES	3
#define PROG_MODEL_ATTRIB_BONE_WEIGHTS	4

#define PROG_SIMPLE_ATTRIB_POSITIONS	0
#define PROG_SIMPLE_ATTRIB_COLORS		1

#define PROG_LANE_ATTRIB_POSITIONS		0

#ifdef __cplusplus
	#define U8VEC4 glm::u8vec4
#else
	#define U8VEC4 vec4
#endif

struct VtxSimple
{
	vec3	pos;
	vec4	color;
};

struct VtxModel
{
	vec3	pos;
	vec2	uv;
	vec3	normal;
	U8VEC4	boneIndices;
	U8VEC4	boneWeights;
};

struct VtxLane
{
	vec3	pos;
};

#endif // _SHARED_DEFINES_H
