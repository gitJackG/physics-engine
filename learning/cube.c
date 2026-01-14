#define GLEW_STATIC
#include <GL/glew.h>

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define CGLM_FORCE_RADIANS
#include <cglm/cglm.h>


/* ---------- Shader helpers ---------- */

GLuint compile_shader(GLenum type, const char* src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);

    GLint ok;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetShaderInfoLog(shader, 512, NULL, log);
        fprintf(stderr, "Shader error: %s\n", log);
        exit(1);
    }
    return shader;
}

GLuint create_program(const char* vs_src, const char* fs_src) {
    GLuint vs = compile_shader(GL_VERTEX_SHADER, vs_src);
    GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fs_src);

    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);

    GLint ok;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetProgramInfoLog(prog, 512, NULL, log);
        fprintf(stderr, "Link error: %s\n", log);
        exit(1);
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
    return prog;
}

/* ---------- Shaders ---------- */

const char* vertex_shader =
"#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"uniform mat4 uMVP;\n"
"void main() {\n"
"    gl_Position = uMVP * vec4(aPos, 1.0);\n"
"}\n";

const char* fragment_shader =
"#version 330 core\n"
"out vec4 FragColor;\n"
"void main() {\n"
"    FragColor = vec4(1.0);\n"
"}\n";

/* ---------- Main ---------- */

int main(void) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL init failed\n");
        return 1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_Window* window = SDL_CreateWindow(
        "Rotating Cube",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        800, 600,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
    );

    SDL_GLContext ctx = SDL_GL_CreateContext(window);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return 1;
    }
    SDL_GL_SetSwapInterval(1);

    glEnable(GL_DEPTH_TEST);

    /* ---------- Cube geometry ---------- */

    float vertices[] = {
        // back
        -0.5f,-0.5f,-0.5f,  0.5f,-0.5f,-0.5f,  0.5f, 0.5f,-0.5f,
        0.5f, 0.5f,-0.5f, -0.5f, 0.5f,-0.5f, -0.5f,-0.5f,-0.5f,
        // front
        -0.5f,-0.5f, 0.5f,  0.5f,-0.5f, 0.5f,  0.5f, 0.5f, 0.5f,
        0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f,-0.5f, 0.5f,
        // left
        -0.5f, 0.5f, 0.5f, -0.5f, 0.5f,-0.5f, -0.5f,-0.5f,-0.5f,
        -0.5f,-0.5f,-0.5f, -0.5f,-0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
        // right
        0.5f, 0.5f, 0.5f,  0.5f, 0.5f,-0.5f,  0.5f,-0.5f,-0.5f,
        0.5f,-0.5f,-0.5f,  0.5f,-0.5f, 0.5f,  0.5f, 0.5f, 0.5f,
        // bottom
        -0.5f,-0.5f,-0.5f,  0.5f,-0.5f,-0.5f,  0.5f,-0.5f, 0.5f,
        0.5f,-0.5f, 0.5f, -0.5f,-0.5f, 0.5f, -0.5f,-0.5f,-0.5f,
        // top
        -0.5f, 0.5f,-0.5f,  0.5f, 0.5f,-0.5f,  0.5f, 0.5f, 0.5f,
        0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f,-0.5f
    };

    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices),
                 vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    GLuint program = create_program(vertex_shader, fragment_shader);
    GLint uMVP = glGetUniformLocation(program, "uMVP");

    /* ---------- Loop ---------- */

    int running = 1;
    float angle = 0.0f;

    mat4 model, view, proj, mvp;

    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                running = 0;
        }

        angle += 0.01f;

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm_mat4_identity(model);
        glm_rotate(model, angle, (vec3){0.0f, 1.0f, 0.0f});

        glm_lookat(
            (vec3){0.0f, 0.0f, 3.0f},
                   (vec3){0.0f, 0.0f, 0.0f},
                   (vec3){0.0f, 1.0f, 0.0f},
                   view
        );

        glm_perspective(
            glm_rad(60.0f),
                        800.0f / 600.0f,
                        0.1f,
                        100.0f,
                        proj
        );

        glm_mat4_mul(proj, view, mvp);
        glm_mat4_mul(mvp, model, mvp);

        glUseProgram(program);
        glUniformMatrix4fv(uMVP, 1, GL_FALSE, (float*)mvp);

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        SDL_GL_SwapWindow(window);
    }

    SDL_GL_DeleteContext(ctx);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
