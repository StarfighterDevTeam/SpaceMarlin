// model.vert

#version 330 core

#include "../../src/SharedDefines.h"

precision highp float;
precision highp int;

layout(location=MODEL_ATTRIB_POSITIONS) in vec3 pos;
layout(location=MODEL_ATTRIB_UVS)		in vec2 uv;
layout(location=MODEL_ATTRIB_NORMALS)	in vec3 normal;

uniform mat4 gModelViewProjMtx;
//uniform mat4 gProjectionMtx;
//in vec2 vertex_texcoords;

//smooth out vec2 var_texcoords;

//layout (location=0) out vec2 varUv;
varying vec2 varUv;

void main()
{
	//var_texcoords = vertex_texcoords;
	//gl_Position = vec4(vertex_position, 0.0, 1.0);
	
	varUv = uv;
	//gl_Position = vec4(pos, 0.0, 1.0);
	gl_Position = gModelViewProjMtx * vec4(pos, 1.0);
}
