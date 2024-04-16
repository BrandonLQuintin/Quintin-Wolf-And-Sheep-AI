#include "main_menu.h"

void processMainMenu(GLFWwindow* window, Shader t, int &choice){
    glfwSetKeyCallback(window, key_callback);
    glClearColor(0.16f, 0.80f, 1.00f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    processInput(window);

    std::string outputText;

    outputText =  "\\press enter to select\\";

    if (choice == 0)
        outputText += "\\x start program";
    else
        outputText += "\\  start program";

    if (choice == 1)
        outputText += std::string("\\x show_fps: ") + (SHOW_FPS ? "true" : "false");
    else
        outputText += std::string("\\  show_fps: ") + (SHOW_FPS ? "true" : "false");


    if (choice == 2)
        outputText += std::string("\\x quit_program");
    else
        outputText += std::string("\\  quit_program");


    renderText(t, outputText);

    if (enterKeyPressed){
        if (choice == 1){
            SHOW_FPS = !SHOW_FPS;
            enterKeyPressed = false;
        }
        else if (choice == 2){
            glfwTerminate();
            exit(0);
        }

        else{
            if (HIGH_RENDER_DISTANCE){
                VIEW_DISTANCE = 1000.0f;
                DESPAWN_DISTANCE = 300.0f;
                FOG_DENSITY = 0.001f;
            }
            if (!ENABLE_VSYNC){
                glfwSwapInterval(0); // uncapped framerate
            }
            else
                glfwSwapInterval(1);
            ENABLE_TEXT = text;
            mainMenu = false;
            if (ENABLE_SOUND)
                music.play();
            lastFrame = glfwGetTime();
        }
    }

    glfwSwapBuffers(window);
    glfwPollEvents();
}
