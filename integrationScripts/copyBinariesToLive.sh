#!/bin/bash

cp -rf $SERVER_BUILD/router $REALMFOLDER/router/dngrouter
chmod +x $REALMFOLDER/router
cp -rf $SERVER_BUILD/updatesrv $REALMFOLDER/update/dngupdate
chmod +x $REALMFOLDER/update
cp -rf $SERVER_BUILD/datamgr $REALMFOLDER/datamgr/dngdatamgr
chmod +x $REALMFOLDER/datamgr
cp -rf $SERVER_BUILD/roommgr $REALMFOLDER/dawn/bin/main
chmod +x $REALMFOLDER/dawn/bin/main
