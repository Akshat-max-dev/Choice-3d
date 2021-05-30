#source vertex
#version 450 core

layout(location = 0)in vec3 aPosition;

layout(location = 0)out vec3 vWorldPos;

from UniformBuffers.glsl include uniform Camera;

void main()
{
	vWorldPos = aPosition;
	gl_Position = camera.Projection * camera.View * vec4(aPosition, 1.0);
}

#source fragment
#version 450 core

layout(location = 0)out vec4 fColor;

layout(location = 0)in vec3 vWorldPos;

layout(binding = 0)uniform sampler2D hdrMap;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleMap(vec3 v)
{
	vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main()
{
	vec2 uv = SampleMap(normalize(vWorldPos));
	fColor = vec4(texture(hdrMap, uv).rgb, 1.0);
}