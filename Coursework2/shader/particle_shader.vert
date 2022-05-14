#version 460

layout (location = 0) in vec3 VertexInitVel; // particle initial velocity
layout (location = 1) in float VertexBirthTime; // particle birth time

uniform int Pass;

out float Transp; // transparency of the particle
out vec2 TexCoord; // texture coordinate

uniform float Time; // animation time
uniform vec3 Gravity = vec3(0.0,-0.05,0.0); // gravity vector in world coords
uniform float ParticleLifetime; // max particle lifetime
uniform float ParticleSize = 1.0; // particle size
uniform vec3 EmitterPos; // emitter position in world coordinates

uniform mat4 MV;
uniform mat4 Proj;

// offsets to the position in camera coordinates for each vertex of the particles quads
const vec3 offsets[] = vec3[](vec3(-0.5,-0.5,0),vec3(0.5,-0.5,0),vec3(0.5,0.5,0),
							  vec3(-0.5,-0.5,0),vec3(0.5,0.5,0),vec3(-0.5,0.5,0));

// texture coordinates for each vertex of the particles quad
const vec2 texCoords[] = vec2[](vec2(0,0),vec2(1,0),vec2(1,1),vec2(0,0),vec2(1,1),vec2(0,1));

void main()
{
	vec3 cameraPos; // position in camera coordinates
	float t = Time - VertexBirthTime;
	if(t>=0 && t<ParticleLifetime)
	{
		vec3 pos = EmitterPos + VertexInitVel * t + Gravity * t * t;

		// offset the vertex based on the ID
		cameraPos = (MV * vec4(pos,1)).xyz + (offsets[gl_VertexID] * ParticleSize);
		Transp = mix(1,0,t/ParticleLifetime);
	} else {
		// particle doesn't "exist", draw fully transparent
		cameraPos = vec3(0);
		Transp = 0.0;
	}

	TexCoord = texCoords[gl_VertexID];

	// the position in clip coordinates
	gl_Position = Proj * vec4(cameraPos,1);
}
