#source vertex
#version 450 core

layout(location = 0)in vec3 aPosition;

uniform mat4 uViewProjection;
uniform mat4 uTransform;

void main()
{
	gl_Position = uViewProjection * uTransform * vec4(aPosition, 1.0);
}

#source fragment
#version 450 core

out vec4 oColor;

void main()
{
	oColor = vec4(1.0, 1.0, 0.0, 1.0);
}