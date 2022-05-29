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

Scene_Project::Scene_Project(GLFWwindow& window) : window(window), angle(0.0f), drawBuf(1), drawBufF(1), time(0), particleLifetime(10.5f), nParticles(4000), emitterPos(0, 0.8, 0), emitterDir(0, 1, 0)
{
    toggleRotation = false;
    rotationSpeed = 0.01f;
    zoomFOV = 3.0f;

    beginParticles = false;
    lightLVec = vec3(0.8f, 0.8f, 0.8f);
    lightLaVec = vec3(0.1f, 0.2f, 0.3f);
    lightPos = vec4(0.0f, 2.0f, 0.0f, 1.0f);
    accel = vec3(0.0f, -1.5f, 0.0f);
    pSize = 0.05f;
    
    beginFireSimulation = false;
    fireLifetime = 1.5f;
    fSize = 0.08f;
    fireParticles = 4000;

    waterFrequency = 3.0f;
    waterVelocity = 2.5f;
    WaterAmp = 0.10f;

    silEdgeWidthPond = 0.005f;
    silLineColourPond = vec4(0.05f, 0.0f, 0.05f, 1.0f);
    levelsPond = 4;

    toggleWireframeFountain = false;
    wireLineWidthF = 0.75f;
    wireLineColourF = vec4(0.05f, 0.0f, 0.05f, 1.0f);
    
    toggleSilhouetteFountain = false;
    silEdgeWidthF = 0.005f;
    silPctExtend = 0.25f;
    silLineColourF = vec4(0.05f, 0.0f, 0.05f, 1.0f);
    silLightPos = vec4(10.0f, 10.0f, 10.0f, 1.0f);
    silMatKd = vec3(0.7f, 0.5f, 0.2f);
    silMatKa = vec3(0.2f, 0.2f, 0.2f);
    silLightIntensity = vec3(1.0f, 1.0f, 1.0f);

    levelsF = 4, levelsFP = 4;

    fountainAdjacency = ObjMesh::loadWithAdjacency("media/fountain.obj", false);
    fountainNormal = ObjMesh::load("media/fountain.obj", false);

    firePlaceNormal = ObjMesh::load("media/fireplace.obj", false);
    firePlaceAdjacency = ObjMesh::loadWithAdjacency("media/fireplace.obj", false);

    pondbrinksNormal = ObjMesh::load("media/pondbricks.obj", false);
    pondbricksAdjacency = ObjMesh::loadWithAdjacency("media/pondbricks.obj", false);

    water = ObjMesh::load("media/water.obj", false);
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

    glActiveTexture(GL_TEXTURE2);
    Texture::loadTexture("media/texture/fire.png");

    initBuffers(posBuf, velBuf, age, particleArray, feedback, drawBuf, nParticles, particleLifetime);
    initBuffers(posBufF, velBufF, ageF, particleArrayF, feedbackF, drawBufF, fireParticles, fireLifetime);

    particleProg.use();
    particleProg.setUniform("RandomTex", 1);
    particleProg.setUniform("ParticleTex", 0);
    particleProg.setUniform("ParticleLifetime", particleLifetime);
    particleProg.setUniform("Accel", accel);
    particleProg.setUniform("ParticleSize", pSize);
    particleProg.setUniform("Emitter", emitterPos);
    particleProg.setUniform("EmitterBasis", ParticleUtils::makeArbitraryBasis(emitterDir));

    fireProg.use();
    fireProg.setUniform("RandomTex", 1);
    fireProg.setUniform("ParticleTex", 2);
    fireProg.setUniform("ParticleLifetime", fireLifetime);
    fireProg.setUniform("Accel", vec3(0.0f, 0.5f, 0.0f));
    fireProg.setUniform("ParticleSize", fSize);
    fireProg.setUniform("Emitter", vec3(2.0,0.0,1.0));
    fireProg.setUniform("EmitterBasis", ParticleUtils::makeArbitraryBasis(vec3(0.0, 1.0, 0.0)));

    flatProg.use();
    flatProg.setUniform("Colour", vec4(0.3f, 0.3f, 0.3f, 1.0f));

    wireProg.use();
    wireProg.setUniform("Material.Kd", 0.7f, 0.7f, 0.7f);
    wireProg.setUniform("Material.Ka", 0.2f, 0.2f, 0.2f);
    wireProg.setUniform("Light.Intensity", 1.0f, 1.0f, 1.0f);
    wireProg.setUniform("Material.Ks", 0.8f, 0.8f, 0.8f);
    wireProg.setUniform("Material.Shininess", 100.0f);

    silProg.use();
    silProg.setUniform("PctExtend", silPctExtendFP);
    silProg.setUniform("Material.Kd", silMatKd);

    silProg.setUniform("Material.Ka", silMatKa);
    silProg.setUniform("Light.Intensity", silLightIntensity);

    waterProg.use();
    waterProg.setUniform("Material.Kd", 0.4f, 0.4f, 0.4f);
    waterProg.setUniform("Material.Ks", 0.9f, 0.9f, 0.9f);
    waterProg.setUniform("Material.Ka", 0.5f, 0.5f, 0.5f);
    waterProg.setUniform("Material.Shininess", 180.0f);
}

