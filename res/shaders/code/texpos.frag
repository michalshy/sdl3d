#version 460

layout(location = 0) in vec2 v_texcoord;

layout(location = 0) out vec4 FragColor;

layout(set = 2, binding = 0) uniform sampler2D ourTexture;

void main()
{
    FragColor = texture(ourTexture, v_texcoord);
}
