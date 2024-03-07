#version 330 core

out vec4 FragColor;

in vec2 uvs;
in float y;

uniform sampler2D textureGrass;
uniform sampler2D textureRock;
uniform sampler2D textureSnow;

void main() {

    vec4 grass = texture(textureGrass, uvs);
    vec4 rock = texture(textureRock, uvs);
    vec4 snow = texture(textureSnow, uvs);

    FragColor = snow*smoothstep(0.5, 0.9, y) + rock*(smoothstep(0.1, 0.5, y)*(1 - smoothstep(0.5, 0.9, y))) + grass*(1 - smoothstep(0.1, 0.5, y));

}
