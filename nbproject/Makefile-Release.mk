#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux
CND_DLIB_EXT=so
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/MediaManager.o \
	${OBJECTDIR}/Multimedia.o \
	${OBJECTDIR}/audio.o \
	${OBJECTDIR}/capture.o \
	${OBJECTDIR}/debug.o \
	${OBJECTDIR}/libavcodec_util.o \
	${OBJECTDIR}/main.o \
	${OBJECTDIR}/mypcm.o \
	${OBJECTDIR}/test-echo.o


# C Compiler Flags
CFLAGS=`cppunit-config --cflags` 

# CC Compiler Flags
CCFLAGS=`cppunit-config --cflags` 
CXXFLAGS=`cppunit-config --cflags` 

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=`cppunit-config --libs`  

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${TESTDIR}/TestFiles/f6

${TESTDIR}/TestFiles/f6: ${OBJECTFILES}
	${MKDIR} -p ${TESTDIR}/TestFiles
	${LINK.cc} -o ${TESTDIR}/TestFiles/f6 ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/MediaManager.o: MediaManager.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -I. -I. -I. -I. -I. -I. -I. -I. -I. -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/MediaManager.o MediaManager.cpp

${OBJECTDIR}/Multimedia.o: Multimedia.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -I. -I. -I. -I. -I. -I. -I. -I. -I. -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Multimedia.o Multimedia.cpp

${OBJECTDIR}/audio.o: audio.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -I. -I. -I. -I. -I. -I. -I. -I. -I. -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/audio.o audio.cpp

${OBJECTDIR}/capture.o: capture.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -I. -I. -I. -I. -I. -I. -I. -I. -I. -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/capture.o capture.cpp

${OBJECTDIR}/debug.o: debug.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -I. -I. -I. -I. -I. -I. -I. -I. -I. -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/debug.o debug.cpp

${OBJECTDIR}/libavcodec_util.o: libavcodec_util.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -I. -I. -I. -I. -I. -I. -I. -I. -I. -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libavcodec_util.o libavcodec_util.cpp

${OBJECTDIR}/main.o: main.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -I. -I. -I. -I. -I. -I. -I. -I. -I. -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.cpp

${OBJECTDIR}/mypcm.o: mypcm.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -I. -I. -I. -I. -I. -I. -I. -I. -I. -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/mypcm.o mypcm.cpp

${OBJECTDIR}/test-echo.o: test-echo.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -I. -I. -I. -I. -I. -I. -I. -I. -I. -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test-echo.o test-echo.c

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${TESTDIR}/TestFiles/f6

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
