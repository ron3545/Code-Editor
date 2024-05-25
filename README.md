# RobLy Code Editor

![image](https://github.com/ron3545/Code-Editor/assets/86136180/59aaf4c9-39b4-4179-a4fe-1e414b56f94c)



https://github.com/ron3545/Code-Editor/assets/86136180/9e385664-3607-48e1-a6a7-000e7c6b014b

## Install and Configure
### On Linux/WSL2
Install dependencies using [vcpkg - (VC++ Package Manager)](https://vcpkg.io/en/index.html) 

```bash
# Clone this repository with recursive option
$ git clone --recursive https://github.com/ron3545/Code-Editor.git
$ cd Code-Editor

# Run the bootstrap script for vcpkg
$ ./vcpkg/bootstrap-vcpkg.sh

# Install dependencies required (will add to manifest later)
$ sudo apt install libxinerama-dev libxcursor-dev xorg-dev libglu1-mesa-dev pkg-config

# Install GLFW
$ sudo apt install libglfw3-dev -y

# Install OpenGL for WSL (if it complains during compilation)
$ sudo apt install mesa-utils libglu1-mesa-dev freeglut3-dev mesa-common-dev


# Install imgui dependencies using vcpkg manifest (vcpkg.json)
$ ./vcpkg/vcpkg install 

# Compile and Run (or run ./linux-build.sh)
$ cmake . -B build/ 
$ cmake --build build
$ ./build/src/RobLy_Core 

```
That's it!

## Installing Catkin on Ubuntu
First you must have the ROS repositories which contain the .deb for catkin_tools:\

```
$ sudo sh \
    -c 'echo "deb http://packages.ros.org/ros/ubuntu `lsb_release -sc` main" \
        > /etc/apt/sources.list.d/ros-latest.list'
$ wget http://packages.ros.org/ros.key -O - | sudo apt-key add -
```

Once you have added that repository, run these commands to install catkin_tools:

```
$ sudo apt-get update
$ sudo apt-get install python3-catkin-tools
```

Setting Up
```
sudo su 
apt install python-rosdep2 
apt-get-update
apt-get dist-upgrade
```

in new terminal:
```
sudo pip3 install -U catkin_tools
sudo apt-get install ros-iron-moveit
source /opt/ros/iron/setup.bash
sudo apt-get install ros-iron-moveit ros-iron-moveit-visual-tools
```

## Overview
The ArmSimPro Code Editor is a versatile text editor designed specifically for programming microcontrollers that control a 6 Degrees of Freedom (6 DOF) robotic arm kit for kids. It provides a user-friendly interface for writing, editing, and debugging code related to the control and movement of the robotic arm.

## Note:
I usually develop on my own, so there could be bad design patterns and bugs that you may see. If you see some bugs or bad design paterns don't hesitate to post an issue. 

- The software needs boost libraries. You can download it here: https://github.com/boostorg/cmake

## Features
#### Code Editing
- Syntax highlighting for various programming languages, facilitating easy code reading and editing.
- Support for multiple file types commonly used in microcontroller programming.

#### Navigation
- Line numbering and markers for easy navigation and identification of specific code sections.
- Cursor positioning and selection tools to streamline code navigation.

#### Visual Representation
- Dynamic rendering of code with ImGui, providing a visually appealing and interactive environment.
- Adjustable palette and theming options for a personalized coding experience.

#### Language Support
- Language definition configurations for C++ and C languages.
- Customizable language definitions for additional languages, enhancing versatility.

#### Text Manipulation
- Basic text manipulation operations such as copy, cut, paste, delete, undo, and redo.
- Word selection and line selection modes for efficient text manipulation.

#### Customization
- Adjustable tab size and line spacing to cater to individual preferences.
- Palette customization for code highlighting, aiding readability.

## Usage
1. Installation: Include the provided header file in your project to integrate the ArmSimPro Code Editor.

2. Configuration: Choose the appropriate language definition for your programming language and customize the editor's appearance using palette and theming options.

3. Code Editing: Use the editor to write, edit, and debug microcontroller code for a 6 DOF robotic arm. Take advantage of the syntax highlighting and navigation features for efficient coding.

4. Visual Representation: Leverage the dynamic rendering capabilities of ImGui to visualize your code in an interactive manner.

# Dear ImGui with GLFW + OpenGL3

[**Dear ImGui**](https://github.com/ocornut/imgui) is a bloat-free graphical user interface library for C++. It outputs optimized vertex buffers that you can render anytime in your 3D-pipeline-enabled application. It is fast, portable, renderer agnostic, and self-contained (no external dependencies).

*This sample uses Dear ImGui with GLFW + OpenGL3*

[**GLFW**](https://www.glfw.org/) (Graphics Library Framework) is an Open Source, multi-platform library for OpenGL, OpenGL ES and Vulkan development on the desktop. It provides a simple API for creating windows, contexts and surfaces, receiving input and events.

[**OpenGL**](https://www.opengl.org/) (Open Graphics Library) is a cross-language, cross-platform application programming interface (API) for rendering 2D and 3D vector graphics. The API is typically used to interact with a graphics processing unit (GPU), to achieve hardware-accelerated rendering.

## Docs and Helpful links
- Live Documentation [here](https://pthom.github.io/imgui_manual_online/manual/imgui_manual.html)
- Docs on Examples [here](https://github.com/ocornut/imgui/blob/master/docs/EXAMPLES.md)
- Wiki with examples and usage [here](https://github.com/ocornut/imgui/wiki)
- Useful Extensions [here](https://github.com/ocornut/imgui/wiki/Useful-Extensions)
## Credits
- Dear ImGui - https://github.com/ocornut/imgui  
- GLFW - https://www.glfw.org/  
- OpenGL - https://www.opengl.org/  

## Contributing
Contributions to the ArmSimPro Code Editor are welcome. Feel free to submit issues, pull requests, or feature requests to enhance the editor's functionality and usability.
