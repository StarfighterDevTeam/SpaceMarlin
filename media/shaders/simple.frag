#version 400 core

precision highp float;
precision mediump int;

in vec4 varColor;

layout(location=0) out vec4 fragColor;

void main()
{
	fragColor = varColor;
}
