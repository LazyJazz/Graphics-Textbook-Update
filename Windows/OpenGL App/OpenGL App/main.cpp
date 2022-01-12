#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstring>
#include <cstdint>
#include <cmath>
#include <thread>
#include <random>

const double pi = 3.14159265358979323846264338327950288419716939937510;

struct Matrix
{
    float m[4][4];
    Matrix(float scale = 0.0) : Matrix(
        scale, 0.0f, 0.0f, 0.0f,
        0.0f, scale, 0.0f, 0.0f,
        0.0f, 0.0f, scale, 0.0f,
        0.0f, 0.0f, 0.0f, scale
    )
    {}
    Matrix(
        float m00, float m01, float m02, float m03,
        float m10, float m11, float m12, float m13,
        float m20, float m21, float m22, float m23,
        float m30, float m31, float m32, float m33
    )
    {
        m[0][0] = m00; m[0][1] = m01; m[0][2] = m02; m[0][3] = m03;
        m[1][0] = m10; m[1][1] = m11; m[1][2] = m12; m[1][3] = m13;
        m[2][0] = m20; m[2][1] = m21; m[2][2] = m22; m[2][3] = m23;
        m[3][0] = m30; m[3][1] = m31; m[3][2] = m32; m[3][3] = m33;
    }
    Matrix operator * (const Matrix& mat) const
    {
        Matrix res;
        for (int j = 0; j < 4; j++)
            for (int i = 0; i < 4; i++)
                for (int k = 0; k < 4; k++)
                    res.m[j][i] += m[k][i] * mat.m[j][k];
        return res;
    }
};

Matrix ProjectionMatrix(float Near, float Far, float aspect, float FOV = 0.5*pi)
{
    float vertical_scale = tan(FOV * 0.5);
    return Matrix(
        1.0f / aspect / vertical_scale, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f / vertical_scale, 0.0f, 0.0f,
        0.0f, 0.0f, Far / (Far - Near), 1.0f,
        0.0f, 0.0f, (Far * Near) / (Near - Far), 0.0f
    );
}

Matrix TranslateMatrix(float x, float y, float z)
{
    return Matrix(
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        x, y, z, 1.0
    );
}


Matrix RotationMatrix(float pitch, float yaw, float roll)
{
    return
        Matrix(
            cos(yaw), 0.0, sin(yaw), 0.0,
            0.0, 1.0, 0.0, 0.0,
            -sin(yaw), 0.0, cos(yaw), 0.0,
            0.0, 0.0, 0.0, 1.0
        ) *
        Matrix(
            1.0, 0.0, 0.0, 0.0,
            0.0, cos(pitch), sin(pitch), 0.0,
            0.0, -sin(pitch), cos(pitch), 0.0,
            0.0, 0.0, 0.0, 1.0
        ) *
        Matrix(
            cos(roll), -sin(roll), 0.0, 0.0,
            sin(roll), cos(roll), 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0,
            0.0, 0.0, 0.0, 1.0
        );
}

struct Vec3f
{
    float x, y, z;
    Vec3f() { x = y = z = 0.0f; }
    Vec3f(float _x, float _y, float _z) { x = _x; y = _y; z = _z; }
    friend Vec3f operator * (const Matrix& mat, const Vec3f& vec)
    {
        return Vec3f(
            vec.x * mat.m[0][0] + vec.y * mat.m[1][0] + vec.z * mat.m[2][0],
            vec.x * mat.m[0][1] + vec.y * mat.m[1][1] + vec.z * mat.m[2][1],
            vec.x * mat.m[0][2] + vec.y * mat.m[1][2] + vec.z * mat.m[2][2]
        );
    }
    friend Vec3f operator * (const Vec3f& vec, const Matrix& mat)
    {
        return Vec3f(
            vec.x * mat.m[0][0] + vec.y * mat.m[0][1] + vec.z * mat.m[0][2],
            vec.x * mat.m[1][0] + vec.y * mat.m[1][1] + vec.z * mat.m[1][2],
            vec.x * mat.m[2][0] + vec.y * mat.m[2][1] + vec.z * mat.m[2][2]
        );
    }
    Vec3f operator * (const float & scale) { return Vec3f(x*scale, y*scale, z*scale); }
    Vec3f operator + (const Vec3f & b) { return Vec3f(x+b.x, y+b.y, z+b.z); }
    Vec3f operator - (const Vec3f & b) { return Vec3f(x-b.x, y-b.y, z-b.z); }
};

