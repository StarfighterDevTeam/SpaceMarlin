#version 400 core
#include "SharedDefines.h"
PROG_FRAGMENT_SHADER(PROG_TONEMAPPING)

layout(location=0) out vec4 fragColor;

in vec2 varUv;

void main()
{
	// TODO
	vec2 uv = varUv;
	//uv = ( (uv*2-1) *(1+0.1*(sin(gTime*20))) ) *0.5 + 0.5;
	fragColor = textureLod(texScene, uv, 0);
	float vignette = 1-length((uv*2-1));
	float vignetteAmount = 0.3;
	vignette = vignetteAmount*vignette + (1-vignetteAmount);
	fragColor *= vignette;
}
