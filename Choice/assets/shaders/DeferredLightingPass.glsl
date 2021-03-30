#source vertex
#version 450 core

out vec2 vTexCoords;

void main()
{
	float x = -1.0 + float((gl_VertexID & 1) << 2);
	float y = -1.0 + float((gl_VertexID & 2) << 1);
	float uvx = (x+1.0)*0.5;
	float uvy = (y+1.0)*0.5;
	vTexCoords.x = uvx;
	vTexCoords.y = uvy;
	gl_Position = vec4(x, y, 0.0, 1.0);
}

#source fragment
#version 450 core

out vec4 fResult;

in vec2 vTexCoords;

struct GBuffer
{
	sampler2D AlbedoS;
};

uniform GBuffer gBuffer;

void main()
{
	fResult = texture(gBuffer.AlbedoS, vTexCoords);
}