#include "calculate_fps.h"

int calculateAverageFPS(float &timeSinceCalculation, float deltaTime, std::vector<float> &fpsArray, bool isSlowMO){
    float elapsedTime = glfwGetTime() - timeSinceCalculation;

    if (elapsedTime > 0.1f){
        timeSinceCalculation = glfwGetTime();
        int fps;
        if (gameOver)
            fps = 404;
        else if (HIGH_SPEED && !animationModeActivate)
            fps = 1.0f / (deltaTime / 200.0f);
        else if (animationModeActivate)
            fps = 1.0f / (animationDeltaTime);
        else
            fps = 1.0f / deltaTime;
        fpsArray.push_back(fps);

    }

    int total = 0;
    for (int i = 0; i < fpsArray.size(); i++){
        if (fpsArray.size() >= 10){
            fpsArray.erase(fpsArray.begin());
        }
        total += fpsArray[i];
    }
    int averageFPS = total / fpsArray.size();
    return averageFPS;

};