float dot(Vec3f va, Vec3f vb) { return va.x*vb.x + va.y*vb.y + va.z*vb.z; }
Vec3f cross(Vec3f va, Vec3f vb) { return Vec3f(va.y*vb.z - va.z*vb.y, va.z*vb.x - va.x*vb.z, va.x*vb.y - va.y*vb.x); }
float length(Vec3f v) { return sqrt(v.x*v.x + v.y*v.y + v.z*v.z); }
Vec3f normalize(Vec3f v) { return v*(1.0f / length(v)); }

Matrix LookAtMatrix(Vec3f stayAt, Vec3f lookAt)
{
    Vec3f z = normalize(lookAt - stayAt), x = normalize(cross(Vec3f(0.0, 1.0, 0.0), z)), y = cross(z, x);
    return Matrix(
        x.x, x.y, x.z, 0.0,
        y.x, y.y, y.z, 0.0,
        z.x, z.y, z.z, 0.0,
        stayAt.x, stayAt.y, stayAt.z, 1.0
    );
}


Matrix fake_inverse(Matrix mat)
{
    Matrix res = Matrix(
        mat.m[0][0], mat.m[1][0], mat.m[2][0], 0.0f,
        mat.m[0][1], mat.m[1][1], mat.m[2][1], 0.0f,
        mat.m[0][2], mat.m[1][2], mat.m[2][2], 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );
    Vec3f b = res*Vec3f(-mat.m[3][0], -mat.m[3][1], -mat.m[3][2]);
    res.m[3][0] = b.x;
    res.m[3][1] = b.y;
    res.m[3][2] = b.z;
    return res;
}

struct Vertex
{
    Vec3f pos;
    Vec3f normal;
    Vec3f color;
};

struct TriInd
{
    uint32_t i0, i1, i2;
    TriInd() { i0 = i1 = i2 = 0; }
    TriInd(uint32_t _i0, uint32_t _i1, uint32_t _i2) { i0 = _i0; i1 = _i1; i2 = _i2; }
    TriInd operator + (const uint32_t offset) { return TriInd(i0+offset, i1+offset, i2+offset); }
};


const int BALL_ACCURACY = 40;

Vec3f sphere_vertices[BALL_ACCURACY * (BALL_ACCURACY - 1) + 2];
TriInd sphere_indices[(BALL_ACCURACY - 1) * BALL_ACCURACY * 2];

Vertex vertex_buffer[3*1048576];
TriInd index_buffer[1048576];
uint32_t cnt_vertex, cnt_index;

uint32_t vertex_buffer_object;
uint32_t index_buffer_object;

uint32_t shader_program_object;

uint32_t depth_map_object;
uint32_t depth_map_framebuffer_object;
uint32_t depth_shader_program_object;
const uint32_t SHADOW_WIDTH = 2048;
const uint32_t SHADOW_HEIGHT = 2048;

uint32_t CompileGLSLShaderFromFile(
    const char* shader_file_path,
    uint32_t shader_type
)
{
    FILE* file = nullptr;
#ifdef _WIN32
    fopen_s(&file, shader_file_path, "rb");
#else
    file = fopen(shader_file_path, "rb");
#endif
    if (!file) return 0;
    fseek(file, 0, SEEK_END);
    int64_t length = ftell(file);
    char* source_code = new char[length + 1];
    memset(source_code, 0, length + 1);
    fseek(file, 0, SEEK_SET);
    fread(source_code, 1, length, file);
    fclose(file);

    uint32_t shader_object = glCreateShader(shader_type);
    glShaderSource(shader_object, 1, &source_code, nullptr);
    delete[] source_code;
    glCompileShader(shader_object);

    int32_t compilation_success;
    glGetShaderiv(shader_object, GL_COMPILE_STATUS, &compilation_success);
    if (!compilation_success)
    {
        int32_t log_length;
        glGetShaderiv(shader_object, GL_INFO_LOG_LENGTH, &log_length);
        char* error_info = new char[log_length + 1];
        glGetShaderInfoLog(shader_object, log_length + 1, &log_length, error_info);
        std::cout << error_info << std::endl;
        glDeleteShader(shader_object);
        return 0;
    }
    return shader_object;
}

