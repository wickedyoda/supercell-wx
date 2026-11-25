#version 330 core

// Lower the default precision to medium
precision mediump float;

uniform sampler2D uTexture;

in float texCoord;

layout (location = 0) out vec4 fragColor;

void main()
{
   fragColor = texture(uTexture, vec2(texCoord.x, 0.5));
}
