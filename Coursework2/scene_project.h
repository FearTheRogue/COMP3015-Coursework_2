#ifndef SCENE_PROJECT_H
#define SCENE_PROJECT_H

#include "helper/scene.h"

#include <glad/glad.h>
#include "helper/glslprogram.h"
#include "helper/grid.h"
#include "helper/objmesh.h"
#include "helper/random.h"
#include "helper/particleutils.h"
#include <GLFW/glfw3.h>

class Scene_Project : public Scene
{
private:
    GLFWwindow& window;

    GLSLProgram prog;
    GLSLProgram particleProg, flatProg;
    GLSLProgram wireProg, silProg, fireProg, waterProg;

    Random rand;

    glm::vec3 emitterPos, emitterDir;

    // particle buffers
    GLuint posBuf[2], velBuf[2], age[2];
    // particle VAOs
    GLuint particleArray[2];
    // transform feedbacks
    GLuint feedback[2];
    GLuint drawBuf;

    // particle buffers
    GLuint posBufF[2], velBufF[2], ageF[2];
    // particle VAOs
    GLuint particleArrayF[2];
    // transform feedbacks
    GLuint feedbackF[2];
    GLuint drawBufF;

    // Movement
    bool toggleRotation;
    float rotationSpeed;
    float zoomFOV;
    float zoomSpeed;

    // Light Properties
    glm::vec3 lightLVec;
    glm::vec3 lightLaVec;
    glm::vec4 lightPos;

    // Particle Properties
    bool beginParticles;
    int nParticles;
    float angle, time, particleLifetime;
    float tPrev;
    float deltaT;
    glm::vec3 accel;
    float pSize;

    // Fire Properties
    bool beginFireSimulation;
    float fireLifetime;
    float fSize;
    int fireParticles;

    // Wireframe Properties
    bool toggleWireframeFountain;
    float wireLineWidthF;
    glm::vec4 wireLineColourF;

    bool toggleWireframeFireplace;
    float wireLineWidthFP;
    glm::vec4 wireLineColourFP;

    // Silhouette Properties
    bool toggleSilhouetteFountain;
    float silEdgeWidthF, silPctExtend;
    glm::vec4 silLineColourF, silLightPos;
    glm::vec3 silMatKd, silMatKa, silLightIntensity;

    int levelsF, levelsFP;

    bool toggleSilhouetteFireplace;
    float silEdgeWidthFP, silPctExtendFP;
    glm::vec4 silLineColourFP;

    glm::mat4 viewport;
    Grid grid;
    
    std::unique_ptr<ObjMesh> fountainAdjacency;
    std::unique_ptr<ObjMesh> fountainNormal;

    std::unique_ptr<ObjMesh> firePlaceNormal;
    std::unique_ptr<ObjMesh> firePlaceAdjacency;

    std::unique_ptr<ObjMesh> pondbrinksNormal;
    std::unique_ptr<ObjMesh> pondbricksAdjacency;

    std::unique_ptr<ObjMesh> water;
    float waterFrequency, waterVelocity, WaterAmp;
    float silEdgeWidthPond;
    glm::vec4 silLineColourPond;
    int levelsPond;

    void initBuffers(GLuint posB[], GLuint velB[], GLuint ageB[], GLuint pArray[], GLuint fb[], GLuint drawBuffer, int numParticles, float lifetime);

    void setMatrices(GLSLProgram&);
    void compile();

    void setupNormalShader();
    void setupWireframeShader(float lineWidth, glm::vec4 lineColour);
    void setupSilhouetteShader(float edgeWidth, glm::vec4 lineColour, int levels);

public:
    Scene_Project(GLFWwindow& window);

    void initScene();
    void update(float t);
    void render();
    void resize(int, int);
};

#endif // SCENE_PROJECT_H