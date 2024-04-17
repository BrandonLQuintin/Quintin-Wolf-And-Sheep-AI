#define STB_IMAGE_IMPLEMENTATION
#define GLFW_INCLUDE_NONE

#include <iostream>
#include <string>
#include <vector>
#include <cmath>

// opengl & SFML
#include <glad/glad.h> // loads OpenGL pointers
#include <GLFW/glfw3.h> // window manager
#include <SFML/Audio.hpp>

//#include <stb_image.h> // helps load images (commented out because it's in textures.h)
#include <shader_m.h> // shader class from learnopengl.com on lesson "Coordinate Systems"

// opengl math libraries
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

// variables (may also contain functions)
#include "globals.h"
#include "opengl/camera_variables.h"
#include "shapes/shapes.h"
#include "shapes/initialize_shape_vertices.h"

// functions
#include "opengl/window_functions.h"
#include "opengl/shader_functions.h"
#include "opengl/textures.h"
#include "opengl/text_render.h"
#include "game/entity.h"
#include "game/calculate_fps.h"
#include "game/main_menu.h"
#include "game/gameplay.h"
#include "shapes/terrain.h"
#include "game/sound.h"

void spawnSheep(std::vector<sheep> &sheepVector);
void spawnWolf(std::vector<wolves> &wolfVector);

