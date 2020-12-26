#!/bin/bash
# Rev: $Id: kpq3ded.sh,v 1.9 2006/01/18 13:47:42 raistlin Exp raistlin $
# Needed to make symlinks/shortcuts work.
# the binaries must run with correct working directory

KPQ3_DIR=/usr/local/share/games/kingpinq3

COMPILE_PLATFORM=`uname|sed -e s/_.*//|tr '[:upper:]' '[:lower:]'`
COMPILE_ARCH=`uname -p | sed -e s/i.86/i386/`

EXEC_REL=release

# EXEC_BIN=kingpinq3.${COMPILE_ARCH}
# EXEC_BIN=kingpinq3-smp.${COMPILE_ARCH}
EXEC_BIN=kpq3ded.${COMPILE_ARCH}

EXEC_FLAGS="+set fs_basepath ${KPQ3_DIR} +set vm_game 1 +set vm_cgame 1 +set vm_ui 1 +set sv_pure 1 +set ttycon 0 +set com_ansiColor 0"

EXEC_DIR_LIST="${KPQ3_DIR}"

for d in ${EXEC_DIR_LIST}
do
	if [ -d $d ]; then
		EXEC_DIR=${d}
		break
	fi
done

if [ "X${EXEC_DIR}" != "X" ]; then
	if [ ! -x  ${EXEC_DIR}/${EXEC_BIN} ]; then
		echo "Executable ${EXEC_DIR}/${EXEC_BIN} not found!" ; exit 1
	fi
	cd ${KPQ3_DIR} && \
	${EXEC_DIR}/${EXEC_BIN} ${EXEC_FLAGS} $*
	exit $? 
else
	echo "No kpq3 binaries found!"
	exit 1
fi
  