uint32_t LinkProgram(uint32_t vs_object, uint32_t fs_object)
{
    uint32_t program_object = glCreateProgram();
    glAttachShader(program_object, vs_object);
    glAttachShader(program_object, fs_object);
    glLinkProgram(program_object);
    int32_t link_success;
    glGetProgramiv(program_object, GL_LINK_STATUS, &link_success);
    if (!link_success)
    {
        int32_t log_length;
        glGetProgramiv(program_object, GL_INFO_LOG_LENGTH, &log_length);
        char * error_info = new char [log_length + 1];
        glGetProgramInfoLog(program_object, log_length + 1, &log_length, error_info);
        std::cout << error_info << std::endl;
        glDeleteProgram(program_object);
        delete[] error_info;
        return 0;
    }
    return program_object;
}

/* 这个函数用于初始化渲染过程中用到的资源 */
void InitAssets()
{
    sphere_vertices[0] = Vec3f(0.0f, 0.0f, 1.0f);
    sphere_vertices[1] = Vec3f(0.0f, 0.0f, -1.0f);
    for (int i = 1; i <= BALL_ACCURACY - 1; i++)
    {
        float y = i * (1.0f / BALL_ACCURACY) * pi;
        float siny = sin(y), cosy = cos(y);
        for (int j = 0; j < BALL_ACCURACY; j++)
        {
            float x = j * (2.0f / BALL_ACCURACY) * pi;
            float sinx = sin(x), cosx = cos(x);
            sphere_vertices[(i - 1) * BALL_ACCURACY + j + 2] = Vec3f(siny * sinx, siny * cosx, cosy);
        }
    }
    for (int i = 0; i < BALL_ACCURACY - 2; i++)
    {
        int next_i = i + 1;
        for (int j = 0; j < BALL_ACCURACY; j++)
        {
            int next_j = (j + 1) % BALL_ACCURACY;
            sphere_indices[i * BALL_ACCURACY * 2 + j * 2] = TriInd(i * BALL_ACCURACY + j + 2, next_i * BALL_ACCURACY + j + 2, i * BALL_ACCURACY + next_j + 2);
            sphere_indices[i * BALL_ACCURACY * 2 + j * 2 + 1] = TriInd(next_i * BALL_ACCURACY + j + 2, next_i * BALL_ACCURACY + next_j + 2, i * BALL_ACCURACY + next_j + 2);
        }
    }
    for (int j = 0; j < BALL_ACCURACY; j++)
    {
        int next_j = (j + 1) % BALL_ACCURACY;
        sphere_indices[(BALL_ACCURACY - 2) * BALL_ACCURACY * 2 + j * 2] = TriInd((BALL_ACCURACY - 2) * BALL_ACCURACY + next_j + 2, (BALL_ACCURACY - 2) * BALL_ACCURACY + j + 2, 1);
        sphere_indices[(BALL_ACCURACY - 2) * BALL_ACCURACY * 2 + j * 2 + 1] = TriInd(0, j + 2, next_j + 2);
    }

    glCreateBuffers(1, &vertex_buffer_object);
    glNamedBufferData(vertex_buffer_object, sizeof(vertex_buffer), nullptr, GL_DYNAMIC_DRAW);

    glCreateBuffers(1, &index_buffer_object);
    glNamedBufferData(index_buffer_object, sizeof(index_buffer), nullptr, GL_DYNAMIC_DRAW);


    uint32_t vertex_shader_object = CompileGLSLShaderFromFile("vertex_shader.glsl", GL_VERTEX_SHADER);
    uint32_t fragment_shader_object = CompileGLSLShaderFromFile("fragment_shader.glsl", GL_FRAGMENT_SHADER);

    shader_program_object = LinkProgram(vertex_shader_object, fragment_shader_object);
    glDeleteShader(vertex_shader_object);
    glDeleteShader(fragment_shader_object);

    vertex_shader_object = CompileGLSLShaderFromFile("depth_vertex_shader.glsl", GL_VERTEX_SHADER);
    fragment_shader_object = CompileGLSLShaderFromFile("depth_fragment_shader.glsl", GL_FRAGMENT_SHADER);
    depth_shader_program_object = LinkProgram(vertex_shader_object, fragment_shader_object);
    glDeleteShader(vertex_shader_object);
    glDeleteShader(fragment_shader_object);

    glCreateTextures(GL_TEXTURE_2D, 1, &depth_map_object);
    glBindTexture(GL_TEXTURE_2D, depth_map_object);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glBindTexture(GL_TEXTURE_2D, 0);

    glCreateFramebuffers(1, &depth_map_framebuffer_object);
    glBindFramebuffer(GL_FRAMEBUFFER, depth_map_framebuffer_object);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_map_object, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    Matrix depth_map_mat_trans = Matrix(
        0.12, 0.0, 0.0, 0.0,
        0.0, 0.12, 0.0, 0.0,
        0.0, 0.0, 1.0 / 99.0, 0.0,
        0.0, 0.0,-1.0 / 99.0, 1.0
    ) * fake_inverse(LookAtMatrix(Vec3f(30.0, 20.0, -10.0), Vec3f(0.0, 0.0, 0.0)));
    glProgramUniformMatrix4fv(depth_shader_program_object, glGetUniformLocation(depth_shader_program_object, "mat_trans"), 1, false, (float*)&depth_map_mat_trans);
    glProgramUniformMatrix4fv(shader_program_object, glGetUniformLocation(shader_program_object, "mat_depth"), 1, false, (float*)&depth_map_mat_trans);
}

