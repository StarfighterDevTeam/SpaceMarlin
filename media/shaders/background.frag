#version 400 core

precision highp float;
precision mediump int;

uniform sampler2D texPerlin;
uniform float gTime;

in vec2 varUv;

layout(location=0) out vec4 fragColor;

void main()
{
	vec4 tPerlin = texture2D(texPerlin, varUv);
	
	fragColor = vec4(tPerlin.rgb, 1);
}
