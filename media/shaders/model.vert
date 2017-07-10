#version 400 core
#include "SharedDefines.h"
PROG_VERTEX_SHADER(PROG_MODEL)

out vec2 varUv;

void main()
{
	vec3 lPos = pos;
	float enveloppe = (lPos.x+2);
	//enveloppe = enveloppe < 0 ? 0 : enveloppe;
	lPos.y += enveloppe * sin(15*gTime + lPos.x)*0.05;

	varUv = uv;
	gl_Position = gLocalToProjMtx * vec4(lPos, 1.0);
}
