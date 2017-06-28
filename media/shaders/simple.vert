#version 400 core

#include "../../src/SharedDefines.h"

precision highp float;
precision highp int;

layout(location=PROG_SIMPLE_ATTRIB_POSITIONS)	in vec3 pos;
layout(location=PROG_SIMPLE_ATTRIB_COLORS)		in vec4 color;

uniform mat4 gModelViewProjMtx;

out vec4 varColor;

void main()
{
	varColor = color;
	gl_Position = gModelViewProjMtx * vec4(pos, 1.0);
}
