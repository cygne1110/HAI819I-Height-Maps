#version 330 core

layout (location = 0) in vec3 _pos;
layout (location = 1) in vec2 _uvs;

out vec2 uvs;
out float y;

uniform mat4 mvp;
uniform sampler2D heightMap;


void main() {

    uvs = _uvs;
    y = 1 - texture(heightMap, uvs).r;
    gl_Position = mvp * vec4(_pos.x, y, _pos.z, 1.0);

}
