# RobLy Code Editor

![image](https://github.com/ron3545/Code-Editor/assets/86136180/59aaf4c9-39b4-4179-a4fe-1e414b56f94c)



https://github.com/ron3545/Code-Editor/assets/86136180/9e385664-3607-48e1-a6a7-000e7c6b014b

## Install and Configure
### On Linux/WSL2
Install dependencies using [vcpkg - (VC++ Package Manager)](https://vcpkg.io/en/index.html) 

```bash
# Clone this repository with recursive option
git clone --recursive https://github.com/ron3545/Code-Editor.git
cd Code-Editor

# Run the bootstrap script for vcpkg
./vcpkg/bootstrap-vcpkg.sh

# Install dependencies required (will add to manifest later)
sudo apt install libxinerama-dev libxcursor-dev xorg-dev libglu1-mesa-dev pkg-config

# Install GLFW
sudo apt install libglfw3-dev -y

# Install OpenGL for WSL (if it complains during compilation)
sudo apt install mesa-utils libglu1-mesa-dev freeglut3-dev mesa-common-dev


# Install imgui dependencies using vcpkg manifest (vcpkg.json)
./vcpkg/vcpkg install 

# Compile and Run (or run ./linux-build.sh)
cmake . -B build/ 
cmake --build build
./build/src/RobLy_Core 

```
That's it!

## Setup ROS Melodic and MoveIt on Ubuntu 18.04
```
su root 
nano /etc/sudoers

<username> user_name ALL=(ALL)  ALL
```
```
sudo sh -c 'echo "deb http://packages.ros.org/ros/ubuntu $(lsb_release -sc) main" > /etc/apt/sources.list.d/ros-latest.list'
sudo apt install curl # if you haven't already installed curl
curl -s https://raw.githubusercontent.com/ros/rosdistro/master/ros.asc | sudo apt-key add -
sudo apt update

sudo apt install ros-melodic-desktop 
echo "source /opt/ros/melodic/setup.bash" >> ~/.bashrc
source ~/.bashrc

sudo su 
apt install python-rosdep2 
apt-get update
apt-get dist-upgrade
```
in new terminal: 
```
sudo apt-get install ros-melodic-catkin python-catkin-tools 
sudo apt install ros-melodic-moveit
source /opt/ros/melodic/setup.bash 
sudo apt-get install ros-melodic-moveit ros-melodic-moveit-visual-tools 

sudo apt-get install git
git clone https://github.com/ArctosRobotics/ROS

cd ROS 
catkin build 
source devel/setup.bash 

nano ~/.bashrc
	#add this line: 
	source /home/<your username>/ROS/devel/setup.bash 

cd
```


```
sudo apt update
sudo apt install python3-pip
pip3 install python-can[serial]
sudo apt-get install python3-tk
pip3 install sv-ttk
 ```
```
git clone https://github.com/ArctosRobotics/arctosgui


pip3 install ttkthemes

sudo apt install python3-rosdep python3-rosinstall-generator python3-wstool build-essential 

# optional part
sudo apt install python3-rosinstall python3-catkin-tools python3-osrf-pycommon

sudo apt-get install ros-melodic-robot-state-publisher 
sudo apt-get install ros-melodic-joint-state-publisher 

# make sure the arduino mega is pluged in before executing this command. This is my usb port name on my device. Yours can be different
sudo chmod a+rw /dev/ttyUSB1

# make sure this is updated before running RVIZ and gui
sudo rosdep init
rosdep update

cd arctosgui 
ls 
chmod +x run.sh 
ls # rin.sh should turn green 
./run.sh 
```
4 tabs will open 
you can manually open them by: 
```
roslaunch arctos_config demo.launch
rosrun moveo_moveit interface.py 
rosrun moveo_moveit transform.py 
python3 ui.py 
```

if rosrun causing command not found. do this:
```
sudo apt install ros-melodic-rosbash
```

Wait for the gui and rviz to show 

In moveit rviz go File>Open config or Ctrl+O and open 
arctosgui_config.rviz

Connect the robot 

If you have other port than /dev/ttyACM0 edit files 
send.py and ros.py to adress your specific port 

Plan new toolpath by moving joints or tool
Run ROS button will send CAN messages from new pose 

Run RoboDK will send gcode.tap file to robot 
Make sure that you copy the gcode from RoboDK post processor to gcode.tap or adapt it to export code to arctos gui location under the name gcode.tap and replace it. 

Set gear ratios in convert.py and roscan.py 
gear_ratios = [1, 1, 1, 1, 1, 1]  # Replace with your actual gearbox ratios

Raw gear ratios. 
X  13.5
Y  150
Z  150
A  48
B  67.82
C  67.82

In theory raw gear ratios should be multiplied to 0.5, so gear_ratios would be
[6.75, 75, 75, 24, 33.91, 33.91]
They are not tested! 

# Change this according to your folder 
```
user_path = "/home/ee/arctosgui" 
```

you can also refer to [this video](https://www.youtube.com/watch?v=R71iVkeIhtA&t=141s) or [this github repo](https://github.com/ArctosRobotics/arctosgui/blob/main/README.md?plain=1).


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
- [Dear ImGui](https://github.com/ocornut/imgui)  
- [GLFW] (https://www.glfw.org/)  
- [OpenGL](https://www.opengl.org/) 

## Contributing
Contributions to the ArmSimPro Code Editor are welcome. Feel free to submit issues, pull requests, or feature requests to enhance the editor's functionality and usability.
