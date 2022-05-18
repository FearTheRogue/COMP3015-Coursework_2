#include "scene_project.h"

#include "imgui.h";
#include "imgui_impl_glfw.h";
#include "imgui_impl_opengl3.h";
#include <iomanip>
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <algorithm>

#include <cstdio>
#include <cstdlib>

#include <string>
using std::string;

#include <iostream>
using std::cerr;
using std::endl;

#include "helper/glutils.h"
#include <glm/gtc/matrix_transform.hpp>
#include "helper/texture.h"

using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;

Scene_Project::Scene_Project() : angle(0.0f), time(0), particleLifetime(10.5f), nParticles(1500), emitterPos(0, 0.8, 0), emitterDir(0, 1, 0)
{
    beginParticles = false;
    lightLVec = vec3(0.8f, 0.8f, 0.8f);
    lightLaVec = vec3(0.1f, 0.2f, 0.3f);
    lightPos = vec4(0.0f, 2.0f, 0.0f, 1.0f);
    fountain = ObjMesh::load("media/fountain.obj", false);
}

void Scene_Project::initScene()
{
    compile();

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    // enable alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // enable depth test
    glEnable(GL_DEPTH_TEST);

    angle = glm::half_pi<float>();

    initBuffers();

    // the particle texture
    glActiveTexture(GL_TEXTURE0);
    Texture::loadTexture("media/texture/bluewater.png");
}

void Scene_Project::initBuffers()
{
    // generate the buffers for initial velocity and birth time
    glGenBuffers(1, &initVel); // initial velocity buffer
    glGenBuffers(1, &startTime); // start time buffer

    // allocate space for all buffers
    int size = nParticles * sizeof(float);
    glBindBuffer(GL_ARRAY_BUFFER, initVel);
    glBufferData(GL_ARRAY_BUFFER, size * 3, 0, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, startTime);
    glBufferData(GL_ARRAY_BUFFER, size, 0, GL_STATIC_DRAW);

    // fill the first velocity buffer with random velocities
    mat3 emitterBasis = ParticleUtils::makeArbitraryBasis(emitterDir);
    vec3 v(0.0f);
    float velocity, theta, phi;
    std::vector<GLfloat> data(nParticles * 3);

    for (uint32_t i = 0; i < nParticles; i++)
    {
        // pick the direction of the velocity
        theta = glm::mix(0.0f, glm::pi<float>() / 20.0f, randFloat());
        phi = glm::mix(0.0f, glm::two_pi<float>(), randFloat());

        v.x = sinf(theta) * cosf(phi);
        v.y = cosf(theta);
        v.z = sinf(theta) * sinf(phi);

        // scale the set the magnitude of the velocity
        velocity = glm::mix(1.25f, 1.5f, randFloat());
        v = glm::normalize(emitterBasis * v) * velocity;

        data[3 * i] = v.x;
        data[3 * i + 1] = v.y;
        data[3 * i + 2] = v.z;
    }

    glBindBuffer(GL_ARRAY_BUFFER, initVel);
    glBufferSubData(GL_ARRAY_BUFFER, 0, size * 3, data.data()); // glBufferSubData(GL_ARRAY_BUFFER, 0, size * 3, data.data());

    // fill the start time buffer
    float rate = particleLifetime / nParticles;
    for (int i = 0; i < nParticles; i++)
    {
        data[i] = rate * i;
    }

    glBindBuffer(GL_ARRAY_BUFFER, startTime);
    glBufferSubData(GL_ARRAY_BUFFER, 0, nParticles * sizeof(float), data.data()); // glBufferSubData(GL_ARRAY_BUFFER, 0, nParticles * sizeof(float), data.data());

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenVertexArrays(1, &particles); // glGenVertexArrays(1, &particles);
    glBindVertexArray(particles);
    glBindBuffer(GL_ARRAY_BUFFER, initVel);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, startTime);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, 0); // glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    glVertexAttribDivisor(0, 1);
    glVertexAttribDivisor(1, 1);

    glBindVertexArray(0);
}

float Scene_Project::randFloat()
{
    return rand.nextFloat();
}

void Scene_Project::setMatrices(GLSLProgram& p)
{
    // model view matrix
    mat4 mv = view * model;

    p.setUniform("MV", mv);
    p.setUniform("Proj", projection);

    p.setUniform("NormalMatrix", glm::mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])));
    p.setUniform("MVP", projection * mv);
}

