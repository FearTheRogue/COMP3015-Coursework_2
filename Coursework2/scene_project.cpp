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
#include "helper/particleutils.h"

using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;

Scene_Project::Scene_Project() : angle(0.0f), drawBuf(1), time(0), particleLifetime(10.5f), nParticles(4000), emitterPos(0, 0.8, 0), emitterDir(0, 1, 0)
{
    beginParticles = false;
    lightLVec = vec3(0.8f, 0.8f, 0.8f);
    lightLaVec = vec3(0.1f, 0.2f, 0.3f);
    lightPos = vec4(0.0f, 2.0f, 0.0f, 1.0f);
    accel = vec3(0.0f, -1.5f, 0.0f);
    pSize = 0.05f;
    toggleWireframe = false;
    toggleSilhouette = false;
    fountainAdjacency = ObjMesh::loadWithAdjacency("media/fountain.obj", false);
    fountainNormal = ObjMesh::load("media/fountain.obj", false);
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

    // the particle texture
    glActiveTexture(GL_TEXTURE0);
    Texture::loadTexture("media/texture/bluewater.png");

    glActiveTexture(GL_TEXTURE1);
    ParticleUtils::createRandomTex1D(nParticles * 3);

    initBuffers();

    particleProg.use();
    particleProg.setUniform("RandomTex", 1);
    particleProg.setUniform("ParticleTex", 0);
    particleProg.setUniform("ParticleLifetime", particleLifetime);
    particleProg.setUniform("Accel", accel);
    particleProg.setUniform("ParticleSize", pSize);
    particleProg.setUniform("Emitter", emitterPos);
    particleProg.setUniform("EmitterBasis", ParticleUtils::makeArbitraryBasis(emitterDir));

    flatProg.use();
    flatProg.setUniform("Colour", vec4(0.3f, 0.3f, 0.3f, 1.0f));
}

void Scene_Project::initBuffers()
{
    // generate the buffers
    glGenBuffers(2, posBuf); // position buffers
    glGenBuffers(2, velBuf); // velocity buffers
    glGenBuffers(2, age); // age buffers

    // allocate space for all buffers
    int size = nParticles * 3 * sizeof(GLfloat);
    glBindBuffer(GL_ARRAY_BUFFER, posBuf[0]);
    glBufferData(GL_ARRAY_BUFFER, size, 0, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, posBuf[1]);
    glBufferData(GL_ARRAY_BUFFER, size, 0, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, velBuf[0]);
    glBufferData(GL_ARRAY_BUFFER, size, 0, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, velBuf[1]);
    glBufferData(GL_ARRAY_BUFFER, size, 0, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, age[0]);
    glBufferData(GL_ARRAY_BUFFER, nParticles * sizeof(float), 0, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, age[1]);
    glBufferData(GL_ARRAY_BUFFER, nParticles * sizeof(float), 0, GL_DYNAMIC_COPY);

    // fill the first age buffer
    std::vector<GLfloat> tempData(nParticles);
    float rate = particleLifetime / nParticles;

    for (int i = 0; i < nParticles; i++)
    {
        tempData[i] = rate * (i - nParticles);
    }

    glBindBuffer(GL_ARRAY_BUFFER, age[0]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, nParticles * sizeof(float), tempData.data());

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // create vertex arrays for each set of buffers
    glGenVertexArrays(2, particleArray);

    // set up particle array 0
    glBindVertexArray(particleArray[0]);
    glBindBuffer(GL_ARRAY_BUFFER, posBuf[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, velBuf[0]);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, age[0]);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);

    // set up particle array 1
    glBindVertexArray(particleArray[1]);
    glBindBuffer(GL_ARRAY_BUFFER, posBuf[1]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, velBuf[1]);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, age[1]);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    // setup the feedback objects
    glGenTransformFeedbacks(2, feedback);

    // transform feedback 0
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, feedback[0]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, posBuf[0]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, velBuf[0]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 2, age[0]);

    // transform feedback 1
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, feedback[1]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, posBuf[1]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, velBuf[1]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 2, age[1]);

    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
}

