#version 310 es

precision lowp float;

layout(location = 0) uniform mat4 matrix;

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoords;

out vec2 vTexCoords;

void main() {
	gl_Position = matrix * vec4(position, 1.0f);
	vTexCoords = texCoords;
}
