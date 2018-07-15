# Simulator

The simulator project should enable you to iterate and test your code faster.
It is designed to simuate the TinyScreen Video Game Kit. It provides certain
basic libraries that are used by the tiny screen library and the arduino
projects are compiled against these. It can be used to emulate 

It's using GLFW 3 for displaying the TinyScreen's content. 

The code is a fork from zet23t's TinyDuino 2 Play Lib (https://github.com/zet23t/td2play).
This is focused solely on a simulator.

# How to Use

Install the glfw3 library. Under fedora this is simply "dnf install glfw-devel".
To compile an arduino program run the tinyscreensim command. The first argument
is the output file. The second argument is the .ino file. This is followed by
any additional C++/C files. For example:

../tinyscreensim/tinyscreensim viobyte viobyte.ino Sprite.cpp

Many programs will not work directly with the simulator. The arduino compile
allows invalid C++ where functions are used before they are defined. In order
to compile TinyArcade programs some small modifications are needed to support
the simulator. A simple example program is provided called colortest.

If you want to be able to take a screenshot at actual screen size instead of
the enlarged display, you can add --png as the first argument to tinyscreensim.
Boost is required to enable this feature.

# Hotkeys

Arrow keys: joystick controll

- G/H: Button 1 and 2 of tiny arcade
- W/E/S/D: Screen buttons
- R: Record session to TSV file
- P: Take a screenshot (must be enabled with --png)

# Approach

The simulator mimics certain aspects of the TinyDuino Screen+ library and 
some other parts that come with Arduino. 

# Bugs

Fonts and print commands are not implemented. Many other features are
implemented.
