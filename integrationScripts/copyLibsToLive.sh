#!/bin/bash

mv $REALMFOLDER/dawn/data $REALMFOLDER/dawn/previous_data
cp -r $SERVER_SOURCE/data $REALMFOLDER/dawn/data