void Scene_Project::initBuffers(GLuint posB[], GLuint velB[], GLuint ageB[], GLuint pArray[], GLuint fb[], GLuint drawBuffer, int numParticles, float lifetime)
{
    // generate the buffers
    glGenBuffers(2, posB); // position buffers
    glGenBuffers(2, velB); // velocity buffers
    glGenBuffers(2, ageB); // age buffers

    // allocate space for all buffers
    int size = nParticles * 3 * sizeof(GLfloat);
    glBindBuffer(GL_ARRAY_BUFFER, posB[0]);
    glBufferData(GL_ARRAY_BUFFER, size, 0, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, posB[1]);
    glBufferData(GL_ARRAY_BUFFER, size, 0, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, velB[0]);
    glBufferData(GL_ARRAY_BUFFER, size, 0, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, velB[1]);
    glBufferData(GL_ARRAY_BUFFER, size, 0, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, ageB[0]);
    glBufferData(GL_ARRAY_BUFFER, nParticles * sizeof(float), 0, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, ageB[1]);
    glBufferData(GL_ARRAY_BUFFER, nParticles * sizeof(float), 0, GL_DYNAMIC_COPY);

    // fill the first age buffer
    std::vector<GLfloat> tempData(numParticles);
    float rate = lifetime / numParticles;

    for (int i = 0; i < numParticles; i++)
    {
        tempData[i] = rate * (i - numParticles);
    }

    glBindBuffer(GL_ARRAY_BUFFER, ageB[0]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, numParticles * sizeof(float), tempData.data());

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // create vertex arrays for each set of buffers
    glGenVertexArrays(2, pArray);

    // set up particle array 0
    glBindVertexArray(pArray[0]);
    glBindBuffer(GL_ARRAY_BUFFER, posB[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, velB[0]);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, ageB[0]);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);

    // set up particle array 1
    glBindVertexArray(pArray[1]);
    glBindBuffer(GL_ARRAY_BUFFER, posB[1]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, velB[1]);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, ageB[1]);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    // setup the feedback objects
    glGenTransformFeedbacks(2, fb);

    // transform feedback 0
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, fb[0]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, posB[0]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, velB[0]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 2, ageB[0]);

    // transform feedback 1
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, fb[1]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, posB[1]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, velB[1]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 2, ageB[1]);

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

    p.setUniform("ModelViewMatrix", projection * mv);
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

        fireProg.compileShader("shader/fire_simulation.vert");
        fireProg.compileShader("shader/fire_simulation.frag");

        // setup the transform feedback (must be done before linking the program
        GLuint progFireHandle = fireProg.getHandle();
        const char* fireOutputNames[] = { "Position", "Velocity", "Age" };
        glTransformFeedbackVaryings(progFireHandle, 3, fireOutputNames, GL_SEPARATE_ATTRIBS);
        fireProg.link();

        flatProg.compileShader("shader/flat_vert.glsl");
        flatProg.compileShader("shader/flat_frag.glsl");
        flatProg.link();

        waterProg.compileShader("shader/wateranimation.vert");
        waterProg.compileShader("shader/wateranimation.frag");
        waterProg.link();

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

