#version 400 core

precision highp float;
precision mediump int;

uniform sampler2D texPerlin;
uniform mat4 gModelViewProjMtx;
uniform mat4 gProjToWorldMtx;
uniform float gAspectRatio;
uniform float gTime;

in vec3 varPos;

layout(location=0) out vec4 fragColor;

void main()
{
	//vec4 tPerlin = texture2D(texPerlin, varUv);
	vec2 uv = varPos.xy * 0.5 + 0.5;
	vec4 worldSpacePos = gProjToWorldMtx * vec4(varPos.xy, 1, 1);

	// TODO...
	vec4 tPerlin = texture2D(texPerlin, worldSpacePos.xy);
	//fragColor = worldSpacePos;
	
	fragColor = vec4(tPerlin.rgb, 1);
	//fragColor = vec4(gAspectRatio,0,0,1);
	//fragColor = vec4(gAspectRatio,0,0,1);
}
