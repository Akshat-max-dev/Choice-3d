#source vertex
#version 450 core

layout(location = 0)in vec3 aPosition;

out vec3 vWorldPos;

uniform mat4 uProjection;
uniform mat4 uView;

void main()
{
	vWorldPos = aPosition;
	gl_Position = uProjection * uView * vec4(aPosition, 1.0);
}

#source fragment
#version 450 core

out vec4 fColor;

in vec3 vWorldPos;

uniform sampler2D hdrMap;

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