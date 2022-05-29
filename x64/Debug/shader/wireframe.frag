#version 460

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

// line struct for rendering the wireframe

uniform struct LineInfo {
	float Width;
	vec4 Colour;
} Line;

// in variables coming from geometry shader
in vec3 GPosition;
in vec3 GNormal;
noperspective in vec3 GEdgeDistance;

// output of the fragment shader
layout (location = 0) out vec4 FragColor;

vec3 phongModel(vec3 position, vec3 normal) 
{
    // ambient 
    vec3 ambient = Material.Kd * Light.Intensity;

    // diffuse
    vec3 s = vec3(0.0);

    if(Light.Position.w == 0)
    {
        s = Light.Position.xyz; // directional light
    } 
    else 
    {
        s = normalize(vec3(Light.Position) - position); // point light
    }

    float sDotN = max(dot(s, normal), 0.0);
    vec3 diffuse = Material.Kd * sDotN;

    // specular
    vec3 spec = vec3(0.0);

    if(sDotN > 0.0) {
        vec3 v = normalize(-position.xyz);
        vec3 r = reflect(-s, normal);
        spec = Material.Ks * pow(max(dot(r,v), 0.0), Material.Shininess);
    }

    return ambient + Light.Intensity * (diffuse + spec);
}

void main()
{
	// calculate the colour
	vec4 color = vec4(phongModel(GPosition, GNormal),1.0);

	// find the smallest distance for the fragment
	float d = min(GEdgeDistance.x, GEdgeDistance.y);
	d = min(d,GEdgeDistance.z);

	float mixVal;
	
	if(d < Line.Width - 1) 
	{
		mixVal = 1.0;
	}
	else if (d > Line.Width + 1)
	{
		mixVal = 0.0;
	}
	else
	{
		float x = d - (Line.Width - 1);
		mixVal = exp2(-2.0 * (x*x));
	}

	FragColor = mix(color, Line.Colour, mixVal);
}