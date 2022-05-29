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

    float angle, time;
    float tPrev, deltaT;

    // Movement
    bool toggleRotation;
    float rotationSpeed;
    float zoomFOV;
    float zoomSpeed;

    // Light Properties
    glm::vec3 lightLVec;
    glm::vec3 lightLaVec;
    glm::vec4 lightPos;

    glm::mat4 viewport;
    Grid grid;
    
    // Fountain model
    std::unique_ptr<ObjMesh> fountainAdjacency;
    std::unique_ptr<ObjMesh> fountainNormal;
    // Fountain Wireframe Properties
    float wireLineWidthFountain;
    glm::vec4 wireLineColourFountain;
    // Fountain Silhouette Properties
    float silEdgeWidthFountain;
    glm::vec4 silLineColourFountain;
    int levelsFountain;

    // Particle Properties
    bool beginParticles;
    int nParticles;
    float particleLifetime;
    // particle buffers
    GLuint posBuf[2], velBuf[2], age[2];
    // particle VAOs
    GLuint particleArray[2];
    // transform feedbacks
    GLuint feedback[2];
    GLuint drawBuf;
    glm::vec3 accel;
    float pSize;

    // Fireplace model
    std::unique_ptr<ObjMesh> firePlaceNormal;
    std::unique_ptr<ObjMesh> firePlaceAdjacency;
    // Fireplace Wireframe Properties
    float wireLineWidthFireplace;
    glm::vec4 wireLineColourFireplace;
    // Fireplace Silhouette Properties
    float silEdgeWidthFireplace;
    glm::vec4 silLineColourFireplace;
    int levelsFireplace;

    // Fire Properties
    bool beginFireSimulation;
    float fireLifetime;
    float fSize;
    int fireParticles;
    // particle buffers
    GLuint posBufFire[2], velBufFire[2], ageFire[2];
    // particle VAOs
    GLuint particleArrayFire[2];
    // transform feedbacks
    GLuint feedbackFire[2];
    GLuint drawBufFire;

    // Pond Model
    std::unique_ptr<ObjMesh> pondbrinksNormal;
    std::unique_ptr<ObjMesh> pondbricksAdjacency;
    // Pond Silhouette Properties
    float silEdgeWidthPond;
    glm::vec4 silLineColourPond;
    int levelsPond;
    // Pond Wireframe Properties
    float wireLineWidthPond;
    glm::vec4 wireLineColourPond;

    // Water model
    std::unique_ptr<ObjMesh> water;
    // Water Properties
    float waterFrequency, waterVelocity, WaterAmp;

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