int main(){
    GLFWwindow* window = createWindow();
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    Shader billboardShader("shaders/billboard_shader.vs", "shaders/billboard_shader.fs");
    Shader phongShader("shaders/phong_lighting.vs", "shaders/phong_lighting.fs");
    Shader t("shaders/text_shader.vs", "shaders/text_shader.fs"); // variable name shortened so I can call it faster with renderText()

    billboardShader.use();
    billboardShader.setVec3("objectColor", 1.0f, 1.0f, 1.0f);
    billboardShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
    billboardShader.setVec3("lightPos", glm::vec3(-10000.0f, 1000.0f, 10000.0f));

    phongShader.use();
    phongShader.setVec3("objectColor", 1.0f, 1.0f, 1.0f);
    phongShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
    phongShader.setVec3("lightPos", glm::vec3(-1000.0f, 1000.0f, 1000.0f));

    generatePlaneVertices(phongTerrainVector, phongTerrainIndicesVector);
    initializeAllShapes();

    // ----- TEXTURES -----
    unsigned int texture1, texture2, texture3;
    loadTexture(texture1, "resources/textures/TextureAtlas.png");
    loadTexture(texture2, "resources/textures/TextAtlas.png");
    loadTexture(texture3, "resources/textures/GrassTiles.png");
    billboardShader.use();
    billboardShader.setInt("texture1", 0);
    billboardShader.setInt("texture2", 1);

    phongShader.use();
    phongShader.setInt("texture1", 0);
    phongShader.setInt("texture2", 2);

    t.use();
    t.setInt("texture1", 0);
    t.setInt("texture2", 1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture2);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, texture3);

    // first atlas values
    std::vector<float> boxAtlasUV = returnTextureUV(0, 0);
    std::vector<float> cobbleAtlasUV = returnTextureUV(1, 0);
    std::vector<float> oceroAtlasUV = returnTextureUV(2, 0);
    std::vector<float> shadowAtlasUV = returnTextureUV(0, 2);
    std::vector<float> rainAtlasUV = returnTextureUV(2, 2);

    std::vector<float> wolfAtlasUV = returnTextureUV(5, 0);
    std::vector<float> sheepAtlasUV = returnTextureUV(4, 0);

    std::vector<float> dust1AtlasUV = returnTextureUV(13, 3);
    std::vector<float> dust2AtlasUV = returnTextureUV(14, 3);
    std::vector<float> dust3AtlasUV = returnTextureUV(15, 3);

    std::vector<float> redSquareUV = returnTextureUV(1, 1);

    std::vector<float> cloudAtlasUV = {0.6875f, 1.0f, 0.25f, 0.375f}; // these textures take up multiple 64x64 pixel grids so I gave it hard coded numbers.
    std::vector<float> treeAtlasUV = {0.75f, 1.0f, 0.0625f, 0.25f};

    // ----- INITIALIZE OBJECTS -----

    // camera position
    cameraPos.x = 0.0f;
    cameraPos.y = getHeight(0.0f, 0.0f) + 5.0f;
    cameraPos.z = 0.0f;

    // initialize various sphere locations
    float heightOffset = 50.0f;

    generateSurroundingPositions(initialSpherePositions, 25);
    for (int i = 0; i < initialSpherePositions.size(); i++){
        initialSpherePositions[i] = initialSpherePositions[i] * glm::vec3(30.0f, 30.0f, 30.0f);
        initialSpherePositions[i].y = 10.0f + heightOffset;
    }

    // 1 terrain
    shape terrains[1];
    int terrainsArraySize = sizeof(terrains) / sizeof(terrains[0]);

    // 100 trees
    tree treesArray[5000];
    std::vector<tree> trees(std::begin(treesArray), std::end(treesArray));
    std::vector<tree> activeTrees;


    const float HALF_CHUNK_SIZE = CHUNK_SIZE / 2;
    for (int i = 0; i < trees.size(); i++){
        trees[i].modelMatrix[3][0] = cameraPos.x + randomInRange(-HALF_CHUNK_SIZE, HALF_CHUNK_SIZE);
        trees[i].modelMatrix[3][2] = cameraPos.z + randomInRange(-HALF_CHUNK_SIZE, HALF_CHUNK_SIZE);
        trees[i].modelMatrix[3][1] = getHeight(trees[i].modelMatrix[3][0], trees[i].modelMatrix[3][2]) + 2.3f;
        trees[i].modelMatrix = glm::scale(trees[i].modelMatrix, glm::vec3(5.0f));
    }
    for (int i = 0; i < trees.size(); i++){
        trees[i].cloneModelMatrix = trees[i].modelMatrix;
    }
    for (int i = 0; i < trees.size(); i++){
        trees[i].cloneModelMatrix = glm::rotate(trees[i].cloneModelMatrix, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        trees[i].cloneModelMatrix[3][0] = trees[i].modelMatrix[3][0] + 0.55f;
        trees[i].cloneModelMatrix[3][1] = trees[i].modelMatrix[3][1];
        trees[i].cloneModelMatrix[3][2] = trees[i].modelMatrix[3][2] + 0.55f;
    }

    // 2 boxes
    shape boxes[2];
    int boxesArraySize = sizeof(boxes) / sizeof(boxes[0]);
    boxes[0].modelMatrix = glm::translate(boxes[0].modelMatrix, glm::vec3(1.0f, 0.5f + heightOffset, -3.0f));
    boxes[1].modelMatrix = glm::translate(boxes[1].modelMatrix, glm::vec3(1.0f, 1.5f + heightOffset, -3.0f));

    // 1 floor
    shape floors[1];
    int floorsArraySize = sizeof(floors) / sizeof(floors[0]);
    floors[0].modelMatrix = glm::translate(floors[0].modelMatrix, glm::vec3(0.0f, 0.0f + heightOffset, 0.0f));
    floors[0].modelMatrix = glm::scale(floors[0].modelMatrix, glm::vec3(500.0f));

    // 1 pyramid
    shape pyramids[1];
    int pyramidsArraySize = sizeof(pyramids) / sizeof(pyramids[0]);
    pyramids[0].modelMatrix = glm::translate(pyramids[0].modelMatrix, glm::vec3(0.0f, 0.0f + heightOffset, -3.0f));

    // 25 spheres
    shape spheres[25];
    int spheresArraySize = sizeof(spheres) / sizeof(spheres[0]);
    for (int i = 0; i < spheresArraySize; i++){
        spheres[i].modelMatrix = glm::translate(spheres[i].modelMatrix, initialSpherePositions[i]);
    }

    // 1 cone
    shape cones[1];
    int conesArraySize = sizeof(cones) / sizeof(cones[0]);
    cones[0].modelMatrix = glm::translate(cones[0].modelMatrix, glm::vec3(-1.0f, 0.0f + heightOffset, -3.0f));

    // 1 tube
    shape tubes[1];
    int tubesArraySize = sizeof(tubes) / sizeof(tubes[0]);
    tubes[0].modelMatrix = glm::translate(tubes[0].modelMatrix, glm::vec3(-2.0f, 0.0f + heightOffset, -3.0f));

    // 1 billboard
    shape billboards[3];
    int billboardsArraySize = sizeof(billboards) / sizeof(billboards[0]);
    for (int i = 0; i < billboardsArraySize; i++){
        billboards[i].modelMatrix = glm::translate(billboards[i].modelMatrix, glm::vec3(1 - (i * 1.1), 0.0f + heightOffset, -5.0f));
    }


    // 1000 sheeps
    std::vector<sheep> sheeps(1000);
    sheeps.reserve(10000);
    for (int i = 0; i < sheeps.size(); i++){
        sheeps[i].modelMatrix[3][0] = randomInRange(-200.0f, 200.0f);
        sheeps[i].modelMatrix[3][2] = randomInRange(-200.0f, 200.0f);
        sheeps[i].modelMatrix[3][1] = getHeight(sheeps[i].modelMatrix[3][0], sheeps[i].modelMatrix[3][2]) + 0.5f;

        sheeps[i].goTo.x = randomInRange(-200.0f, 200.0f);
        sheeps[i].goTo.z = randomInRange(-200.0f, 200.0f);
        sheeps[i].goTo.y = getHeight(sheeps[i].goTo.x, sheeps[i].goTo.z) + 0.5f;
    }
    totalSheeps = sheeps.size();
    sheeps[0].modelMatrix[3][0] = 5.0f;
    sheeps[0].modelMatrix[3][2] = 0.0f;
    sheeps[0].modelMatrix[3][1] = getHeight(sheeps[0].modelMatrix[3][0], sheeps[0].modelMatrix[3][2]) + 0.5f;

    bool HIGH_SPEED = false;

    // 10 wolves
    std::vector<wolves> wolves(10);
    for (int i = 0; i < wolves.size(); i++){
        wolves[i].modelMatrix[3][0] = randomInRange(-200.0f, 200.0f);
        wolves[i].modelMatrix[3][2] = randomInRange(-200.0f, 200.0f);
        wolves[i].modelMatrix[3][1] = getHeight(wolves[i].modelMatrix[3][0], wolves[i].modelMatrix[3][2]) + 0.5f;

        wolves[i].goTo = glm::vec3(randomInRange(-200.0f, 200.0f), 0.0f, randomInRange(-200.0f, 200.0f));
        wolves[i].goTo.y = getHeight(wolves[i].goTo.x, wolves[i].goTo.z);
    }
    wolves[0].modelMatrix[3][0] = 5.0f;
    wolves[0].modelMatrix[3][2] = -5.0f;
    wolves[0].modelMatrix[3][1] = getHeight(wolves[0].modelMatrix[3][0], wolves[0].modelMatrix[3][2]) + 0.5f;



    // rain drops
    rainEntity rainDrops[600];
    int rainDropsArraySize = sizeof(rainDrops) / sizeof(rainDrops[0]);
    for (int i = 0; i < rainDropsArraySize; i++)
        initializeRainLocation(rainDrops[i]);

    if (IS_RAINING){
        phongShader.use();
        phongShader.setBool("isRaining", true);
        phongShader.setFloat("fogDensity", FOG_DENSITY);
        billboardShader.use();
        billboardShader.setBool("isRaining", true);
        billboardShader.setFloat("fogDensity", FOG_DENSITY);
    }

    // ----- MAIN PROGRAM -----

    while (!glfwWindowShouldClose(window)){
        while (mainMenu){
            processMainMenu(window, t, menuChoice);
            timeSinceLastEnemyWait = glfwGetTime();
        }

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;

        if (HIGH_SPEED)
            deltaTime *= 200.0f;

        timer += deltaTime * 1.0f;
        if (timer > 300.0f){
            timer = 0.0;
            int spawnCount = static_cast<int>(totalSheeps * 0.2);
            for (int i = 0; i < spawnCount; i++){
                spawnSheep(sheeps);
            }
        }

        lastFrame = currentFrame;
        processInput(window);

        glClearColor(0.792f, 0.957f, 1.00f, 1.0f);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (!FREECAM_CONTROLS_ENABLED){
            cameraFront = glm::normalize(glm::vec3(player[3][0], player[3][1], player[3][2]) - cameraPos);
            view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        }


        billboardShader.use();
        billboardShader.setMat4("view", view);
        billboardShader.setMat4("projection", projection);
        billboardShader.setVec3("viewPos", cameraPos);

        phongShader.use();
        phongShader.setMat4("view", view);
        phongShader.setMat4("projection", projection);
        phongShader.setVec3("viewPos", cameraPos);

        // ----- OBJECTS ------

        // ### TERRAIN
        for (int i = 0; i < terrainsArraySize; i++){
            phongShader.use();
            glBindVertexArray(phongTerrainVAO);
            setTextureUV(phongShader, oceroAtlasUV, true);

            phongShader.setMat4("model", terrains[i].modelMatrix);
            glDrawElements(GL_TRIANGLES, phongTerrainIndicesVector.size(), GL_UNSIGNED_INT, 0);
        }

        // ### SHEEPS
        billboardShader.use();
        glBindVertexArray(phongBillboardVAO);
        setTextureUV(billboardShader, sheepAtlasUV, false);
        for (int i = 0; i < sheeps.size(); i++){
            if (sheeps[i].isAlive){
                sheeps[i].distanceFromDestination = calculateDistance(glm::vec3(sheeps[i].modelMatrix[3][0], sheeps[i].modelMatrix[3][1], sheeps[i].modelMatrix[3][2]),
                                        sheeps[i].goTo);
                sheeps[i].hunger -= deltaTime * 0.1f;
                if (sheeps[i].hunger < 0){
                    sheeps[i].isAlive = false;
                    totalSheeps -= 1;
                }


                glm::vec3 sheepPos = glm::vec3(sheeps[i].modelMatrix[3][0], sheeps[i].modelMatrix[3][1], sheeps[i].modelMatrix[3][2]);


                if (sheeps[i].distanceFromDestination > 1.0f){
                    if (sheeps[i].isRunning)
                        moveToPoint(sheepPos, sheeps[i].goTo, deltaTime, 5.0f);
                    else
                        moveToPoint(sheepPos, sheeps[i].goTo, deltaTime, 1.0f);
                    sheeps[i].modelMatrix[3][0] = sheepPos.x;
                    sheeps[i].modelMatrix[3][1] = getHeight(sheepPos.x, sheepPos.z) + 0.5f;
                    sheeps[i].modelMatrix[3][2] = sheepPos.z;
                }
                else{
                    //spawnSheep(sheeps);
                    sheeps[i].hunger = 40.0f;
                    sheeps[i].goTo.x = randomInRange(-200.0f, 200.0f);
                    sheeps[i].goTo.z = randomInRange(-200.0f, 200.0f);
                    sheeps[i].goTo.y = getHeight(sheeps[i].goTo.x, sheeps[i].goTo.z) + 0.5f;
                    //std::cout << "New goTo for sheep " << i << "!" << std::endl;
                }

                // find a sheep target
                if (sheeps[i].isRunning){
                    sheeps[i].timeSinceRunning -= deltaTime * 10.0f;
                    if (sheeps[i].timeSinceRunning < 0){
                        sheeps[i].isRunning = false;
                        wolves[sheeps[i].nearestWolf].isChasing = false;
                    }
                }
                float nearestDistance = 1000.0f;
                if (!sheeps[i].isRunning){
                    for (int x = 0; x < wolves.size(); x++){
                        float distanceFromWolf = calculateDistance(sheepPos, glm::vec3(wolves[x].modelMatrix[3][0], wolves[x].modelMatrix[3][1], wolves[x].modelMatrix[3][2]));
                        if (distanceFromWolf < nearestDistance){
                            nearestDistance = distanceFromWolf;
                            if (nearestDistance < 5.0f){
                                //std::cout << "S " << i << " is getting chased!" << std::endl;
                                sheeps[i].isRunning = true;
                                wolves[x].isChasing = true;
                                sheeps[i].nearestWolf = x;
                                wolves[x].targetSheep = &sheeps[i];
                                wolves[x].sheepID = i;
                                sheeps[i].timeSinceRunning = 100.0f;
                                wolves[x].timeSinceChasing = 100.0f;
                            }
                        }
                    }
                }



                billboardShader.setMat4("model", sheeps[i].modelMatrix);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }


        }

        // ### WOLVES
        billboardShader.use();
        glBindVertexArray(phongBillboardVAO);
        setTextureUV(billboardShader, wolfAtlasUV, false);
        for (int i = 0; i < wolves.size(); i++){
            wolves[i].distanceFromDestination = calculateDistance(glm::vec3(wolves[i].modelMatrix[3][0], wolves[i].modelMatrix[3][1], wolves[i].modelMatrix[3][2]),
                                                            wolves[i].goTo);

            wolves[i].hunger -= deltaTime * 0.1f;
            if (wolves[i].hunger < 0){
                std::cout << "Wolf " << i << " starved to death!" << std::endl;
                wolves.erase(wolves.begin() + i);
            }


            if (wolves[i].distanceFromDestination > 1.0f){
                glm::vec3 wolfPos = glm::vec3(wolves[i].modelMatrix[3][0], wolves[i].modelMatrix[3][1], wolves[i].modelMatrix[3][2]);

                if (wolves[i].isChasing){
                    glm::vec3 sheepPos = glm::vec3(wolves[i].targetSheep->modelMatrix[3][0], wolves[i].targetSheep->modelMatrix[3][1], wolves[i].targetSheep->modelMatrix[3][2]);
                    wolves[i].goTo = sheepPos;
                    moveToPoint(wolfPos, wolves[i].goTo, deltaTime, 5.0f);
                    // check if wolf killed sheep
                    float distanceFromSheep = calculateDistance(wolfPos, sheepPos);
                    if (distanceFromSheep < 1.0f){
                        sheeps[wolves[i].sheepID].isAlive = false;
                        totalSheeps -= 1;
                        float chance = randomInRange(0.0f, 100.0f);
                        if (chance < 1.0f)
                            spawnWolf(wolves);
                        wolves[i].hunger = 100.0f;
                        wolves[i].isChasing = false;
                        wolves[i].timeSinceChasing = 0.0f;
                    }
                }
                else
                    moveToPoint(wolfPos, wolves[i].goTo, deltaTime, 1.0f);

                moveToPoint(wolfPos, wolves[i].goTo, deltaTime, 1.0f);
                wolves[i].modelMatrix[3][0] = wolfPos.x;
                wolves[i].modelMatrix[3][1] = getHeight(wolfPos.x, wolfPos.z) + 0.5f;
                wolves[i].modelMatrix[3][2] = wolfPos.z;
            }
            else{
                wolves[i].goTo.x = randomInRange(-200.0f, 200.0f);
                wolves[i].goTo.z = randomInRange(-200.0f, 200.0f);
                wolves[i].goTo.y = getHeight(wolves[i].goTo.x, wolves[i].goTo.z) + 0.5f;
                //std::cout << "New goTo for wolf " << i << "!" << std::endl;
            }

            billboardShader.setMat4("model", wolves[i].modelMatrix);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }


        // ### TREES
        phongShader.use();
        phongShader.setBool("isTree", true);

        glEnable(GL_BLEND);
        glBindVertexArray(phongBillboardVAO);
        setTextureUV(phongShader, treeAtlasUV, false);

        // uses binary search to insert in correct order instead of sorting every frame
        for (int i = 0; i < trees.size(); i++) { // Adds activeTrees based on distance (before rendering) so transparency occurs correctly. chatGPT generated
            glm::vec3 treePosition = glm::vec3(trees[i].modelMatrix[3]);
            trees[i].distanceFromCamera = renderDistanceCheck(cameraPos, treePosition);

            if (trees[i].distanceFromCamera < DESPAWN_DISTANCE) {
                bool inserted = false;
                for (std::vector<tree>::iterator it = activeTrees.begin(); it != activeTrees.end(); ++it) {
                    if (trees[i].distanceFromCamera > it->distanceFromCamera) {
                        activeTrees.insert(it, trees[i]);
                        inserted = true;
                        break;
                    }
                }
                if (!inserted) {
                    activeTrees.push_back(trees[i]);
                }
            }
        }
        for (int i = 0; i < activeTrees.size(); i++) {
            if (activeTrees[i].distanceFromCamera < 5.0f){
                phongShader.setBool("isTransparent", true);
                phongShader.setMat4("model", activeTrees[i].modelMatrix);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                phongShader.setMat4("model", activeTrees[i].cloneModelMatrix);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                phongShader.setBool("isTransparent", false);
            }
            else{
                phongShader.setMat4("model", activeTrees[i].modelMatrix);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                phongShader.setMat4("model", activeTrees[i].cloneModelMatrix);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }
        }
        activeTrees.clear();
        phongShader.setBool("isTree", false);

        glDisable(GL_BLEND);

        // ----- DRAW TEXT ------
        int fps = calculateAverageFPS(timeSinceLastFPSCalculation, deltaTime, fpsVector, SLOW_MO);

        std::string text;
        if (FREECAM_CONTROLS_ENABLED){
            text += "\\camera coordinates: [" + std::to_string(cameraPos.x) + ", "+ std::to_string(cameraPos.y) + ", " + std::to_string(cameraPos.z) + "]\\\\\\world size: 200 x 200 meters";
        }
        if (totalSheeps < 0)
            totalSheeps = 0;
        text += "\\total sheeps: " + std::to_string(totalSheeps);
        text += "\\total wolves: " + std::to_string(wolves.size());
        renderText(t, text);

        // end of a frame

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;

}


void spawnSheep(std::vector<sheep> &sheepVector){
    sheep newSheep;

    newSheep.modelMatrix[3][0] = randomInRange(-200.0f, 200.0f);
    newSheep.modelMatrix[3][2] = randomInRange(-200.0f, 200.0f);
    newSheep.modelMatrix[3][1] = getHeight(newSheep.modelMatrix[3][0], newSheep.modelMatrix[3][2]) + 0.5f;

    newSheep.goTo.x = randomInRange(-200.0f, 200.0f);
    newSheep.goTo.z = randomInRange(-200.0f, 200.0f);
    newSheep.goTo.y = getHeight(newSheep.goTo.x, newSheep.goTo.z) + 0.5f;

    totalSheeps += 1;
    sheepVector.push_back(newSheep);
}
void spawnWolf(std::vector<wolves> &wolfVector){
    wolves newWolf;

    newWolf.modelMatrix[3][0] = randomInRange(-200.0f, 200.0f);
    newWolf.modelMatrix[3][2] = randomInRange(-200.0f, 200.0f);
    newWolf.modelMatrix[3][1] = getHeight(newWolf.modelMatrix[3][0], newWolf.modelMatrix[3][2]) + 0.5f;

    newWolf.goTo.x = randomInRange(-200.0f, 200.0f);
    newWolf.goTo.z = randomInRange(-200.0f, 200.0f);
    newWolf.goTo.y = getHeight(newWolf.goTo.x, newWolf.goTo.z) + 0.5f;

    wolfVector.push_back(newWolf);
}

