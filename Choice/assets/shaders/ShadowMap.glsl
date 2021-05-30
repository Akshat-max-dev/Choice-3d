#source vertex
#version 450 core

layout(location = 0)in vec3 aPosition;

from UniformBuffers.glsl include uniform Camera,uniform Transform;

void main()
{
	//ViewProjection -> LightViewProjection
	gl_Position = camera.Projection * camera.View * uTransform * vec4(aPosition, 1.0);
}

#source fragment
#version 450 core

void main()
{
}