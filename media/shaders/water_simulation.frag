#version 400 core
#include "SharedDefines.h"
PROG_FRAGMENT_SHADER(PROG_WATER_SIMULATION)

layout(location=0) out float fragHeight;

in vec2 varUv;

void main()
{
	//fragHeight = 10*max(0, -0.7 + 1-length(varUv*2-1));
	//fragHeight = 0;

	//vec2 uv = varUv;
	//uv.y += gTime*0.00001;
	//fragHeight =
	//	gTerm1 * texture(texHeights1, uv).r +
	//	gTerm2 * texture(texHeights2, uv).r +
	//	gTerm3 * (
	//		texture(texHeights1, uv + gTexelSize * vec2(-1, 0)).r +
	//		texture(texHeights1, uv + gTexelSize * vec2(+1, 0)).r +
	//		texture(texHeights1,  + gTexelSize * vec2(0, -1)).r +
	//		texture(texHeights1,  + gTexelSize * vec2(0, +1)).r
	//		);

	fragHeight = 0.1*sin(gTime*5 + varUv.y*10)*0.5;
}
