#!/bin/bash

cd ../
./build
cd -
./copyBinariesToLive.sh
./copyLibsToLive.sh
cd $REALMFOLDER
./startRealm.sh