void Scene_Project::setMatrices(GLSLProgram& p)
{
    // model view matrix
    mat4 mv = view * model;

    p.setUniform("MV", mv);
    p.setUniform("Proj", projection);

    p.setUniform("NormalMatrix", glm::mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])));
    p.setUniform("MVP", projection * mv);

    p.setUniform("ViewportMatrix", viewport);
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

        // setup the transform feedback (must be done before linking the program
        GLuint progHandle = particleProg.getHandle();
        const char* outputNames[] = { "Position", "Velocity", "Age" };
        glTransformFeedbackVaryings(progHandle, 3, outputNames, GL_SEPARATE_ATTRIBS);
        particleProg.link();

        flatProg.compileShader("shader/flat_vert.glsl");
        flatProg.compileShader("shader/flat_frag.glsl");
        flatProg.link();

        wireProg.compileShader("shader/wireframe.vert");
        wireProg.compileShader("shader/wireframe.frag");
        wireProg.compileShader("shader/wireframe.geom");
        wireProg.link();

        silProg.compileShader("shader/silhouette.vert");
        silProg.compileShader("shader/silhouette.frag");
        silProg.compileShader("shader/silhouette.geom");
        silProg.link();
    }
    catch (GLSLProgramException& e) {
        cerr << e.what() << endl;
        exit(EXIT_FAILURE);
    }
}

