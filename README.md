# Dwarves and Giants (a Sacred Temple Fork) Server Source Code

This contains the source code for the private server Sacred Temple, adapted to be used with Dwarves and Giants. This is currently a private project, and as such the code should not be distributed any way other than through this private GitHub repository. 

### Directory Structure
* data - Directory that contains separately generated data files for items, npcs, worlds, etc. Generally maintained in a separate repo with stock objects here.
* datamgr - The C++ service that connects to the MySql database for the roommgr service.
* router - The C++ service that routers users to the update server and to servers loaded in the database.
* global - Custom global library for functionality that is common across services (Packets, etc)
* roommgr - The C++ service that maintains the state of the server, and contains the logic to the game.
* tokenize - Custom program to convert data files to compiled code. Allows for creation of content through a formatted document (i.e Sierra's version of XML, JSON, etc)
* updates - The C++ service that serves up the rtp patch files to the user. It maintains state of versioning on the server side.

### Instructions to Compile

Tested on either physical machine or Virtual Machine. Obtain an image of Ubuntu 18.04.xx 64Bit Desktop, and install it to either a secondary machine, second partition/drive, or virtual machine. Once on the system, take the source code and drop it in `~/home/cmake/` so the directory should read something like `~/home/cmake/source/`. Install VS Code via the Ubuntu Software Center; search `VS Code` and install Visual Studio Code. Then, open a terminal and enter the following commands; `sudo apt get install cmake` `sudo apt get install gcc` `sudo apt get install g++` `sudo apt get install make`. Y to install all the packages when asked. Once these are all installed, open VS Code and then inside VS Code open the `~/home/cmake/source/` folder. On the top menu bar do, `Terminal->New Terminal` a terminal should open at the bottom with the output `~/cmake/source$ ` type; `chmod 755 *` now you are ready to build. Type; `./buildonly` to build the server. There *should* be errors. As long as your roommgr fully builds, it *should* work.


**Note: You don't actually need to use VS Code to build it. Will work fine from a terminal pointed at ~/cmake/source/ using ./buildonly
VS Code is only necessary to make changed to the code**
