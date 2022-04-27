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

Scene_Project::Scene_Project() : angle(0.0f)
{
    fountain = ObjMesh::load("media/fountain.obj", true);
}

void Scene_Project::initScene()
{
    compile();

    glEnable(GL_DEPTH_TEST);

    glClearColor(0.9f, 0.9f, 0.9f, 1.0f);

    view = glm::lookAt(vec3(0.5f, 0.75f, 0.75f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
    projection = mat4(1.0f);

    prog.setUniform("Light.Position", view * glm::vec4(1.0f, 1.2f, 1.0f, 1.0f));
    prog.setUniform("Light.L", vec3(0.8f, 0.8f, 0.8f));
    prog.setUniform("Light.La", vec3(0.5f, 0.5f, 0.8f));
}

void Scene_Project::setMatrices()
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

    float deltaT = t - tPrev;

    if (tPrev == 0.0f)
        deltaT = 0.0f;

    tPrev = t;

    angle += 0.25f * deltaT;

    if (angle > glm::two_pi<float>())
        angle -= glm::two_pi<float>();
}

void Scene_Project::render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //create the rotation matrix here and update the uniform in the shader 

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
