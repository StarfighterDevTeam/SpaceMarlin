#version 400 core

precision highp float;
precision mediump int;

uniform samplerCube texSky;

in vec3 varUv;

layout(location=0) out vec4 fragColor;

void main()
{
	fragColor = texture(texSky, varUv);
}