void Scene_Project::setupNormalShader()
{
    // Lighting the scene
    // Render model first, so it's drawn at the bottom
    prog.use();
    prog.setUniform("Light.Position", lightPos); // glm::vec4(0.0f, 2.0f, 0.0f, 1.0f)
    prog.setUniform("Light.L", lightLVec); // vec3(0.8f, 0.8f, 0.8f)
    prog.setUniform("Light.La", lightLaVec); // vec3(0.1f, 0.2f, 0.3f)

    prog.setUniform("Material.Kd", 0.4f, 0.4f, 0.4f);
    prog.setUniform("Material.Ks", 0.9f, 0.9f, 0.9f);
    prog.setUniform("Material.Ka", 0.5f, 0.5f, 0.5f);
    prog.setUniform("Material.Shininess", 180.0f);

    setMatrices(prog);
}

void Scene_Project::setupWireframeShader(float lineWidth, glm::vec4 lineColour)
{
    wireProg.use();
    wireProg.setUniform("Line.Width", lineWidth);
    wireProg.setUniform("Line.Colour", lineColour);
    wireProg.setUniform("Light.Position", lightPos);

    setMatrices(wireProg);
}

void Scene_Project::setupSilhouetteShader(float edgeWidth, glm::vec4 lineColour, int levels)
{
    silProg.use();
    silProg.setUniform("EdgeWidth", edgeWidth);
    silProg.setUniform("LineColour", lineColour);
    silProg.setUniform("Light.Position", lightPos);
    silProg.setUniform("levels", levels);

    setMatrices(silProg);
}

void Scene_Project::update(float t)
{
    //update your angle here

    deltaT = t - time;
    time = t;

    if(toggleRotation)
        angle = std::fmod(angle + rotationSpeed, glm::two_pi<float>());
    else
    {
        if (glfwGetKey(&window, GLFW_KEY_A))
        {
            angle = std::fmod(angle + rotationSpeed, glm::two_pi<float>());
        }

        if (glfwGetKey(&window, GLFW_KEY_D))
        {
            angle = std::fmod(angle - rotationSpeed, glm::two_pi<float>());
        }

        if (glfwGetKey(&window, GLFW_KEY_W))
        {
            zoomFOV = zoomFOV - rotationSpeed;
        }

        if (glfwGetKey(&window, GLFW_KEY_S))
        {
            zoomFOV = zoomFOV + rotationSpeed;
        }
    }
}