void ResetScene()
{
    cnt_index = 0;
    cnt_vertex = 0;
}

void LoadTriangle(Vec3f v0, Vec3f v1, Vec3f v2, Vec3f color)
{
    Vec3f normal = normalize(cross(v1 - v0, v2 - v0));
    vertex_buffer[cnt_vertex].pos = v0;
    vertex_buffer[cnt_vertex].normal = normal;
    vertex_buffer[cnt_vertex].color = color;
    vertex_buffer[cnt_vertex + 1].pos = v1;
    vertex_buffer[cnt_vertex + 1].normal = normal;
    vertex_buffer[cnt_vertex + 1].color = color;
    vertex_buffer[cnt_vertex + 2].pos = v2;
    vertex_buffer[cnt_vertex + 2].normal = normal;
    vertex_buffer[cnt_vertex + 2].color = color;
    index_buffer[cnt_index] = TriInd(cnt_vertex, cnt_vertex + 2, cnt_vertex + 1);
    cnt_vertex += 3;
    cnt_index += 1;
}

void LoadSphere(Vec3f origin, float radius, Vec3f color)
{
    for (int i = 0; i < BALL_ACCURACY*(BALL_ACCURACY - 1)+2; i++)
    {
        vertex_buffer[i+cnt_vertex].pos = sphere_vertices[i] * radius + origin;
        vertex_buffer[i+cnt_vertex].normal = sphere_vertices[i];
        vertex_buffer[i+cnt_vertex].color = color;
    }
    for (int i = 0; i < BALL_ACCURACY * (BALL_ACCURACY - 1) *2; i++)
        index_buffer[i + cnt_index] = sphere_indices[i] + cnt_vertex;
    cnt_vertex += BALL_ACCURACY * (BALL_ACCURACY - 1) + 2;
    cnt_index += BALL_ACCURACY * (BALL_ACCURACY - 1) * 2;
}

