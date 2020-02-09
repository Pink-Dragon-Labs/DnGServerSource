cmake_minimum_required (VERSION 2.6)
project (ROOMMGR)

INCLUDE_DIRECTORIES( ${MYSQL_INCLUDE_DIR} )

add_definitions(-std=c++11)
#add_definitions(-fpermissive)
add_definitions(-Wno-write-strings)
add_definitions(-Wno-unused-result)
add_definitions(-ggdb)
add_definitions(-fpermissive)
#add_definitions(-Wall)
add_definitions(-O3)

set(FILEPATH_ROOMMGR ${CMAKE_CURRENT_SOURCE_DIR} )

set(SRC_ROOMMGR
    ${FILEPATH_ROOMMGR}/mlbandit.cpp
    ${FILEPATH_ROOMMGR}/mltraveller.cpp
    ${FILEPATH_ROOMMGR}/mlxerxes.cpp
    ${FILEPATH_ROOMMGR}/mlhousepet.cpp
    ${FILEPATH_ROOMMGR}/mlcastleboss.cpp
    ${FILEPATH_ROOMMGR}/mlcthief.cpp
    ${FILEPATH_ROOMMGR}/mlcwiz.cpp
    ${FILEPATH_ROOMMGR}/mlcguard.cpp
    ${FILEPATH_ROOMMGR}/mlgonan.cpp
    ${FILEPATH_ROOMMGR}/mlgonanpet.cpp
    ${FILEPATH_ROOMMGR}/mldragon.cpp
    ${FILEPATH_ROOMMGR}/mlbmystic.cpp
    ${FILEPATH_ROOMMGR}/mlswashbuckler.cpp
    ${FILEPATH_ROOMMGR}/mlgodprot.cpp
    ${FILEPATH_ROOMMGR}/mlgodsewer.cpp
    ${FILEPATH_ROOMMGR}/main.cpp
    ${FILEPATH_ROOMMGR}/bcarry.cpp
    ${FILEPATH_ROOMMGR}/bcontain.cpp
    ${FILEPATH_ROOMMGR}/bplayer.cpp
    ${FILEPATH_ROOMMGR}/bhead.cpp
    ${FILEPATH_ROOMMGR}/bwear.cpp
    ${FILEPATH_ROOMMGR}/bopen.cpp
    ${FILEPATH_ROOMMGR}/bdescribed.cpp
    ${FILEPATH_ROOMMGR}/bcycle.cpp
    ${FILEPATH_ROOMMGR}/bcharacter.cpp
    ${FILEPATH_ROOMMGR}/block.cpp
    ${FILEPATH_ROOMMGR}/bkey.cpp
    ${FILEPATH_ROOMMGR}/bweapon.cpp
    ${FILEPATH_ROOMMGR}/bentry.cpp
    ${FILEPATH_ROOMMGR}/bshop.cpp
    ${FILEPATH_ROOMMGR}/bnpc.cpp
    ${FILEPATH_ROOMMGR}/bconsume.cpp
    ${FILEPATH_ROOMMGR}/bpassword.cpp
    ${FILEPATH_ROOMMGR}/bgate.cpp
    ${FILEPATH_ROOMMGR}/bsit.cpp
    ${FILEPATH_ROOMMGR}/bscroll.cpp
    ${FILEPATH_ROOMMGR}/btalk.cpp
    ${FILEPATH_ROOMMGR}/buse.cpp
    ${FILEPATH_ROOMMGR}/bmix.cpp
    ${FILEPATH_ROOMMGR}/bswitch.cpp
    ${FILEPATH_ROOMMGR}/btreasure.cpp
    ${FILEPATH_ROOMMGR}/bdye.cpp
    ${FILEPATH_ROOMMGR}/bspellbag.cpp
    ${FILEPATH_ROOMMGR}/talktree.cpp
    ${FILEPATH_ROOMMGR}/wobject.cpp
    ${FILEPATH_ROOMMGR}/rmparser.cpp
    ${FILEPATH_ROOMMGR}/rmroom.cpp
    ${FILEPATH_ROOMMGR}/rmplayer.cpp
    ${FILEPATH_ROOMMGR}/timesys.cpp
    ${FILEPATH_ROOMMGR}/worldlib.cpp
    ${FILEPATH_ROOMMGR}/zone.cpp
    ${FILEPATH_ROOMMGR}/misc.cpp
    ${FILEPATH_ROOMMGR}/tables.cpp
    ${FILEPATH_ROOMMGR}/dice.cpp
    ${FILEPATH_ROOMMGR}/wobjectbase.cpp
    ${FILEPATH_ROOMMGR}/properties.cpp
    ${FILEPATH_ROOMMGR}/characters.cpp
    ${FILEPATH_ROOMMGR}/house.cpp
    ${FILEPATH_ROOMMGR}/actions.cpp
    ${FILEPATH_ROOMMGR}/combat.cpp
    ${FILEPATH_ROOMMGR}/magic.cpp
    ${FILEPATH_ROOMMGR}/rmtools.cpp
    ${FILEPATH_ROOMMGR}/npc.cpp
    ${FILEPATH_ROOMMGR}/mlmarvin.cpp
    ${FILEPATH_ROOMMGR}/mlseraph.cpp
    ${FILEPATH_ROOMMGR}/mlguardian.cpp
    ${FILEPATH_ROOMMGR}/mlfaery.cpp
    ${FILEPATH_ROOMMGR}/mlmagis.cpp
    ${FILEPATH_ROOMMGR}/mltulor.cpp
    ${FILEPATH_ROOMMGR}/mldemon.cpp
    ${FILEPATH_ROOMMGR}/mlmonstr.cpp
    ${FILEPATH_ROOMMGR}/mlgood.cpp
    ${FILEPATH_ROOMMGR}/mleelem.cpp
    ${FILEPATH_ROOMMGR}/mlfelem.cpp
    ${FILEPATH_ROOMMGR}/mlwelem.cpp
    ${FILEPATH_ROOMMGR}/mlaelem.cpp
    ${FILEPATH_ROOMMGR}/mlnecro.cpp
    ${FILEPATH_ROOMMGR}/mlogrema.cpp
    ${FILEPATH_ROOMMGR}/mltroll.cpp
    ${FILEPATH_ROOMMGR}/channel.cpp
    ${FILEPATH_ROOMMGR}/squirmy.cpp
    ${FILEPATH_ROOMMGR}/quest.cpp
    ${FILEPATH_ROOMMGR}/recipe.cpp
    ${FILEPATH_ROOMMGR}/mllich.cpp
    ${FILEPATH_ROOMMGR}/mlimp.cpp
    ${FILEPATH_ROOMMGR}/mlthief.cpp
    ${FILEPATH_ROOMMGR}/mlsmarty.cpp
    ${FILEPATH_ROOMMGR}/mlhunter.cpp
    ${FILEPATH_ROOMMGR}/mlmouse.cpp
    ${FILEPATH_ROOMMGR}/mlfury.cpp
    ${FILEPATH_ROOMMGR}/mlpaladi.cpp
    ${FILEPATH_ROOMMGR}/mlcleric.cpp
    ${FILEPATH_ROOMMGR}/mllwiz.cpp
    ${FILEPATH_ROOMMGR}/mlstorm.cpp
    ${FILEPATH_ROOMMGR}/mlaction.cpp
    ${FILEPATH_ROOMMGR}/mlpwiz.cpp
    ${FILEPATH_ROOMMGR}/bountylist.cpp
    ${FILEPATH_ROOMMGR}/tracking.cpp
    ${FILEPATH_ROOMMGR}/audit.cpp
    ${FILEPATH_ROOMMGR}/globals.cpp
    ${FILEPATH_ROOMMGR}/callbacks.cpp
    ${FILEPATH_ROOMMGR}/login.cpp
    ${FILEPATH_ROOMMGR}/datamgrclient.cpp
    ${FILEPATH_ROOMMGR}/mlwatcher.cpp
    ${FILEPATH_ROOMMGR}/mlhellsoul.cpp
    ${FILEPATH_ROOMMGR}/mlberserk.cpp
    ${FILEPATH_ROOMMGR}/mlmistwiz.cpp
    ${FILEPATH_ROOMMGR}/ambushgroup.cpp
    ${FILEPATH_ROOMMGR}/friendmgr.cpp
    ${FILEPATH_ROOMMGR}/scriptmgr.cpp
    ${FILEPATH_ROOMMGR}/zonescripts.cpp
    ${FILEPATH_ROOMMGR}/playerstate.cpp
    ${FILEPATH_ROOMMGR}/mlminotaur.cpp
    ${FILEPATH_ROOMMGR}/mlminotaurguard.cpp
    ${FILEPATH_ROOMMGR}/mlpumpkindropper.cpp
  )

add_executable(roommgr ${SRC_ROOMMGR} )
TARGET_LINK_LIBRARIES( roommgr ${LIBRARY_OUTPUT_PATH}/libglobal.a ${MYSQL_CLIENT_LIBS} )

 