# Sacred Temple Server Source Code

This contains the source code for the private server Sacred Temple. This is currently a private project, and as such the code should not be distributed any way other than through this private GitHub repository. 

### Directory Structure
* data - Directory that contains separately generated data files for items, npcs, worlds, etc. Generally maintained in a separate repo with stock objects here.
* datamgr - The C++ service that connects to the MySql database for the roommgr service.
* router - The C++ service that routers users to the update server and to servers loaded in the database.
* global - Custom global library for functionality that is common across services (Packets, etc)
* roommgr - The C++ service that maintains the state of the server, and contains the logic to the game.
* tokenize - Custom program to convert data files to compiled code. Allows for creation of content through a formatted document (i.e Sierra's version of XML, JSON, etc)
* updates - The C++ service that serves up the rtp patch files to the user. It maintains state of versioning on the server side.