Vec3f balls_pos[64];
Vec3f balls_velocity[64];
Vec3f balls_color[64];
const float ball_radius = 0.8;
const float absorb_scale = 0.8;

void InitBalls()
{
    std::mt19937 rd(2022);
    std::uniform_real_distribution<float> d(-1.0, 1.0);
    std::uniform_real_distribution<float> d_color(0.0, 1.0);
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            for (int  k = 0; k < 4; k++)
            {
                balls_pos[i*16 + j*4 + k] = Vec3f(
                    -3.0 + i * 2.0,
                    -3.0 + j * 2.0,
                    -3.0 + k * 2.0
                );
                balls_velocity[i*16 + j*4 + k] = Vec3f(d(rd), d(rd), d(rd));
                balls_color[i*16 + j*4 + k] = Vec3f(d_color(rd), d_color(rd), d_color(rd));
            }
}

void BallCollision(int i, int j)
{
    Vec3f direction = normalize(balls_pos[j] - balls_pos[i]);
    Vec3f relative_velocity = balls_velocity[i] - balls_velocity[j];
    Vec3f impulse = direction * fmaxf(dot(direction, relative_velocity), 0.0f) * (0.5 + 0.5*absorb_scale);
    balls_velocity[j] = balls_velocity[j] + impulse;
    balls_velocity[i] = balls_velocity[i] - impulse;
}

void UpdateBalls(float time_step)
{
    for (int i = 0; i < 64; i++)
        balls_velocity[i].y -= 9.8*time_step;
    for (int t = 0; t < 5; t++)
    {
        for (int i = 0; i < 64; i++)
            for (int j = i + 1; j < 64; j++)
                if (length(balls_pos[i] - balls_pos[j]) <= ball_radius * 2.0)
                    BallCollision(i, j);
        for (int i = 0; i < 64; i++)
        {
            if (balls_pos[i].x > 5.0 - ball_radius) balls_velocity[i].x = fminf(balls_velocity[i].x, -balls_velocity[i].x * absorb_scale);
            if (balls_pos[i].x < -5.0 + ball_radius) balls_velocity[i].x = fmaxf(balls_velocity[i].x, -balls_velocity[i].x * absorb_scale);
            if (balls_pos[i].y > 5.0 - ball_radius) balls_velocity[i].y = fminf(balls_velocity[i].y, -balls_velocity[i].y * absorb_scale);
            if (balls_pos[i].y < -5.0 + ball_radius) balls_velocity[i].y = fmaxf(balls_velocity[i].y, -balls_velocity[i].y * absorb_scale);
            if (balls_pos[i].z > 5.0 - ball_radius) balls_velocity[i].z = fminf(balls_velocity[i].z, -balls_velocity[i].z * absorb_scale);
            if (balls_pos[i].z < -5.0 + ball_radius) balls_velocity[i].z = fmaxf(balls_velocity[i].z, -balls_velocity[i].z * absorb_scale);
        }
    }
    for (int i = 0; i < 64; i++)
        balls_pos[i] = balls_pos[i] + balls_velocity[i] * time_step;
}

