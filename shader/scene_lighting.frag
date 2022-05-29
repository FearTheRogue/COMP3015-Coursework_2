#version 460

layout (location = 0) out vec4 FragColor;

in vec3 Position;
in vec3 Normal;

uniform struct LightInfo {
    vec4 Position; // Light position in eye coords.
    vec3 La; // Ambient light intensity
    vec3 L; // Diffuse and specular light intensity
} Light;

uniform struct MaterialInfo {
    vec3 Ka; // Ambient reflectivity
    vec3 Kd; // Diffuse reflectivity
    vec3 Ks; // Specular reflectivity
    float Shininess; // Specular shininess factor
} Material;

vec3 blinnPhong(vec3 pos, vec3 norm)
{
    vec3 ambient = Material.Ka * Light.La;
    
    vec3 s = vec3(0.0);

    if(Light.Position.w == 0)
    {
        s = Light.Position.xyz; // directional light
    } 
    else 
    {
        s = normalize(vec3(Light.Position) - pos); // point light
    }

    float sDotN = max(dot(s, norm), 0.0);
    vec3 diffuse = Material.Kd * sDotN;

    vec3 spec = vec3(0.0);

    if (sDotN > 0.0) {
        vec3 v = normalize(-pos.xyz);
        vec3 h = normalize(v + s);
        spec = Material.Ks * pow(max(dot(h, norm), 0.0), Material.Shininess);
    }

    return ambient + Light.L * (diffuse + spec);
}

void main()
{
    // Blinn Phong Shading
    FragColor = vec4(blinnPhong(Position, normalize(Normal)), 1);
}