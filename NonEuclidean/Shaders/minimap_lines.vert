#version 450

// Globals
layout (location = 0) in vec2 position;

void main(void) {
	gl_Position = vec4(position, 0.0f, 1.0f);
}
