#version 400 core

precision highp float;
precision mediump int;

uniform sampler2D texScene;

layout(location=0) out vec4 fragColor;

in vec2 varUv;

void main()
{
	// TODO
	fragColor = textureLod(texScene, varUv, 0);
}
