#version 450

in vec2 coord2d;
// uniform float t;
uniform mat4 mvp;


void main(void) {
	vec4 new_coord = vec4(coord2d, 0, 1);
	new_coord = (new_coord) * mvp;
    gl_Position = new_coord;
}