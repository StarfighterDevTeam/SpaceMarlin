#version 400 core
#include "SharedDefines.h"
PROG_FRAGMENT_SHADER(PROG_WATER_SIMULATION)

layout(location=0) out float fragHeight;

in vec2 varUv;

void main()
{
	//if(fract(gTime) < 0.01)
		fragHeight = max(0.0, -0.9 + length(1-abs((varUv*2-1))) );
	//else
	//	fragHeight = 0;

	//fragHeight =
	//	gTerm1 * texture(texHeights1, varUv).r +
	//	gTerm2 * texture(texHeights2, varUv).r +
	//	gTerm3 * (
	//		texture(texHeights1, varUv + gTexelSize * vec2(-1, 0)).r +
	//		texture(texHeights1, varUv + gTexelSize * vec2(+1, 0)).r +
	//		texture(texHeights1, varUv + gTexelSize * vec2(0, -1)).r +
	//		texture(texHeights1, varUv + gTexelSize * vec2(0, +1)).r
	//		);
	//fragHeight = varUv.x;
}
