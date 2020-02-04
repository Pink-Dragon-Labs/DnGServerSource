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

# Using the Docker Image

This image uses the DnGServer image as a base image. For more information about running the server itself, view the
README.md at that repo.

This will target the building of new binaries from source, and using them in the servers.

By default, when running running the image, it will immediately build all binaries, copy them to the server directory,
and start the servers. It will also provide a test user with both a username and password of `test`.

Really, thats all there is to it. To make modifications to the source, edit the source OUTSIDE of the docker, and then
rebuild the image with:

```sh
# TAG_NAME can be any name you want to give it
docker image build -f dockerfile -t TAG_NAME .
```

Then, run the server with 
```sh
# CONTAINER_NAME can be anything you want
#TAG_NAME must match what you built the image with above
docker container run --name CONTAINER_NAME -h realmserver -it TAG_NAME
```
NOTE: that will create a container that you can only connect to from the host machine. To make the server accessible
externally, view the `docker network` instructions in the README.md of the DnGServer repository.


