#source vertex
#version 450 core

layout(location = 0)in vec3 aPosition;

out vec3 vWorldPos;

uniform mat4 uProjection;
uniform mat4 uView;

void main()
{
	vWorldPos = aPosition;
	gl_Position = (uProjection * uView * vec4(aPosition, 1.0)).xyww;
}

#source fragment
#version 450 core

out vec4 fColor;

in vec3 vWorldPos;

uniform samplerCube hdrSkybox;

void main()
{
	vec3 envColor = texture(hdrSkybox, vWorldPos).rgb;

	// HDR tonemap and gamma correct
	envColor = envColor / (envColor + vec3(1.0));
    envColor = pow(envColor, vec3(1.0/2.2));

	fColor = vec4(envColor, 1.0);
}