void LoadScene()
{
    ResetScene();

    LoadTriangle(Vec3f(5.0, -5.0, 5.0), Vec3f(-5.0, -5.0, -5.0), Vec3f(-5.0, -5.0, 5.0), Vec3f(0.7, 0.7, 1.0));
    LoadTriangle(Vec3f(5.0, -5.0, 5.0), Vec3f(5.0, -5.0, -5.0), Vec3f(-5.0, -5.0, -5.0), Vec3f(0.7, 0.7, 1.0));
    LoadTriangle(Vec3f(5.0, 5.0, 5.0), Vec3f(-5.0, -5.0, 5.0), Vec3f(-5.0, 5.0, 5.0), Vec3f(0.7, 1.0, 0.7));
    LoadTriangle(Vec3f(5.0, 5.0, 5.0), Vec3f(5.0, -5.0, 5.0), Vec3f(-5.0, -5.0, 5.0), Vec3f(0.7, 1.0, 0.7));
    LoadTriangle(Vec3f(-5.0, 5.0, 5.0), Vec3f(-5.0, -5.0, -5.0), Vec3f(-5.0, 5.0, -5.0), Vec3f(1.0, 0.7, 0.7));
    LoadTriangle(Vec3f(-5.0, 5.0, 5.0), Vec3f(-5.0, -5.0, 5.0), Vec3f(-5.0, -5.0, -5.0), Vec3f(1.0, 0.7, 0.7));

    LoadTriangle(Vec3f(-5.0, 5.0, -5.0), Vec3f(5.0, 5.0, 5.0), Vec3f(-5.0, 5.0, 5.0), Vec3f(0.7, 0.7, 1.0));
    LoadTriangle(Vec3f(5.0, 5.0, -5.0), Vec3f(5.0, 5.0, 5.0), Vec3f(-5.0, 5.0, -5.0), Vec3f(0.7, 0.7, 1.0));
    LoadTriangle(Vec3f(5.0, 5.0, -5.0), Vec3f(-5.0, 5.0, -5.0), Vec3f(-5.0, -5.0, -5.0), Vec3f(0.7, 1.0, 0.7));
    LoadTriangle(Vec3f(5.0, 5.0, -5.0), Vec3f(-5.0, -5.0, -5.0), Vec3f(5.0, -5.0, -5.0), Vec3f(0.7, 1.0, 0.7));
    LoadTriangle(Vec3f(5.0, -5.0, -5.0), Vec3f(5.0, 5.0, 5.0), Vec3f(5.0, 5.0, -5.0), Vec3f(1.0, 0.7, 0.7));
    LoadTriangle(Vec3f(5.0, -5.0, 5.0), Vec3f(5.0, 5.0, 5.0), Vec3f(5.0, -5.0, -5.0), Vec3f(1.0, 0.7, 0.7));

    for (int i = 0; i < 64; i++)
        LoadSphere(balls_pos[i], ball_radius, balls_color[i]);
    
    glNamedBufferSubData(vertex_buffer_object, 0, sizeof(Vertex) * cnt_vertex, vertex_buffer);
    glNamedBufferSubData(index_buffer_object, 0, sizeof(TriInd) * cnt_index, index_buffer);
}

void Print(Matrix mat)
{
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            printf("%.3f%c", mat.m[i][j], (j == 3) ? '\n' : '\t');
}

