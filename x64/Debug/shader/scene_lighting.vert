#version 460

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;

out vec3 Position;
out vec3 Normal;

uniform mat4 RotationMatrix;
uniform mat4 ModelViewMatrix;
uniform mat3 NormalMatrix;
uniform mat4 MVP;

void getCamSpaceValues(out vec3 norm, out vec3 pos)
{
    norm = normalize(NormalMatrix * VertexNormal);
    pos = vec3(ModelViewMatrix * vec4(VertexPosition, 1.0));
}

void main()
{
    getCamSpaceValues(Normal, Position);

    gl_Position = MVP * vec4(VertexPosition,1.0);
}
