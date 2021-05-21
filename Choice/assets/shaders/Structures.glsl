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
	vec3 Color;
	mat4 LightVP;
};

struct PointLight
{
	vec3 Position;
	vec3 Color;
	float Radius;
};