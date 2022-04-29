#include "scene_project.h"

#include <cstdio>
#include <cstdlib>

#include <string>
using std::string;

#include <iostream>
using std::cerr;
using std::endl;

#include "helper/glutils.h"
#include <glm/gtc/matrix_transform.hpp>

using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;

Scene_Project::Scene_Project() : plane(10,10,1,1)
{
    fountain = ObjMesh::load("media/fountain.obj", true);
}

void Scene_Project::initScene()
{
    compile();

    glEnable(GL_DEPTH_TEST);

    //glClearColor(0.9f, 0.9f, 0.9f, 1.0f);

    projection = mat4(1.0f);

    angle = glm::half_pi<float>();

    prog.setUniform("Light.Position", view * glm::vec4(0.0f, 2.0f, 0.0f, 1.0f));
    prog.setUniform("Light.L", vec3(0.8f, 0.8f, 0.8f));
    prog.setUniform("Light.La", vec3(0.1f, 0.2f, 0.3f));
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
    glBufferSubData(GL_ARRAY_BUFFER, 0, size * 3, data.data());

    // fill the start time buffer
    float rate = particleLifetime / nParticles;
    for (int i = 0; i < nParticles; i++)
    {
        data[i] = rate * i;
    }

    glBindBuffer(GL_ARRAY_BUFFER, startTime);
    glBufferSubData(GL_ARRAY_BUFFER, 0, nParticles * sizeof(float), data.data());

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenVertexArrays(1, &particles);
    glBindVertexArray(particles);
    glBindBuffer(GL_ARRAY_BUFFER, initVel);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, startTime);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, 0);
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
}

void Scene_Project::setMatrices(GLSLProgram& p)
{
    mat4 mv = view * model;

    prog.setUniform("ModelViewMatrix", mv);
    prog.setUniform("NormalMatrix", mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])));
    prog.setUniform("MVP", projection * mv);
}

void Scene_Project::compile()
{
    try {
        prog.compileShader("shader/projectshader.vert");
        prog.compileShader("shader/projectshader.frag");
        prog.link();
        prog.use();
    }
    catch (GLSLProgramException& e) {
        cerr << e.what() << endl;
        exit(EXIT_FAILURE);
    }
}

void Scene_Project::update(float t)
{
    //update your angle here

    //angle = std::fmod(angle + 0.001f, glm::half_pi<float>());
    //angle = std::fmod(angle + t, glm::half_pi<float>());
}

void Scene_Project::render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //create the rotation matrix here and update the uniform in the shader 

    view = glm::lookAt(vec3(0.5f * cos(angle), 0.75f, 3.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 2.0f, 0.0f));
    projection = glm::perspective(glm::radians(70.0f), (float)width / height, 0.3f, 100.0f);

    //setMatrices();
    //plane.render();

    setMatrices();
    grid.render();

    prog.setUniform("Material.Kd", 0.4f, 0.4f, 0.4f);
    prog.setUniform("Material.Ks", 0.9f, 0.9f, 0.9f);
    prog.setUniform("Material.Ka", 0.5f, 0.5f, 0.5f);
    prog.setUniform("Material.Shininess", 180.0f);

    model = mat4(1.0f);
    model = glm::rotate(model, glm::radians(90.0f), vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, vec3(1.0f));

    setMatrices();
    fountain->render();
}

void Scene_Project::resize(int w, int h)
{
    // setup the viewport and the projection matrix
    glViewport(0, 0, w, h);

    width = w;
    height = h;

    projection = glm::perspective(glm::radians(70.0f), (float)w / h, 0.3f, 100.0f);
}
