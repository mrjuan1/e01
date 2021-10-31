#version 310 es

precision lowp float;

layout(location = 1) uniform vec4 colour;

layout(location = 2) uniform bool useTexture;
layout(location = 3) uniform sampler2D texSampler;

in vec2 vTexCoords;

layout(location = 0) out vec4 outColour;

void main() {
	outColour = colour;

	if(useTexture)
		outColour *= texture(texSampler, vTexCoords);

	if(outColour.a == 0.0f)
		discard;
}
