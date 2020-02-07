#!/bin/bash

cp $SERVER_BUILD/router $REALMFOLDER/dngrouter
chmod +x $REALMFOLDER/router
cp $SERVER_BUILD/updatesrv $REALMFOLDER/dngupdate
chmod +x $REALMFOLDER/update
cp $SERVER_BUILD/datamgr $REALMFOLDER/dngdatamgr
chmod +x $REALMFOLDER/datamgr
cp $SERVER_BUILD/roommgr $REALMFOLDER/dawn/bin/main
chmod +x $REALMFOLDER/dawn/bin/main
