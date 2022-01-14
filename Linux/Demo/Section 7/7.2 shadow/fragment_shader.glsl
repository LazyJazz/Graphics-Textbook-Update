#version 450 core

const int shadow_sample = 8;

in vec4 fs_norm;
in vec4 fs_pos;
in vec3 fs_color;
in float fs_flag;

out vec4 color0;

uniform sampler2D depth_map;

uniform mat4 mat_trans;
uniform mat4 mat_depth;

uniform vec3 v_light_direct;

uniform vec3 lights_pos[8];
uniform vec3 lights_brightness[8];

float PI = 3.14159265358979323846264338327950288419716939937510;
float INV_PI = 1.0 / PI;

float lambertian(vec3 vNorm, vec3 vIn, vec3 vOut) { return 1.0; }

float disney_diffuse(vec3 vNorm, vec3 vIn, vec3 vOut)
{
    vec3 vHalf = normalize(vIn + vOut);
    float cos_theta_d = dot(vHalf, vNorm);
    float cos_theta_i = dot(vIn, vNorm);
    float cos_theta_o = dot(vOut, vNorm);
    float FD90 = 0.5 + 2 * cos_theta_d * cos_theta_d;
    return (1.0+(FD90-1.0)*pow(1.0-cos_theta_i, 5.0))*(1.0+(FD90-1.0)*pow(1.0-cos_theta_o, 5.0));
}

float blinn_phong(vec3 vNorm, vec3 vIn, vec3 vOut)
{
    vec3 vHalf = normalize(vIn + vOut);
    return pow(dot(vHalf, vNorm), 100.0);
}

float mixed(vec3 vNorm, vec3 vIn, vec3 vOut)
{
    return lambertian(vNorm, vIn, vOut) + blinn_phong(vNorm, vIn, vOut);
}

vec3 point_light(vec3 light_pos, vec3 light_color, vec4 frag_pos, vec4 frag_norm)
{
    float d = distance(vec4(light_pos, 1.0), frag_pos);
    float costheta = max(dot(frag_norm, normalize(vec4(light_pos, 1.0) - frag_pos)), 0.0);
    if (costheta == 0.0) return vec3(0.0, 0.0, 0.0);
    return 
    costheta
    * mixed(frag_norm.xyz,normalize(light_pos-frag_pos.xyz),normalize(-frag_pos.xyz))
    * light_color / (d * d + 1.0);
}

vec3 parallel_light(vec3 light_direction, vec3 light_color, vec4 frag_pos, vec4 frag_norm)
{
    float costheta = max(dot(frag_norm.xyz, -light_direction), 0.0);
    if (costheta == 0.0) return vec3(0.0, 0.0, 0.0);
    return 
    costheta
    *
    mixed(frag_norm.xyz, -light_direction, normalize(-frag_pos.xyz))
    * light_color;
}

float CheckLightVisibility(vec4 pos)
{
    pos = mat_depth*pos;
    if (pos.x < -1.0 || pos.x > 1.0) return 1.0;
    if (pos.y < -1.0 || pos.y > 1.0) return 1.0;
    if (pos.z < -1.0 || pos.z > 1.0) return 1.0;
    pos = (pos + vec4(1.0, 1.0, 1.0, 1.0)) * 0.5;


    float res = 0.0;
    for (int i = -shadow_sample; i <= shadow_sample; i++)
        for (int j = -shadow_sample; j <= shadow_sample; j++)
            res += ((pos.z <= texture(depth_map, pos.xy + vec2( i / 4096.0, j / 4096.0)).z*1.001) ? 1.0 : 0.0);
    return res * (1.0 / pow(shadow_sample*2+1, 2));
}

void main()
{
    if (fs_flag > 0.5)
    {
        color0 = vec4(fs_color, 1.0);
    }
    else
    {
        vec3 lighting = vec3(0.0, 0.0, 0.0);
        lighting += vec3(1.0, 1.0, 1.0)*0.1;

        lighting +=
        CheckLightVisibility(fs_pos) * 
        0.9 *
        parallel_light(normalize(mat_trans*vec4(v_light_direct, 0.0)).xyz, vec3(1.0, 1.0, 1.0), mat_trans*fs_pos, normalize(mat_trans*fs_norm));

        for (int i = 0; i < 8; i++)
            lighting += 0.9 * point_light((mat_trans*vec4(lights_pos[i], 1.0)).xyz, lights_brightness[i], mat_trans*fs_pos, normalize(mat_trans*fs_norm));

        vec3 color = fs_color;
        
        color0 = vec4(color * lighting, 1.0);
    }
}