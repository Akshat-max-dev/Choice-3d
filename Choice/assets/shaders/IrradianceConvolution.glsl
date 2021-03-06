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

layout(location = 0)out vec4 icResult;

layout(location = 0)in vec3 vWorldPos;

layout(binding = 0)uniform samplerCube hdrSkybox;

const float PI = 3.14159265359;

void main()
{
	vec3 N = normalize(vWorldPos);

    vec3 irradiance = vec3(0.0);   
    
    // tangent space calculation from origin point
    vec3 up    = vec3(0.0, 1.0, 0.0);
    vec3 right = cross(up, N);
    up         = cross(N, right);
       
    float sampleDelta = 0.025;
    float nrSamples = 0.0f;
    for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            // spherical to cartesian (in tangent space)
            vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            // tangent space to world
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N; 

            irradiance += texture(hdrSkybox, sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    irradiance = PI * irradiance * (1.0 / float(nrSamples));
    
    icResult = vec4(irradiance, 1.0);
}