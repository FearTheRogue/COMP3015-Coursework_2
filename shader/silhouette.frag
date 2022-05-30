#version 460

uniform vec4 LineColour;

in vec3 GPosition;
in vec3 GNormal;

flat in int GIsEdge;

layout (location = 0) out vec4 FragColor;

struct LightInfo {
	vec4 Position; // Light position in eye coords
	vec3 Intensity; // A,D,S intensity;
};

uniform LightInfo Light;

struct MaterialInfo {
	vec3 Ka; // Ambient reflectivity
	vec3 Kd; // Diffuse reflectivity
	vec3 Ks; // Specular reflectivity
	float Shininess; // Specular shininess factor
};

uniform MaterialInfo Material;

// toon shading levels
uniform int levels;
const float scaleFactor = 1.0 / levels;

vec3 toonShade () 
{
	vec3 s = vec3(0.0);

	if(Light.Position.w == 0)
    {
        s = Light.Position.xyz; // directional light
    } 
    else 
    {
        s = normalize(Light.Position.xyz - GPosition.xyz); // point light
    }

	vec3 ambient = Material.Ka;
	float cosine = dot(s, GNormal);
	vec3 diffuse = Material.Kd * ceil(cosine * levels) * scaleFactor;

	return Light.Intensity * (ambient + diffuse);
}

void main()
{
	if (GIsEdge == 1)
		FragColor = LineColour;
	else
		FragColor = vec4(toonShade(), 1.0);
}