void Scene_Project::update(float t)
{
    //update your angle here

    deltaT = t - time;
    time = t;

    if(toggleRotation)
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

    ImGui::Checkbox("Toggle Rotation", &toggleRotation);

    if (ImGui::CollapsingHeader("Particle System"))
    {
        //ImGui::Text("Particle System");
        ImGui::Checkbox("Begin Particles", &beginParticles);

        float particleAccel[3] = { accel.x,accel.y,accel.z };
        ImGui::SliderFloat3("Particle Accelaration", particleAccel, -2.0, 2.0);
        accel = vec3(particleAccel[0], particleAccel[1], particleAccel[2]);

        float particleSize = pSize;
        ImGui::SliderFloat("Particle Size", &particleSize, 0.001f, 1.0f);
        pSize = particleSize;

        float pLifetime = particleLifetime;
        ImGui::SliderFloat("Particle Lifetime", &pLifetime, 1.0f, 10.5f);
        particleLifetime = pLifetime;

        int particleAmount = nParticles;
        ImGui::SliderInt("Particle Amount", &particleAmount, 0, 10000);
        nParticles = particleAmount;

        //float particleCol[4] = { pColour.r,pColour.g,pColour.b,pColour.a };
        //ImGui::ColorEdit4("Particle Colour", particleCol);
        //pColour = vec4(particleCol[0], particleCol[1], particleCol[2], particleCol[3]);

        if (ImGui::Button("Reset Particles"))
        {
            time = 0.0f;
            deltaT = 0.0f;
        }
    }

    if (ImGui::CollapsingHeader("Light Properties"))
    {
        //ImGui::Text("Light Properties");
        ImGui::Text("Light Position");
        float pos[4] = { lightPos.x ,lightPos.y, lightPos.z , lightPos.w };
        ImGui::SliderFloat4("Light Position", pos, -10.0, 10.0);
        lightPos = vec4(pos[0], pos[1], pos[2], pos[3]);

        ImGui::Text("Light L");
        float lightL[3] = { lightLVec.x, lightLVec.y, lightLVec.z };
        ImGui::SliderFloat3("Light L", lightL, 0.0, 1.0);
        lightLVec = vec3(lightL[0], lightL[1], lightL[2]);

        ImGui::Text("Light La");
        float lightLa[3] = { lightLaVec.x, lightLaVec.y, lightLaVec.z };
        ImGui::SliderFloat3("Light La", lightLa, 0.0, 1.0);
        lightLaVec = vec3(lightLa[0], lightLa[1], lightLa[2]);
    }

    if (ImGui::CollapsingHeader("Wireframe"))
    {
        if (ImGui::Checkbox("Toggle Wireframe", &toggleWireframe))
        {
            toggleSilhouette = false;
        }

        if (ImGui::Checkbox("Toggle Silhouette", &toggleSilhouette))
        {
            toggleWireframe = false;
        }
    }

    ImGui::End();

    // Render and draw ImGui to the screen
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    view = glm::lookAt(vec3(3.0f * cos(angle), 1.5f, 3.0f * sin(angle)), vec3(0.0f, 1.5f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
    projection = glm::perspective(glm::radians(60.0f), (float)width / height, 0.3f, 100.0f);

    if (!toggleWireframe && !toggleSilhouette)
    {
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
    }

    if (toggleWireframe)
    {
        wireProg.use();
        wireProg.setUniform("Line.Width", 0.75f);
        wireProg.setUniform("Line.Colour", vec4(0.05f, 0.0f, 0.05f, 1.0f));
        wireProg.setUniform("Material.Kd", 0.7f, 0.7f, 0.7f);
        wireProg.setUniform("Light.Position", vec4(0.0f, 0.0f, 0.0f, 1.0f));
        wireProg.setUniform("Material.Ka", 0.2f, 0.2f, 0.2f);
        wireProg.setUniform("Light.Intensity", 1.0f, 1.0f, 1.0f);
        wireProg.setUniform("Material.Ks", 0.8f, 0.8f, 0.8f);
        wireProg.setUniform("Material.Shininess", 100.0f);
    }

    if (toggleSilhouette)
    {
        silProg.use();
        silProg.setUniform("EdgeWidth", 0.005f);
        silProg.setUniform("PctExtend", 0.25f);
        silProg.setUniform("LineColour", vec4(0.05f, 0.0f, 0.05f, 1.0f));
        silProg.setUniform("Material.Kd", 0.7f, 0.5f, 0.2f);
        silProg.setUniform("Light.Position", vec4(10.0f, 10.0f, 10.0f, 1.0f));
        silProg.setUniform("Material.Ka", 0.2f, 0.2f, 0.2f);
        silProg.setUniform("Light.Intensity", 1.0f, 1.0f, 1.0f);
    }

    model = mat4(1.0f);
    model = glm::translate(model, vec3(0.0f, 0.0f, 0.0f));

    if (toggleWireframe)
    {
        setMatrices(wireProg);
        fountainNormal->render();
    }
    else if (toggleSilhouette)
    {
        setMatrices(silProg);
        fountainAdjacency->render();
    }
    else
    {
        setMatrices(prog);
        fountainNormal->render();
    }

    // Render Grid

    flatProg.use();
    setMatrices(flatProg);
    grid.render();

    // Particle System
    particleProg.use();
    particleProg.setUniform("Time", time);
    particleProg.setUniform("DeltaT", deltaT);

    particleProg.setUniform("ParticleLifetime", particleLifetime);
    particleProg.setUniform("Accel", accel);
    particleProg.setUniform("ParticleSize", pSize);

    // update pass
    particleProg.setUniform("Pass", 1);

    glEnable(GL_RASTERIZER_DISCARD);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, feedback[drawBuf]);
    glBeginTransformFeedback(GL_POINTS);

    glBindVertexArray(particleArray[1 - drawBuf]);
    glVertexAttribDivisor(0, 0);
    glVertexAttribDivisor(1, 0);
    glVertexAttribDivisor(2, 0);
    glDrawArrays(GL_POINTS, 0, nParticles);
    glBindVertexArray(0);

    glEndTransformFeedback();
    glDisable(GL_RASTERIZER_DISCARD);

    // render pass
    particleProg.setUniform("Pass", 2);
    setMatrices(particleProg);

    glDepthMask(GL_FALSE);
    glBindVertexArray(particleArray[drawBuf]);
    glVertexAttribDivisor(0, 1);
    glVertexAttribDivisor(1, 1);
    glVertexAttribDivisor(2, 1);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, nParticles);
    glBindVertexArray(0);
    glDepthMask(GL_TRUE);

    // swap buffers
    drawBuf = 1 - drawBuf;
}

void Scene_Project::resize(int w, int h)
{
    // setup the viewport and the projection matrix
    glViewport(0, 0, w, h);

    width = w;
    height = h;

    projection = glm::perspective(glm::radians(60.0f), (float)w / h, 0.3f, 100.0f);

    float w2 = w / 2.0f;
    float h2 = h / 2.0f;

    viewport = mat4(vec4(w2, 0.0f, 0.0f, 0.0f),
        vec4(0.0f, h2, 0.0f, 0.0f),
        vec4(0.0f, 0.0f, 1.0f, 0.0f),
        vec4(w2 + 0, h2 + 0, 0.0f, 1.0f));
}