void Scene_Project::compile()
{
    try {
        prog.compileShader("shader/scene_lighting.vert");
        prog.compileShader("shader/scene_lighting.frag");
        prog.link();
        prog.use();

        particleProg.compileShader("shader/particle_shader.vert");
        particleProg.compileShader("shader/particle_shader.frag");
        particleProg.link();

        flatProg.compileShader("shader/flat_vert.glsl");
        flatProg.compileShader("shader/flat_frag.glsl");
        flatProg.link();
    }
    catch (GLSLProgramException& e) {
        cerr << e.what() << endl;
        exit(EXIT_FAILURE);
    }
}

void Scene_Project::update(float t)
{
    //update your angle here

    if(beginParticles)
        time = t;

    angle = std::fmod(angle + 0.01f, glm::two_pi<float>());
}

void Scene_Project::render()
{
    // we clear colour and depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // ImGui Initilisation
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Console");
    ImGui::Text("Particle System");
    ImGui::Checkbox("Begin Particles", &beginParticles);

    ImGui::Text("Light Properties");

    ImGui::Text("Light Position");
    float pos[4] = { lightPos.x ,lightPos.y, lightPos.z , lightPos.a };
    ImGui::SliderFloat4("Light Position", pos, -10.0, 10.0);
    lightPos = vec4(pos[0], pos[1], pos[2], pos[3]);

    ImGui::Text("Light L");
    float lightL[3] = { lightLVec.x, lightLVec.y, lightLVec.z };
    ImGui::SliderFloat3("Light L", lightL, 0.0, 1.0);
    lightLVec = vec3(lightL[0], lightL[1], lightL[2]);

    ImGui::Text("Light La");
    float lightLa[3] = { lightLaVec.x, lightLaVec.y, lightLaVec.z };
    ImGui::SliderFloat3("Light L", lightLa, 0.0, 1.0);
    lightLaVec = vec3(lightLa[0], lightLa[1], lightLa[2]);

    ImGui::End();

    // Render and draw ImGui to the screen
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    view = glm::lookAt(vec3(3.0f * cos(angle), 1.5f, 3.0f * sin(angle)), vec3(0.0f, 1.5f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
    projection = glm::perspective(glm::radians(60.0f), (float)width / height, 0.3f, 100.0f);
    
    // Lighting the scene
    // Render model first, so it's drawn at the bottom
    prog.use();
    prog.setUniform("Light.Position", view * lightPos); // glm::vec4(0.0f, 2.0f, 0.0f, 1.0f)
    prog.setUniform("Light.L", lightLVec); // vec3(0.8f, 0.8f, 0.8f)
    prog.setUniform("Light.La", lightLaVec); // vec3(0.1f, 0.2f, 0.3f)

    prog.setUniform("Material.Kd", 0.4f, 0.4f, 0.4f);
    prog.setUniform("Material.Ks", 0.9f, 0.9f, 0.9f);
    prog.setUniform("Material.Ka", 0.5f, 0.5f, 0.5f);
    prog.setUniform("Material.Shininess", 180.0f);

    model = mat4(1.0f);
    model = glm::translate(model, vec3(0.0f, 0.0f, 0.0f));

    setMatrices(prog);
    fountain->render();
    
    // Particle colour
    model = mat4(1.0f);
    flatProg.use();
    vec4 particleColour = vec4(0.4f, 0.4f, 0.4f, 1.0f);
    flatProg.setUniform("Colour", particleColour);

    setMatrices(flatProg);
    grid.render();

    // Particle properties
    glDepthMask(GL_FALSE);
    particleProg.use();
    particleProg.setUniform("ParticleTex", 0);
    particleProg.setUniform("ParticleLifetime", particleLifetime);
    particleProg.setUniform("ParticleSize", 0.05f);
    particleProg.setUniform("Gravity", vec3(0.0f, -0.3f, 0.0f));
    particleProg.setUniform("EmitterPos", emitterPos);
    setMatrices(particleProg);

    particleProg.setUniform("Time", time);
    glBindVertexArray(particles);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, nParticles);
    glBindVertexArray(0);
    glDepthMask(GL_TRUE);
}

void Scene_Project::resize(int w, int h)
{
    // setup the viewport and the projection matrix
    glViewport(0, 0, w, h);

    width = w;
    height = h;

    projection = glm::perspective(glm::radians(60.0f), (float)w / h, 0.3f, 100.0f);
}
