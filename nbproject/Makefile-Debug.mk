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
CND_CONF=Debug
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
CCFLAGS=`cppunit-config --cflags` -D__STDC_CONSTANT_MACROS 
CXXFLAGS=`cppunit-config --cflags` -D__STDC_CONSTANT_MACROS 

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-lxml2 -lpthread -lssl -lcrypto -lz `cppunit-config --libs` `pkg-config --cflags --libs libv4l2` -lwebsockets -lbase -lblkid -lpulse-simple -lpulse -lavcodec -lavutil -lasound -lva -lswresample  

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/ferryport

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/ferryport: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/ferryport ${OBJECTFILES} ${LDLIBSOPTIONS} -Wall

${OBJECTDIR}/MediaManager.o: MediaManager.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -D_DEBUG -I/usr/include/libxml2 -I. -I/usr/local/include/ferryfair `pkg-config --cflags --cflags libv4l2` -std=c++11  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/MediaManager.o MediaManager.cpp

${OBJECTDIR}/Multimedia.o: Multimedia.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -D_DEBUG -I/usr/include/libxml2 -I. -I/usr/local/include/ferryfair `pkg-config --cflags --cflags libv4l2` -std=c++11  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Multimedia.o Multimedia.cpp

${OBJECTDIR}/audio.o: audio.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -D_DEBUG -I/usr/include/libxml2 -I. -I/usr/local/include/ferryfair `pkg-config --cflags --cflags libv4l2` -std=c++11  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/audio.o audio.cpp

${OBJECTDIR}/capture.o: capture.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -D_DEBUG -I/usr/include/libxml2 -I. -I/usr/local/include/ferryfair `pkg-config --cflags --cflags libv4l2` -std=c++11  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/capture.o capture.cpp

${OBJECTDIR}/debug.o: debug.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -D_DEBUG -I/usr/include/libxml2 -I. -I/usr/local/include/ferryfair `pkg-config --cflags --cflags libv4l2` -std=c++11  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/debug.o debug.cpp

${OBJECTDIR}/libavcodec_util.o: libavcodec_util.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -D_DEBUG -I/usr/include/libxml2 -I. -I/usr/local/include/ferryfair `pkg-config --cflags --cflags libv4l2` -std=c++11  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libavcodec_util.o libavcodec_util.cpp

${OBJECTDIR}/main.o: main.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -D_DEBUG -I/usr/include/libxml2 -I. -I/usr/local/include/ferryfair `pkg-config --cflags --cflags libv4l2` -std=c++11  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.cpp

${OBJECTDIR}/mypcm.o: mypcm.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -D_DEBUG -I/usr/include/libxml2 -I. -I/usr/local/include/ferryfair `pkg-config --cflags --cflags libv4l2` -std=c++11  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/mypcm.o mypcm.cpp

${OBJECTDIR}/test-echo.o: test-echo.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -I. `pkg-config --cflags --cflags libv4l2`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test-echo.o test-echo.c

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/ferryport

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