int main(void)
{
    GLFWwindow* window;

    /* 初始化 GLFW 库 */
    if (!glfwInit())
        return -1;

    /* 创建窗口 */
    glfwWindowHint(GLFW_SAMPLES, 4);
    window = glfwCreateWindow(768, 768, "Lighting", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* 将窗口设置为当前上下文 */
    glfwMakeContextCurrent(window);

    /* 初始化 GLAD 库，在这一步之后才能调用 glXXX 函数 */
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        glfwTerminate();
        return -1;
    }

    /* 显示设备信息以及 GL 版本 */
    std::cout << "GPU:         " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "GPU Vendor:  " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "GL Version:  " << glGetString(GL_VERSION) << std::endl;

    /* 调用初始化函数 */

    InitAssets();

    int32_t mat_proj_location, mat_trans_location;
    mat_proj_location = glGetUniformLocation(shader_program_object, "mat_proj");
    mat_trans_location = glGetUniformLocation(shader_program_object, "mat_trans");

    glfwSwapInterval(0);

    Matrix CameraRotation = RotationMatrix(0.19*pi, 0.225*pi, 0.0);
    Vec3f CameraTranslation = Vec3f(9.0, 9.0f, -11.0f);

    InitBalls();

    double camera_pitch = 0.19*pi, camera_yaw = 0.225*pi;
    double last_x = 0.0, last_y = 0.0;
    int last_click = 0;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    std::chrono::steady_clock::time_point tp = std::chrono::steady_clock::now();

    for (int i = 0; i < 1000; i++)
        UpdateBalls(0.002);

    /* 消息循环 */
    while (!glfwWindowShouldClose(window))
    {
        std::chrono::steady_clock::time_point this_tp = std::chrono::steady_clock::now();
        std::cout << "Last frame time used: " << (this_tp - tp) / std::chrono::milliseconds(1) << "ms\n";
        tp = this_tp;

        CameraRotation = RotationMatrix(camera_pitch, camera_yaw, 0.0);
        LoadScene();


        /* 在这里实现渲染代码 */

        glBindFramebuffer(GL_FRAMEBUFFER, depth_map_framebuffer_object);
        glClear(GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glUseProgram(depth_shader_program_object);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_object);
        glVertexAttribPointer(0, 3, GL_FLOAT, false, 36, (void*)0);
        glEnableVertexAttribArray(0);
        glDrawElements(GL_TRIANGLES, cnt_index * 3, GL_UNSIGNED_INT, nullptr);
        glDisableVertexAttribArray(0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        glClear(GL_DEPTH_BUFFER_BIT);

        int32_t width, height;
        glfwGetWindowSize(window, &width, &height);
        glViewport(0, 0, width, height);
        glUseProgram(shader_program_object);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_object);
        glVertexAttribPointer(0, 3, GL_FLOAT, false, 36, (void*)0);
        glVertexAttribPointer(1, 3, GL_FLOAT, false, 36, (void*)12);
        glVertexAttribPointer(2, 3, GL_FLOAT, false, 36, (void*)24);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);

        Matrix mat_proj, mat_trans;
        mat_proj = ProjectionMatrix(1.0f, 100.0f, (float)width / (float)height, pi / 3.0);
        mat_trans = fake_inverse(
            TranslateMatrix(CameraTranslation.x, CameraTranslation.y, CameraTranslation.z) * CameraRotation
        );
        glUniformMatrix4fv(mat_proj_location, 1, 0, (float*)&mat_proj);
        glUniformMatrix4fv(mat_trans_location, 1, 0, (float*)&mat_trans);

        glBindTexture(GL_TEXTURE_2D, depth_map_object);
        glDrawElements(GL_TRIANGLES, cnt_index * 3, GL_UNSIGNED_INT, nullptr);

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);

        /* 交换缓冲 */
        glfwSwapBuffers(window);

        /* 处理窗口消息 */
        glfwPollEvents();


        /* 更新帧资源 */
        for (int i = 0; i < 10; i++)
            UpdateBalls(0.002);

        const float move_speed = 0.05;
        if (glfwGetKey(window, GLFW_KEY_W)) CameraTranslation = CameraTranslation + Vec3f(CameraRotation.m[2][0], CameraRotation.m[2][1], CameraRotation.m[2][2]) * move_speed;
        if (glfwGetKey(window, GLFW_KEY_S)) CameraTranslation = CameraTranslation - Vec3f(CameraRotation.m[2][0], CameraRotation.m[2][1], CameraRotation.m[2][2]) * move_speed;
        if (glfwGetKey(window, GLFW_KEY_D)) CameraTranslation = CameraTranslation + Vec3f(CameraRotation.m[0][0], CameraRotation.m[0][1], CameraRotation.m[0][2]) * move_speed;
        if (glfwGetKey(window, GLFW_KEY_A)) CameraTranslation = CameraTranslation - Vec3f(CameraRotation.m[0][0], CameraRotation.m[0][1], CameraRotation.m[0][2]) * move_speed;
        if (glfwGetKey(window, GLFW_KEY_R)) CameraTranslation = CameraTranslation + Vec3f(CameraRotation.m[1][0], CameraRotation.m[1][1], CameraRotation.m[1][2]) * move_speed;
        if (glfwGetKey(window, GLFW_KEY_F)) CameraTranslation = CameraTranslation - Vec3f(CameraRotation.m[1][0], CameraRotation.m[1][1], CameraRotation.m[1][2]) * move_speed;


        {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            int left_click = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
            if (left_click)
            {
                camera_pitch += (ypos - last_y) * 0.002;
                camera_yaw -= (xpos - last_x) * 0.002;
            }
            last_x = xpos;
            last_y = ypos;
        }
    }

    glfwTerminate();
    return 0;
}