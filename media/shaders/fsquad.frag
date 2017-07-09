// OLD SHADER, DEPRECATED
// fsquad.frag

#version 400 core

precision highp float;
precision mediump int;

uniform sampler2DRect texunit;

smooth in vec2 var_texcoords;

out vec4 frag_color;

void main()
{
	frag_color = texture(texunit, var_texcoords);
}
