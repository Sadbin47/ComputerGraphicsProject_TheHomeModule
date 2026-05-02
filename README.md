# Computer Graphics Project: The Home Module

A 2D urban environment built with C++ and OpenGL/GLUT, featuring a day/night cycle, dynamic rain, a moving car, an interactive garage door, and a starry night sky.

## Features
* **Day & Night Cycles:** Seamlessly transition between day and night scenes.
* **Weather System:** Toggleable dynamic rain with darkened storm clouds.
* **Animated Car & Garage:** The car drives down the road, parks in the garage, and the garage door automatically opens/closes.
* **Keyboard Controls:** Fully interactive using the keyboard.

## Controls
* **[Spacebar]** - Pause / Resume the animation
* **[ N ]** - Reset and force start Scene 1
* **[ R ]** - Turn Rain ON
* **[ V ]** - Turn Rain OFF
* **[ Left / Right Arrows ]** - Manually switch between Day and Night scenes
* **[ Esc ]** - Exit the application

## How to Run This Project (Code::Blocks)

This project requires **OpenGL** and **FreeGLUT** to run. 

1. Clone this repository to your local machine.
2. Open the `.cbp` project file using **Code::Blocks**.
3. **Important:** Ensure you have FreeGLUT set up in your Code::Blocks environment.
4. If you need to link the libraries manually, go to `Settings -> Compiler -> Linker Settings` and add:
   * `opengl32`
   * `glu32`
   * `freeglut`
5. Build and Run!

*(Note for Mac/Linux users: You can compile this via terminal using `g++ main.cpp -o app -lGL -lGLU -lglut`)*