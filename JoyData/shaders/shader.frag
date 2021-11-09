#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D texSampler;
layout(binding = 1) uniform Color{
	vec4 data;
} color;

layout(binding = 2) uniform Color{
	vec4 data;
} color2;

void main() {
    outColor = texture(texSampler, fragTexCoord) * color.data;
}