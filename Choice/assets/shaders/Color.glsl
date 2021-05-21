#source vertex
#version 450 core

layout(location = 0)in vec3 aPosition;

from UniformBuffers.glsl include uniform Camera,uniform Transform;

void main()
{
	gl_Position = uViewProjection * uTransform * vec4(aPosition, 1.0);
}

#source fragment
#version 450 core

layout(location = 0)out vec4 cResult;

from UniformBuffers.glsl include uniform Color;

void main()
{
	cResult = cColor;
}