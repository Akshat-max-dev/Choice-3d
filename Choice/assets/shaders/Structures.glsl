//Geometry Pass Vertex Output
struct GPVertexOutput 
{
    vec3 FragPos;
    vec2 TexCoords;
    vec3 Normal;
};

struct DirectionalLight
{
	vec3 Direction;
	float _padding1;
	vec3 Color;
	float _padding2;
	mat4 LightVP;
};

struct PointLight
{
	vec3 Position;
	float _padding1;
	vec3 Color;
	float _padding2;
	float Radius;
};