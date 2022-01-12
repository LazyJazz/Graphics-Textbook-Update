#version 450 core

in vec4 fs_norm;
in vec4 fs_pos;
in vec3 fs_color;

out vec4 color0;

uniform mat4 mat_trans;

float PI = 3.14159265358979323846264338327950288419716939937510;
float INV_PI = 1.0 / PI;

float lambertian_brdf(vec3 vNorm, vec3 vIn, vec3 vOut) { return INV_PI; }

float disney_diffuse_brdf(vec3 vNorm, vec3 vIn, vec3 vOut)
{
    vec3 vHalf = normalize(vIn + vOut);
    float cos_theta_d = dot(vHalf, vNorm);
    float cos_theta_i = dot(vIn, vNorm);
    float cos_theta_o = dot(vOut, vNorm);
    float FD90 = 0.5 + 2 * cos_theta_d * cos_theta_d;
    return INV_PI*(1.0+(FD90-1.0)*pow(1.0-cos_theta_i, 5.0))*(1.0+(FD90-1.0)*pow(1.0-cos_theta_o, 5.0));
}

float blinn_phong_brdf(vec3 vNorm, vec3 vIn, vec3 vOut)
{
    vec3 vHalf = normalize(vIn + vOut);
    return pow(dot(vHalf, vNorm), 100.0);
}

float my_surface_brdf(vec3 vNorm, vec3 vIn, vec3 vOut)
{
    return (
        disney_diffuse_brdf(vNorm, vIn, vOut) + blinn_phong_brdf(vNorm, vIn, vOut)) * 0.5;
}

vec3 point_light(vec3 light_pos, vec3 light_color, vec4 frag_pos, vec4 frag_norm)
{
    float d = distance(vec4(light_pos, 1.0), frag_pos);
    float costheta = max(dot(frag_norm, normalize(vec4(light_pos, 1.0) - frag_pos)), 0.0);
    if (costheta == 0.0) return vec3(0.0, 0.0, 0.0);
    return 
    costheta
    * disney_diffuse_brdf(frag_norm.xyz,normalize(light_pos-frag_pos.xyz),normalize(-frag_pos.xyz))
    * light_color / (4.0 * PI * d * d);
}

vec3 parallel_light(vec3 light_direction, vec3 light_color, vec4 frag_pos, vec4 frag_norm)
{
    float costheta = max(dot(frag_norm.xyz, -light_direction), 0.0);
    if (costheta == 0.0) return vec3(0.0, 0.0, 0.0);
    return 
    costheta
    * (
        disney_diffuse_brdf(frag_norm.xyz, -light_direction, normalize(-frag_pos.xyz))
        + blinn_phong_brdf(frag_norm.xyz, -light_direction, normalize(-frag_pos.xyz))
    )
    * light_color;
}

void main()
{
    vec3 lighting = vec3(0.0, 0.0, 0.0);
    lighting += vec3(0.3, 0.3, 0.3);
    lighting = lighting + 0.7 * parallel_light(normalize(mat_trans*vec4(-1.0, -1.0, 1.0, 0.0)).xyz, vec3(1.0, 1.0, 1.0) * PI, mat_trans*fs_pos, normalize(mat_trans*fs_norm));
    //lighting = lighting + point_light((mat_trans*vec4(light_pos, 1.0)).xyz, light_color, mat_trans*fs_pos, normalize(mat_trans*norm));
    vec3 color = fs_color;
    color0 = vec4(color * lighting, 1.0);
}