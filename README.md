# JDebug
Debugger For The Jolt Physics Engine.
# Warnings

Very Experimental, Not Ready For Production.

# Why Create A "Visual Debugger"?
Well, I Have 3 Core Reasons:

1: Jolt Doesn't have a Fleshed debugger. Despite saving binary "Playbacks" of the physics world, it isn't very verbose when compared other debuggers like Havok's Debugger.

2: I Want/Wanted to learn more about the physics engine.  I have tinkered around with
other physics engines(PhysX, Bullet3,Havok) but never really liked how the two were not very extendable.  

# Building 

The project uses premake5(https://premake.github.io/). just open the project in the project directory, and build. example: 'premake5 vs2022' for windows.
The project ALSO Uses QT6.4(For GUI). the best option is to install QT Through aqt(UNOFFICIAL QT CLI INSTALLER, https://github.com/miurahr/aqtinstall). it's very easy and nice to work with and installs in seconds.

# GUI Info

The GUI Will Currently Use D3D12. Im Planning to add Vulkan Or OpenGL Support Later down the road, but we will see how it goes.
