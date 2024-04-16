# ArmSimPro Code Editor

![image](https://github.com/ron3545/Code-Editor/assets/86136180/59aaf4c9-39b4-4179-a4fe-1e414b56f94c)



https://github.com/ron3545/Code-Editor/assets/86136180/9e385664-3607-48e1-a6a7-000e7c6b014b


## Clone using this command
```
git clone --recursive https://github.com/ron3545/Code-Editor.git
```
## Build on Release
```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
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

## Contributing
Contributions to the ArmSimPro Code Editor are welcome. Feel free to submit issues, pull requests, or feature requests to enhance the editor's functionality and usability.
