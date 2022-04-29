#ifndef SCENE_PROJECT_H
#define SCENE_PROJECT_H

#include "helper/scene.h"

#include <glad/glad.h>
#include "helper/glslprogram.h"
#include "helper/grid.h"
#include "helper/objmesh.h"
#include "helper/plane.h"
#include "helper/random.h"
#include "helper/particleutils.h"

class Scene_Project : public Scene
{
private:

    GLSLProgram prog, flatProg;

    Random rand;

    GLuint initVel, startTime, particles, nParticles;

    float angle, time, particleLifetime;
    float tPrev;

    Plane plane;

    Grid grid;
    std::unique_ptr<ObjMesh> fountain;

    glm::vec3 emitterPos, emitterDir;

    void initBuffers();
    float randFloat();

    void setMatrices(GLSLProgram&);
    void compile();

public:
    Scene_Project();

    void initScene();
    void update(float t);
    void render();
    void resize(int, int);
};

#endif // SCENE_PROJECT_H