void Scene_Project::render()
{
    // we clear colour and depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    static int fountainSelection = 0;
    static int fireplaceSelection = 0;
    static int pondSelection = 0;

    view = glm::lookAt(vec3(zoomFOV * cos(angle), 1.5f, zoomFOV * sin(angle)), vec3(0.0f, 0.5f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
    projection = glm::perspective(glm::radians(60.0f), (float)width / height, 0.3f, 100.0f);

    // Render Grid
    flatProg.use();
    model = glm::translate(model, vec3(0.0f, 0.0f, 0.0f));
    setMatrices(flatProg);
    grid.render();

    model = mat4(1.0f);
    model = glm::translate(model, vec3(2.0f, 0.0f, 0.0f));
    model = glm::translate(model, vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, vec3(0.6f));

    if (fireplaceSelection == 1)
    {
        setupWireframeShader(wireLineWidthFP, wireLineColourFP);
        firePlaceNormal->render();
    }

    else if (fireplaceSelection == 2)
    {
        setupSilhouetteShader(silEdgeWidthFP, silLineColourFP, levelsFP);
        firePlaceAdjacency->render();
    }

    else if (fireplaceSelection == 0)
    {
        setupNormalShader();
        firePlaceNormal->render();
    }

    model = mat4(1.0f);
    model = glm::translate(model, vec3(-3.0f, 0.0f, 0.0f));
    model = glm::translate(model, vec3(0.0f, 0.0f, -1.0f));
    model = glm::scale(model, vec3(0.3f));

    if (pondSelection == 1)
    {
        setupWireframeShader(wireLineWidthF, wireLineColourF);
        pondbrinksNormal->render();
    }

    else if (pondSelection == 2)
    {
        setupSilhouetteShader(silEdgeWidthPond, silLineColourPond, levelsPond);
        pondbricksAdjacency->render();
    }

    else if (pondSelection == 0)
    {
        setupNormalShader();
        pondbrinksNormal->render();
    }

    // Pond water animation

    waterProg.use();
    waterProg.setUniform("Time", time);

    waterProg.setUniform("Light.L", lightLVec);
    waterProg.setUniform("Light.La", vec3(0.7f, 0.9f, 0.9f));
    waterProg.setUniform("Light.Position", lightPos);

    waterProg.setUniform("Freq", waterFrequency);
    waterProg.setUniform("Velocity", waterVelocity);
    waterProg.setUniform("Amp", WaterAmp);

    model = mat4(1.0f);
    model = glm::translate(model, vec3(-3.0f, 0.0f, 0.0f));
    model = glm::translate(model, vec3(0.0f, 0.0f, -1.0f));
    model = glm::scale(model, vec3(0.3f));

    setMatrices(waterProg);
    water->render();

    model = mat4(1.0f);
    model = glm::translate(model, vec3(0.0f, 0.0f, -2.0f));

    if (fountainSelection == 1)
    {
        setupWireframeShader(wireLineWidthF, wireLineColourF);
        fountainNormal->render();
    }
    
    else if (fountainSelection == 2)
    {
        setupSilhouetteShader(silEdgeWidthF, silLineColourF, levelsF);
        fountainAdjacency->render();
    }
       
    else if (fountainSelection == 0)
    {
        setupNormalShader();
        fountainNormal->render();
    }


    
    if (beginFireSimulation)
    {
        fireProg.use();
        fireProg.setUniform("Time", time);
        fireProg.setUniform("DeltaT", deltaT);

        fireProg.setUniform("ParticleLifetime", fireLifetime);
        fireProg.setUniform("ParticleSize", fSize);

        // update pass
        fireProg.setUniform("Pass", 1);

        glEnable(GL_RASTERIZER_DISCARD);
        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, feedbackF[drawBufF]);
        glBeginTransformFeedback(GL_POINTS);

        glBindVertexArray(particleArrayF[1 - drawBufF]);
        glVertexAttribDivisor(0, 0);
        glVertexAttribDivisor(1, 0);
        glVertexAttribDivisor(2, 0);
        glDrawArrays(GL_POINTS, 0, fireParticles);
        glBindVertexArray(0);

        glEndTransformFeedback();
        glDisable(GL_RASTERIZER_DISCARD);

        // render pass
        fireProg.setUniform("Pass", 2);
        setMatrices(fireProg);

        glDepthMask(GL_FALSE);
        glBindVertexArray(particleArrayF[drawBufF]);
        glVertexAttribDivisor(0, 1);
        glVertexAttribDivisor(1, 1);
        glVertexAttribDivisor(2, 1);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, fireParticles);
        glBindVertexArray(0);
        glDepthMask(GL_TRUE);

        // swap buffers
        drawBufF = 1 - drawBufF;
    }

    if (beginParticles)
    {
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

    // ImGui Initilisation
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Console");

    ImGui::TableNextColumn(); ImGui::Checkbox("Toggle Rotation", &toggleRotation); ImGui::SameLine();
    ImGui::TableNextColumn(); ImGui::SliderFloat("Rotation Speed", &rotationSpeed, 0.001f, 0.1f);

    if (!toggleRotation)
    {
        ImGui::Text("Use keyboard keys A & D to rotate.");
        ImGui::Text("Use keyboard keys W & S to zoom.");
    }

    if (ImGui::CollapsingHeader("Light Properties"))
    {
        ImGui::Text("Light Position");
        float pos[3] = { lightPos.x ,lightPos.y, lightPos.z };
        float pos1 = { lightPos.w };
        ImGui::SliderFloat3("Light Position", pos, -10.0, 10.0);
        ImGui::SliderFloat("Light Point/ Directional", &pos1, 0.0f, 1.0f);
        lightPos = vec4(pos[0], pos[1], pos[2], pos1);
    }

    if (ImGui::CollapsingHeader("Particle System"))
    {
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

        if (ImGui::Button("Reset Particles"))
        {
            time = 0.0f;
            deltaT = 0.0f;
        }
    }

    if (ImGui::CollapsingHeader("Fire System"))
    {
        ImGui::Checkbox("Begin Fire", &beginFireSimulation);

        float particleSize = fSize;
        ImGui::SliderFloat("Fire Size", &particleSize, 0.001f, 1.0f);
        fSize = particleSize;

        float fLifetime = fireLifetime;
        ImGui::SliderFloat("Fire Lifetime", &fLifetime, 1.0f, 5.5f);
        fireLifetime = fLifetime;

        int particleAmount = fireParticles;
        ImGui::SliderInt("Fire Particle Amount", &particleAmount, 0, 10000);
        fireParticles = particleAmount;
    }

    if (ImGui::CollapsingHeader("Water System"))
    {
        float frequency = waterFrequency;
        ImGui::SliderFloat("Water Frequency", &frequency, 0.01f, 6.0f);
        waterFrequency = frequency;

        float velocity = waterVelocity;
        ImGui::SliderFloat("Water Velocity", &velocity, 0.01f, 3.0f);
        waterVelocity = velocity;

        float amp = WaterAmp;
        ImGui::SliderFloat("Water Amp", &amp, 0.01f, 1.0f);
        WaterAmp = amp;
    }

    if (ImGui::CollapsingHeader("Geometry Effect Shaders"))
    {
        ImGui::Text("Fountain Model");
        ImGui::RadioButton("No Fountain Effect", &fountainSelection, 0); ImGui::SameLine();
        ImGui::RadioButton("Wireframe Fountain Effect", &fountainSelection, 1); ImGui::SameLine();
        ImGui::RadioButton("Silhouette Fountain Effect", &fountainSelection, 2);

        if(fountainSelection == 1)
        {
            if (ImGui::CollapsingHeader("Fountain Wireframe Settings"))
            {
                ImGui::BulletText("Please ensure that the wireframe toggle is enabled.");

                float wWidth = wireLineWidthF;
                ImGui::SliderFloat("Fountain Wireframe Line Width", &wWidth, 0.1f, 5.0f);
                wireLineWidthF = wWidth;

                float wireCol[3] = { wireLineColourF.r, wireLineColourF.g,wireLineColourF.b };
                ImGui::ColorEdit3("Fountain Wireframe Line Colour", wireCol);
                wireLineColourF = vec4(wireCol[0], wireCol[1], wireCol[2], 1);
            }
        }
        
        if (fountainSelection == 2)
        {
            if (ImGui::CollapsingHeader("Fountain Silhouette Settings"))
            {
                ImGui::BulletText("Please ensure that the silhouette toggle is enabled.");

                float eWidth = silEdgeWidthF;
                ImGui::SliderFloat("Fountain Silhouette Edge Width", &eWidth, 0.001f, 0.01f);
                silEdgeWidthF = eWidth;

                float lineCol[3] = { silLineColourF.r, silLineColourF.g, silLineColourF.b };
                ImGui::ColorEdit3("Fountain Silhouette Line Colour", lineCol);
                silLineColourF = vec4(lineCol[0], lineCol[1], lineCol[2], 1);

                int lvls = levelsF;
                ImGui::SliderInt("Fountain Silhouette Toon Shading Levels", &lvls, 1, 50);
                levelsF = lvls;
            }
        }

        ImGui::Text("Fireplace Model");
        ImGui::RadioButton("No Fireplace Effect", &fireplaceSelection, 0); ImGui::SameLine();
        ImGui::RadioButton("Wireframe Fireplace Effect", &fireplaceSelection, 1); ImGui::SameLine();
        ImGui::RadioButton("Silhouette Fireplace Effect", &fireplaceSelection, 2);

        if (fireplaceSelection == 1)
        {
            if (ImGui::CollapsingHeader("Fireplace Wireframe Settings"))
            {
                float wWidth = wireLineWidthFP;
                ImGui::SliderFloat("Fireplace Wireframe Line Width", &wWidth, 0.1f, 5.0f);
                wireLineWidthFP = wWidth;

                float wireCol[3] = { wireLineColourFP.r, wireLineColourFP.g,wireLineColourFP.b };
                ImGui::ColorEdit3("Fireplace Wireframe Line Colour", wireCol);
                wireLineColourFP = vec4(wireCol[0], wireCol[1], wireCol[2], 1);
            }
        }

        if (fireplaceSelection == 2)
        {
            if (ImGui::CollapsingHeader("Fireplace Silhouette Settings"))
            {
                float eWidth = silEdgeWidthFP;
                ImGui::SliderFloat("Fireplace Silhouette Edge Width", &eWidth, 0.001f, 0.01f);
                silEdgeWidthFP = eWidth;

                float lineCol[3] = { silLineColourFP.r, silLineColourFP.g, silLineColourFP.b };
                ImGui::ColorEdit3("Fireplace Silhouette Line Colour", lineCol);
                silLineColourFP = vec4(lineCol[0], lineCol[1], lineCol[2], 1);

                int lvls = levelsFP;
                ImGui::SliderInt("Fireplace Silhouette Toon Shading Levels", &lvls, 1, 50);
                levelsFP = lvls;
            }
        }

        ImGui::Text("Pond Model");
        ImGui::RadioButton("No Pond Effect", &pondSelection, 0); ImGui::SameLine();
        ImGui::RadioButton("Wireframe Pond Effect", &pondSelection, 1); ImGui::SameLine();
        ImGui::RadioButton("Silhouette Pond Effect", &pondSelection, 2);

        if (pondSelection == 1)
        {
            if (ImGui::CollapsingHeader("Pond Wireframe Settings"))
            {
                float wWidth = wireLineWidthFP;
                ImGui::SliderFloat("Pond Wireframe Line Width", &wWidth, 0.1f, 5.0f);
                wireLineWidthFP = wWidth;

                float wireCol[3] = { wireLineColourFP.r, wireLineColourFP.g,wireLineColourFP.b };
                ImGui::ColorEdit3("Pond Wireframe Line Colour", wireCol);
                wireLineColourFP = vec4(wireCol[0], wireCol[1], wireCol[2], 1);
            }
        }

        if (pondSelection == 2)
        {
            if (ImGui::CollapsingHeader("Pond Silhouette Settings"))
            {
                float eWidth = silEdgeWidthPond;
                ImGui::SliderFloat("Pond Silhouette Edge Width", &eWidth, 0.001f, 0.01f);
                silEdgeWidthPond = eWidth;

                float lineCol[3] = { silLineColourPond.r, silLineColourPond.g, silLineColourPond.b };
                ImGui::ColorEdit3("Pond Silhouette Line Colour", lineCol);
                silLineColourPond = vec4(lineCol[0], lineCol[1], lineCol[2], 1);

                int lvls = levelsPond;
                ImGui::SliderInt("Pond Silhouette Toon Shading Levels", &lvls, 1, 50);
                levelsPond = lvls;
            }
        }
    }

    ImGui::End();

    // Render and draw ImGui to the screen
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Scene_Project::resize(int w, int h)
{
    // setup the viewport and the projection matrix
    glViewport(0, 0, w, h);

    width = w;
    height = h;

    float w2 = w / 2.0f;
    float h2 = h / 2.0f;

    viewport = mat4(vec4(w2, 0.0f, 0.0f, 0.0f),
        vec4(0.0f, h2, 0.0f, 0.0f),
        vec4(0.0f, 0.0f, 1.0f, 0.0f),
        vec4(w2 + 0, h2 + 0, 0.0f, 1.0f));
}