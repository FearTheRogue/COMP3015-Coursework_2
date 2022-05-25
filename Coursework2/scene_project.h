#ifndef SCENE_PROJECT_H
#define SCENE_PROJECT_H

#include "helper/scene.h"

#include <glad/glad.h>
#include "helper/glslprogram.h"
#include "helper/grid.h"
#include "helper/objmesh.h"
#include "helper/random.h"
#include "helper/particleutils.h"

class Scene_Project : public Scene
{
private:
    GLSLProgram prog;
    GLSLProgram particleProg, flatProg;
    GLSLProgram wireProg, silProg;

    Random rand;

    glm::vec3 emitterPos, emitterDir;

    // particle buffers
    GLuint posBuf[2], velBuf[2], age[2];
    // particle VAOs
    GLuint particleArray[2];
    // transform feedbacks
    GLuint feedback[2];
    GLuint drawBuf;

    int nParticles;
    float angle, time, particleLifetime;
    float tPrev;
    float deltaT;

    bool beginParticles;
    bool toggleRotation;
    glm::vec3 lightLVec;
    glm::vec3 lightLaVec;
    glm::vec4 lightPos;
    glm::vec3 accel;
    float pSize;
    //glm::vec4 pColour;

    glm::mat4 viewport;

    bool toggleWireframe;
    bool toggleSilhouette;

    Grid grid;
    std::unique_ptr<ObjMesh> fountain;

    void initBuffers();

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