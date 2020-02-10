#!/bin/bash

cd ../
./build
cd -
./copyBinariesToLive.sh
./copyLibsToLive.sh
cd $REALMFOLDER
./startContainer.sh
