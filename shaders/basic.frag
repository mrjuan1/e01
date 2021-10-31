#version 310 es

precision lowp int;
precision lowp float;
precision lowp sampler2DMS;

layout(location = 0) uniform vec2 size;
layout(location = 1) uniform int samples;

layout(location = 2) uniform sampler2DMS colourSampler;
layout(location = 3) uniform sampler2DMS depthSampler;

in vec2 texCoords;

layout(location = 0) out vec4 outColour;
layout(location = 1) out vec4 outDepth;

void main() {
	outColour = outDepth = vec4(0.0f);
	ivec2 position = ivec2(texCoords * size);

	for(int i = 0; i < samples; i++) {
		outColour += texelFetch(colourSampler, position, i);
		outDepth += texelFetch(depthSampler, position, i);
	}

	float samplesFloat = float(samples);
	outColour /= samplesFloat;
	outDepth /= samplesFloat;
}
