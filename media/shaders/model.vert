#version 400 core

#include "../../src/SharedDefines.h"

precision highp float;
precision highp int;

layout(location=PROG_MODEL_ATTRIB_POSITIONS)	in vec3 pos;
layout(location=PROG_MODEL_ATTRIB_UVS)			in vec2 uv;
layout(location=PROG_MODEL_ATTRIB_NORMALS)		in vec3 normal;

uniform mat4 gModelViewProjMtx;

out vec2 varUv;

void main()
{
	varUv = uv;
	gl_Position = gModelViewProjMtx * vec4(pos, 1.0);
}
