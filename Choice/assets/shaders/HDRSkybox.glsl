#source vertex
#version 450 core

layout(location = 0)in vec3 aPosition;

layout(location = 0)out vec3 vWorldPos;

from UniformBuffers.glsl include uniform Camera;

void main()
{
	vWorldPos = aPosition;
	gl_Position = (camera.Projection * camera.View * vec4(aPosition, 1.0)).xyww;
}

#source fragment
#version 450 core

layout(location = 0)out vec4 fColor;

layout(location = 0)in vec3 vWorldPos;

layout(binding = 0)uniform samplerCube hdrSkybox;

void main()
{
	vec3 envColor = texture(hdrSkybox, vWorldPos).rgb;

	// HDR tonemap and gamma correct
	envColor = envColor / (envColor + vec3(1.0));
    envColor = pow(envColor, vec3(1.0/2.2));

	fColor = vec4(envColor, 1.0);
}