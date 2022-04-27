#ifndef SCENE_PROJECT_H
#define SCENE_PROJECT_H

#include "helper/scene.h"

#include <glad/glad.h>
#include "helper/glslprogram.h"
#include "helper/grid.h"
#include "helper/objmesh.h"

class Scene_Project : public Scene
{
private:

    GLSLProgram prog;
    float angle;
    float tPrev;

    Grid grid;
    std::unique_ptr<ObjMesh> fountain;

    void setMatrices();
    void compile();

public:
    Scene_Project();

    void initScene();
    void update(float t);
    void render();
    void resize(int, int);
};

#endif // SCENE_PROJECT_H