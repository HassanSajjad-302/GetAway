# GetAway
This C++ project consists of 2 LAN/online 
multiplayer command-line/terminal based games. It's written on top of Boost 
Asio(asio standalone). It's easily extensible. You can extend this
project with another command-line online multiplayer game. It very quickly brings you to the point of 
writing core game logic.

This project consists of following 2 games currently:  
1)GetAway   2)Bluff  
GetAway is also known as Thulla in India, Pakistan where it is quite popular.

I also built the android app using NDK. Project [link](https://github.com/HassanSajjad-302/GetAwayAndroid.git)

## Screenshots
![](https://drive.google.com/uc?export=view&id=1Qw8b_GMELB8hnEdyWHhFb02mTLernFXP)
![](https://drive.google.com/uc?export=view&id=1UwrPB9YkhQPMIXaY3obhv-RDKdfh6ZFH)
![](https://drive.google.com/uc?export=view&id=1QRCgOJ8Q7YoAy2Pkh4g6K6fmBDOVB2fU)
![](https://drive.google.com/uc?export=view&id=1alvYXORVRKLPXCLCSrVgY9DYEeji9hao)
![](https://drive.google.com/uc?export=view&id=1DalEy0hjiZ9JUaUYzuwFuoXOjOjBjJeS)
![](https://drive.google.com/uc?export=view&id=1YvnbPBaBOB1FuVGLIsMjiQfeOQJLQSOt)
![](https://drive.google.com/uc?export=view&id=1inmOijzk7GNNoDR4xe5ZDkURtrP7WTU9)
![](https://drive.google.com/uc?export=view&id=1pr1mnBkuDavyweHUwRJFJJGCfTYtXChp)
![](https://drive.google.com/uc?export=view&id=1dqoG63mnTNL1v-SVbQ0GAjOup1QzQUnc)
![](https://drive.google.com/uc?export=view&id=1eZadaocpubUTgqx9odvXIdw_xkdQFzFo)
![](https://drive.google.com/uc?export=view&id=1k7AqAX8Rqx4HmdFjEDl0qsGwsZLSTS0N)

## How To Build The Project 

### Linux
```shell
git clone --recurse-submodules https://github.com/HassanSajjad-302/GetAway.git
cd GetAway
mkdir Build
cd Build
cmake ../
make
```

### Windows
Install CMake, then configure and generate the project for your 
build system. Then compile the program using your build system. 

## Extending The Project
To extend this with your own game, supposedly named Foo, you need
to add following 5 files:  
serverFoo.hpp  
serverFoo.cpp  
clientFoo.hpp  
clientFoo.cpp  
clientFooPF.cpp  
Header files goes in GetAway/header/ while source files goes in 
GetAway/src/. If project expands, I will separate files of each game
in their respective directory and add a CMake target for them.

To get the idea of what goes in these files, please read the
comments in each of the respective similar files for Bluff Game.
i.e.  
[serverBluff.hpp](GetAway/header/serverBluff.hpp)  
[serverBluff.cpp](GetAway/src/serverBluff.cpp)  
[clientBluff.hpp](GetAway/header/clientBluff.hpp)  
[clientBluff.cpp](GetAway/src/clientBluff.cpp)  

Above files have the core logic of the game. While the code for 
client server setup, finding servers on local wifi, in-game chat,
game packet sending and receiving, asynchronous clearance and 
redisplay of screen/text-area goes in other files. Because of 
loose coupling, if you want to extend this project with your 
console based game, you only need to provide these files, with some
small modifications in following files which are explained in those
respective files:  
[constants.hpp](GetAway/header/constants.hpp)  
[inputType.hpp](GetAway/header/inputType.hpp)  
[clientLobby.hpp](GetAway/header/clientLobby.hpp)  
[clientLobby.cpp](GetAway/src/clientLobby.cpp)  
[serverLobby.hpp](GetAway/header/serverLobby.hpp)  
[serverLobby.cpp](GetAway/src/serverLobby.cpp)


### Debugging Your Game
If you want to quickly debug your game, instead of starting 
server, then joining server, then starting game, you need to
define an Object-Like macro in [main.cpp](GetAway/main.cpp). Then 
you can quickly debug the core game. When finished, just undefine 
the macro. Read more in main.cpp.