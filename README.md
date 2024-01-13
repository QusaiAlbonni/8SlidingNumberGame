## SlidingNumbers

SlidingNumbers game, made with modern OpenGL (4.6)

![Demo](./images/demo.png)


## Building From Source

First, you should have "Desktop Development with C++" workload installed (via [visual studio installer](https://learn.microsoft.com/en-us/cpp/build/vscpp-step-0-installation?view=msvc-170#step-4---choose-workloads)).

Next, make sure powershell is recognized by cmd,
- Open cmd
- Run `pwsh --version`

Finally, building the project is straight foreword,
- Clone this repo, 
- Open .sln file with visual studio 2022 
- Run.

## Libraries Used

- Glfw: Creates OpenGL window
- Glad: Loads OpenGL functions
- SFML: Plays audio files
- stb: Loads image files
- ImGUI: Adds Buttons to glfw window
- GLM: Serves OpenGL mathematics