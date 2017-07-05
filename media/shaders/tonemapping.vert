#version 400 core

#include "../../src/SharedDefines.h"

precision highp float;
precision highp int;

layout(location=PROG_FULLSCREENTRIANGLE_ATTRIB_POSITIONS)	in vec2 pos;
layout(location=PROG_FULLSCREENTRIANGLE_ATTRIB_UVS)		in vec2 uv;

out vec2 varUv;

void main()
{
	varUv = uv;
	gl_Position = vec4(pos, 0, 1);
}
