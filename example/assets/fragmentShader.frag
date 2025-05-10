#version 450

layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D uSampler;
layout(set = 1, binding = 0) uniform UBO {
  float n;
} ubo;

void main() {
  vec4 sampledColor = texture(uSampler, inUV);
  outColor = vec4(mix(vec3(ubo.n), sampledColor.rgb, sampledColor.a), 1.0);
}