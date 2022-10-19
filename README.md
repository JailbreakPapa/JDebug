# JDebug
Debugger For The Jolt Physics Engine.
#Warnings

Very Expermental, Not Ready For Production.

#Why Create A "Debugger"?
Well I Have 3 Core Reasons:

1: Jolt Doesn't have a Fleshed out debugger. dispite saving binary "Playbacks" of the physics world, it isnt very usable for engines/applications to use.

2: I Want/Wanted to learn more about the physics engine. it is still a fresh, physics engine, that is very new, and has space to grow. i have tinkered around with
other physics engines(PhysX,Bullet3) but never really liked how the two were not very extendable. 

3: I Want a ACTUAL Project under my belt. 

#Building 

The project uses premake5(https://premake.github.io/). just open the project in the project directory, and build. example: 'premake5 vs2022' for windows.
The project ALSO Uses QT5.12.2(For GUI). best option is to install QT Through aqt(UNOFFICAL QT CLI INSTALLER, https://github.com/miurahr/aqtinstall). its very easy and nice to work with, and installs in seconds.
