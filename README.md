# JDebug
Debugger For The Jolt Physics Engine.
# Warnings

Very Experimental, Not Ready For Production.

# Why Create A "Debugger"?
Well, I Have 3 Core Reasons:

1: Jolt Doesn't have a Fleshed debugger. Despite saving binary "Playbacks" of the physics world, it isn't very usable for engines/applications to use.

2: I Want/Wanted to learn more about the physics engine. it is still a fresh, physics engine, that is very new, and has space to grow. I have tinkered around with
other physics engines(PhysX, Bullet3) but never really liked how the two were not very extendable. 

3: I Want an ACTUAL Project under my belt. 

# Building 

The project uses premake5(https://premake.github.io/). just open the project in the project directory, and build. example: 'premake5 vs2022' for windows.
The project ALSO Uses QT5.12.2(For GUI). the best option is to install QT Through aqt(UNOFFICIAL QT CLI INSTALLER, https://github.com/miurahr/aqtinstall). it's very easy and nice to work with and installs in seconds.
