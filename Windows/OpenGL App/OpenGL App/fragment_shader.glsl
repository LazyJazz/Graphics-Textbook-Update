#version 450 core

/**********************************/
/*          可调参数              */

/* 软阴影采样级别 [0, 10] */
const int shadow_sample = 3;
/* 点光源亮度系数 [0, inf) */
const float point_light_brightness = 1.0;
/* 点光源衰减系数 */
const float Kq = 1.0, Kp = 0.0, Kc = 1.0;
/* 平行光源亮度 */
const vec3 parallel_light_brightness = vec3(1.0, 1.0, 1.0);
/* 环境光亮度 */
const vec3 ambient_light_brightness = vec3(0.1, 0.1, 0.1);

/*                                */
/**********************************/


in vec3 fs_norm;
in vec3 fs_pos;
in vec3 fs_color;
in float fs_flag;

out vec4 color0;

uniform sampler2D depth_map;

uniform mat4 mat_trans;
uniform mat4 mat_depth;

uniform vec3 parallel_light_direction;

uniform vec3 lights_pos[8];
uniform vec3 lights_brightness[8];

float PI = 3.14159265358979323846264338327950288419716939937510;
float INV_PI = 1.0 / PI;

float lambertian_reflect_model(vec3 vIn, vec3 vNorm, vec3 vOut) { return 1.0; }

float disney_diffuse_reflect_model(vec3 vIn, vec3 vNorm, vec3 vOut)
{
    vec3 vHalf = normalize(vIn + vOut);
    float cos_theta_d = dot(vHalf, vNorm);
    float cos_theta_i = dot(vIn, vNorm);
    float cos_theta_o = dot(vOut, vNorm);
    float FD90 = 0.5 + 2 * cos_theta_d * cos_theta_d;
    return (1.0+(FD90-1.0)*pow(1.0-cos_theta_i, 5.0))*(1.0+(FD90-1.0)*pow(1.0-cos_theta_o, 5.0));
}

float specular_reflect_model(vec3 vIn, vec3 vNorm, vec3 vOut)
{
    vec3 vHalf = normalize(vIn + vOut);
    return pow(dot(vHalf, vNorm), 100.0);
}

float mixed_reflect_model(vec3 vIn, vec3 vNorm, vec3 vOut)
{
    return 
    disney_diffuse_reflect_model(vIn, vNorm, vOut) + 
    specular_reflect_model(vIn, vNorm, vOut);
}

float CheckLightVisibility(vec4 pos)
{
    vec3 zaxis = normalize(fs_norm), xaxis, yaxis;
    xaxis = cross(zaxis, vec3(0.0, 1.0, 0.0));
    if (length(xaxis) < 0.01) xaxis = cross(zaxis, vec3(0.0, 0.0, 1.0));
    xaxis = normalize(xaxis);
    yaxis = cross(zaxis, xaxis);

    float res = 0.0;
    for (int i = -shadow_sample; i <= shadow_sample; i++)
        for (int j = -shadow_sample; j <= shadow_sample; j++)
        {
            vec4 sample_pos = mat_depth*(
                pos + 
                vec4(xaxis, 0.0) * 0.01 * i  + 
                vec4(yaxis, 0.0) * 0.01 * j);
            if (sample_pos.x < -1.0 || sample_pos.x > 1.0
            || sample_pos.y < -1.0 || sample_pos.y > 1.0
            || sample_pos.z < -1.0 || sample_pos.z > 1.0)
            {
                res += 1.0;
            }
            else
            {
                sample_pos = (sample_pos + vec4(1.0, 1.0, 1.0, 1.0)) * 0.5;
                res += ((sample_pos.z <= texture(depth_map, sample_pos.xy).z*1.001) ? 1.0 : 0.0);
            }
        }


    return res * (1.0 / pow(shadow_sample*2+1, 2));
}

vec3 point_lights(vec3 frag_pos, vec3 frag_norm)
{
    //return vec3(0.0, 0.0, 0.0);
    vec3 res = vec3(0.0, 0.0, 0.0);
    for (int i = 0; i < 8; i++)
    {
        vec3 vLight = (mat_trans*vec4(lights_pos[i], 1.0)).xyz - frag_pos;
        vec3 vIn = normalize(vLight);
        vec3 vNorm = frag_norm;
        vec3 vOut = normalize(-frag_pos);
        float cos_theta_i = max(dot(vNorm, vIn), 0.0);
        if (cos_theta_i == 0.0) continue;
        float d = length(vLight);
        res +=
        (lights_brightness[i] * point_light_brightness / (Kq*d*d + Kp*d + Kc)) *
        cos_theta_i *
        mixed_reflect_model(vIn, vNorm, vOut);
    }
    return res;
}

vec3 parallel_lights(vec3 frag_pos, vec3 frag_norm)
{
    //return vec3(0.0, 0.0, 0.0);
    vec3 vIn = normalize(-normalize(mat_trans*vec4(parallel_light_direction, 0.0)).xyz);
    vec3 vNorm = frag_norm;
    vec3 vOut = normalize(-frag_pos);
    float cos_theta_i = max(dot(vNorm, vIn), 0.0);
    if (cos_theta_i == 0.0) return vec3(0.0, 0.0, 0.0);
    return
    parallel_light_brightness *
    cos_theta_i *
    mixed_reflect_model(vIn, vNorm, vOut);
}

vec3 ambient_lights(vec3 frag_pos, vec3 frag_norm)
{
    return ambient_light_brightness;
}

vec3 lighting(vec3 frag_pos, vec3 frag_norm, vec3 frag_color)
{
    /* 用于统计被反射到出射光方向的总亮度 */
    vec3 reflected_light = vec3(0.0, 0.0, 0.0);
    /* 计算点光源贡献 */
    reflected_light += point_lights(frag_pos, frag_norm);
    /* 计算平行光源贡献 */
    reflected_light += 
        CheckLightVisibility(vec4(fs_pos, 1.0)) * 
        parallel_lights(frag_pos, frag_norm);
    /* 计算环境光贡献 */
    reflected_light += ambient_lights(frag_pos, frag_norm);
    /* 最终亮度还需要乘以反射率，以表现物体的颜色 */
    return frag_color * reflected_light;
}

void main()
{
    if (fs_flag > 0.5)
    {
        color0 = vec4(fs_color, 1.0);
    }
    else
    {
        vec3 frag_pos = (mat_trans * vec4(fs_pos, 1.0)).xyz;
        vec3 frag_norm = normalize(mat_trans * vec4(fs_norm, 0.0)).xyz;
        
        color0 = vec4(fs_color * lighting(
            frag_pos,
            frag_norm,
            vec3(1.0, 1.0, 1.0)
        ), 1.0);
    }
}