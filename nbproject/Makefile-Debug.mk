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
CND_PLATFORM=GNU-Linux-x86
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
	${OBJECTDIR}/ClientSocket.o \
	${OBJECTDIR}/MediaManager.o \
	${OBJECTDIR}/Multimedia.o \
	${OBJECTDIR}/ServerSocket.o \
	${OBJECTDIR}/Socket.o \
	${OBJECTDIR}/capture.o \
	${OBJECTDIR}/debug.o \
	${OBJECTDIR}/libavcodec_util.o \
	${OBJECTDIR}/main.o \
	${OBJECTDIR}/myconverters.o \
	${OBJECTDIR}/mycurl.o \
	${OBJECTDIR}/mypcm.o \
	${OBJECTDIR}/mystdlib.o \
	${OBJECTDIR}/myxml.o \
	${OBJECTDIR}/test-echo.o

# Test Directory
TESTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}/tests

# Test Files
TESTFILES= \
	${TESTDIR}/TestFiles/f2 \
	${TESTDIR}/TestFiles/f1 \
	${TESTDIR}/TestFiles/f3

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
LDLIBSOPTIONS=-L/usr/lib/i386-linux-gnu ffmpeg_build/lib/libavformat.a ffmpeg_build/lib/libavcodec.a ffmpeg_build/lib/libswscale.a ffmpeg_build/lib/libavdevice.a ffmpeg_build/lib/libavfilter.a ffmpeg_build/lib/libx264.a ffmpeg_build/lib/libfdk-aac.a ffmpeg_build/lib/libswresample.a ffmpeg_build/lib/libpostproc.a ffmpeg_build/lib/libavutil.a -lxml2 -lpthread -lssl -lcrypto -lwebsockets -lvorbis -lvpx -lmp3lame -lz -ldl -lvorbisenc -lva -ltheoraenc -ltheoradec -lopus `cppunit-config --libs` `pkg-config --cflags --libs libv4l2` -lasound -ludev  

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/remotedevicecontroller

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/remotedevicecontroller: ffmpeg_build/lib/libavformat.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/remotedevicecontroller: ffmpeg_build/lib/libavcodec.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/remotedevicecontroller: ffmpeg_build/lib/libswscale.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/remotedevicecontroller: ffmpeg_build/lib/libavdevice.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/remotedevicecontroller: ffmpeg_build/lib/libavfilter.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/remotedevicecontroller: ffmpeg_build/lib/libx264.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/remotedevicecontroller: ffmpeg_build/lib/libfdk-aac.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/remotedevicecontroller: ffmpeg_build/lib/libswresample.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/remotedevicecontroller: ffmpeg_build/lib/libpostproc.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/remotedevicecontroller: ffmpeg_build/lib/libavutil.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/remotedevicecontroller: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/remotedevicecontroller ${OBJECTFILES} ${LDLIBSOPTIONS} -Wall

${OBJECTDIR}/ClientSocket.o: ClientSocket.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -I/usr/include/libxml2 -I/usr/local/ffmpeg_build/include -I. `pkg-config --cflags --cflags libv4l2` -std=c++11  -MMD -MP -MF $@.d -o ${OBJECTDIR}/ClientSocket.o ClientSocket.cpp

${OBJECTDIR}/MediaManager.o: MediaManager.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -I/usr/include/libxml2 -I/usr/local/ffmpeg_build/include -I. `pkg-config --cflags --cflags libv4l2` -std=c++11  -MMD -MP -MF $@.d -o ${OBJECTDIR}/MediaManager.o MediaManager.cpp

${OBJECTDIR}/Multimedia.o: Multimedia.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -I/usr/include/libxml2 -I/usr/local/ffmpeg_build/include -I. `pkg-config --cflags --cflags libv4l2` -std=c++11  -MMD -MP -MF $@.d -o ${OBJECTDIR}/Multimedia.o Multimedia.cpp

${OBJECTDIR}/ServerSocket.o: ServerSocket.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -I/usr/include/libxml2 -I/usr/local/ffmpeg_build/include -I. `pkg-config --cflags --cflags libv4l2` -std=c++11  -MMD -MP -MF $@.d -o ${OBJECTDIR}/ServerSocket.o ServerSocket.cpp

${OBJECTDIR}/Socket.o: Socket.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -I/usr/include/libxml2 -I/usr/local/ffmpeg_build/include -I. `pkg-config --cflags --cflags libv4l2` -std=c++11  -MMD -MP -MF $@.d -o ${OBJECTDIR}/Socket.o Socket.cpp

${OBJECTDIR}/capture.o: capture.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -I/usr/include/libxml2 -I/usr/local/ffmpeg_build/include -I. `pkg-config --cflags --cflags libv4l2` -std=c++11  -MMD -MP -MF $@.d -o ${OBJECTDIR}/capture.o capture.cpp

${OBJECTDIR}/debug.o: debug.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -I/usr/include/libxml2 -I/usr/local/ffmpeg_build/include -I. `pkg-config --cflags --cflags libv4l2` -std=c++11  -MMD -MP -MF $@.d -o ${OBJECTDIR}/debug.o debug.cpp

${OBJECTDIR}/libavcodec_util.o: libavcodec_util.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -I/usr/include/libxml2 -I/usr/local/ffmpeg_build/include -I. `pkg-config --cflags --cflags libv4l2` -std=c++11  -MMD -MP -MF $@.d -o ${OBJECTDIR}/libavcodec_util.o libavcodec_util.cpp

${OBJECTDIR}/main.o: main.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -I/usr/include/libxml2 -I/usr/local/ffmpeg_build/include -I. `pkg-config --cflags --cflags libv4l2` -std=c++11  -MMD -MP -MF $@.d -o ${OBJECTDIR}/main.o main.cpp

${OBJECTDIR}/myconverters.o: myconverters.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -I/usr/include/libxml2 -I/usr/local/ffmpeg_build/include -I. `pkg-config --cflags --cflags libv4l2` -std=c++11  -MMD -MP -MF $@.d -o ${OBJECTDIR}/myconverters.o myconverters.cpp

${OBJECTDIR}/mycurl.o: mycurl.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -I/usr/include/libxml2 -I/usr/local/ffmpeg_build/include -I. `pkg-config --cflags --cflags libv4l2` -std=c++11  -MMD -MP -MF $@.d -o ${OBJECTDIR}/mycurl.o mycurl.cpp

${OBJECTDIR}/mypcm.o: mypcm.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -I/usr/include/libxml2 -I/usr/local/ffmpeg_build/include -I. `pkg-config --cflags --cflags libv4l2` -std=c++11  -MMD -MP -MF $@.d -o ${OBJECTDIR}/mypcm.o mypcm.cpp

${OBJECTDIR}/mystdlib.o: mystdlib.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -I/usr/include/libxml2 -I/usr/local/ffmpeg_build/include -I. `pkg-config --cflags --cflags libv4l2` -std=c++11  -MMD -MP -MF $@.d -o ${OBJECTDIR}/mystdlib.o mystdlib.cpp

${OBJECTDIR}/myxml.o: myxml.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -I/usr/include/libxml2 -I/usr/local/ffmpeg_build/include -I. `pkg-config --cflags --cflags libv4l2` -std=c++11  -MMD -MP -MF $@.d -o ${OBJECTDIR}/myxml.o myxml.cpp

${OBJECTDIR}/test-echo.o: test-echo.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -g -I/usr/local/ffmpeg_build/include -I. `pkg-config --cflags --cflags libv4l2`   -MMD -MP -MF $@.d -o ${OBJECTDIR}/test-echo.o test-echo.c

# Subprojects
.build-subprojects:

# Build Test Targets
.build-tests-conf: .build-conf ${TESTFILES}
${TESTDIR}/TestFiles/f2: ${TESTDIR}/tests/libav_encode_decode.o ${OBJECTFILES:%.o=%_nomain.o}
	${MKDIR} -p ${TESTDIR}/TestFiles
	${LINK.cc}   -o ${TESTDIR}/TestFiles/f2 $^ ${LDLIBSOPTIONS} 

${TESTDIR}/TestFiles/f1: ${TESTDIR}/ffmpeg_build/tests/libavcodec_example_test.o ${OBJECTFILES:%.o=%_nomain.o}
	${MKDIR} -p ${TESTDIR}/TestFiles
	${LINK.cc}   -o ${TESTDIR}/TestFiles/f1 $^ ${LDLIBSOPTIONS} 

${TESTDIR}/TestFiles/f3: ${TESTDIR}/tests/pcm_test.o ${OBJECTFILES:%.o=%_nomain.o}
	${MKDIR} -p ${TESTDIR}/TestFiles
	${LINK.cc}   -o ${TESTDIR}/TestFiles/f3 $^ ${LDLIBSOPTIONS} 


${TESTDIR}/tests/libav_encode_decode.o: tests/libav_encode_decode.cpp 
	${MKDIR} -p ${TESTDIR}/tests
	${RM} $@.d
	$(COMPILE.cc) -g -I/usr/include/libxml2 -I/usr/local/ffmpeg_build/include -I. -I. `pkg-config --cflags --cflags libv4l2` -std=c++11  -MMD -MP -MF $@.d -o ${TESTDIR}/tests/libav_encode_decode.o tests/libav_encode_decode.cpp


${TESTDIR}/ffmpeg_build/tests/libavcodec_example_test.o: ffmpeg_build/tests/libavcodec_example_test.cpp 
	${MKDIR} -p ${TESTDIR}/ffmpeg_build/tests
	${RM} $@.d
	$(COMPILE.cc) -g -I/usr/include/libxml2 -I/usr/local/ffmpeg_build/include -I. -I. `pkg-config --cflags --cflags libv4l2` -std=c++11  -MMD -MP -MF $@.d -o ${TESTDIR}/ffmpeg_build/tests/libavcodec_example_test.o ffmpeg_build/tests/libavcodec_example_test.cpp


${TESTDIR}/tests/pcm_test.o: tests/pcm_test.cpp 
	${MKDIR} -p ${TESTDIR}/tests
	${RM} $@.d
	$(COMPILE.cc) -g -I/usr/include/libxml2 -I/usr/local/ffmpeg_build/include -I. -I. `pkg-config --cflags --cflags libv4l2` -std=c++11  -MMD -MP -MF $@.d -o ${TESTDIR}/tests/pcm_test.o tests/pcm_test.cpp


${OBJECTDIR}/ClientSocket_nomain.o: ${OBJECTDIR}/ClientSocket.o ClientSocket.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/ClientSocket.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} $@.d;\
	    $(COMPILE.cc) -g -I/usr/include/libxml2 -I/usr/local/ffmpeg_build/include -I. `pkg-config --cflags --cflags libv4l2` -std=c++11  -Dmain=__nomain -MMD -MP -MF $@.d -o ${OBJECTDIR}/ClientSocket_nomain.o ClientSocket.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/ClientSocket.o ${OBJECTDIR}/ClientSocket_nomain.o;\
	fi

${OBJECTDIR}/MediaManager_nomain.o: ${OBJECTDIR}/MediaManager.o MediaManager.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/MediaManager.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} $@.d;\
	    $(COMPILE.cc) -g -I/usr/include/libxml2 -I/usr/local/ffmpeg_build/include -I. `pkg-config --cflags --cflags libv4l2` -std=c++11  -Dmain=__nomain -MMD -MP -MF $@.d -o ${OBJECTDIR}/MediaManager_nomain.o MediaManager.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/MediaManager.o ${OBJECTDIR}/MediaManager_nomain.o;\
	fi

${OBJECTDIR}/Multimedia_nomain.o: ${OBJECTDIR}/Multimedia.o Multimedia.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/Multimedia.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} $@.d;\
	    $(COMPILE.cc) -g -I/usr/include/libxml2 -I/usr/local/ffmpeg_build/include -I. `pkg-config --cflags --cflags libv4l2` -std=c++11  -Dmain=__nomain -MMD -MP -MF $@.d -o ${OBJECTDIR}/Multimedia_nomain.o Multimedia.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/Multimedia.o ${OBJECTDIR}/Multimedia_nomain.o;\
	fi

${OBJECTDIR}/ServerSocket_nomain.o: ${OBJECTDIR}/ServerSocket.o ServerSocket.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/ServerSocket.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} $@.d;\
	    $(COMPILE.cc) -g -I/usr/include/libxml2 -I/usr/local/ffmpeg_build/include -I. `pkg-config --cflags --cflags libv4l2` -std=c++11  -Dmain=__nomain -MMD -MP -MF $@.d -o ${OBJECTDIR}/ServerSocket_nomain.o ServerSocket.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/ServerSocket.o ${OBJECTDIR}/ServerSocket_nomain.o;\
	fi

${OBJECTDIR}/Socket_nomain.o: ${OBJECTDIR}/Socket.o Socket.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/Socket.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} $@.d;\
	    $(COMPILE.cc) -g -I/usr/include/libxml2 -I/usr/local/ffmpeg_build/include -I. `pkg-config --cflags --cflags libv4l2` -std=c++11  -Dmain=__nomain -MMD -MP -MF $@.d -o ${OBJECTDIR}/Socket_nomain.o Socket.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/Socket.o ${OBJECTDIR}/Socket_nomain.o;\
	fi

${OBJECTDIR}/capture_nomain.o: ${OBJECTDIR}/capture.o capture.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/capture.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} $@.d;\
	    $(COMPILE.cc) -g -I/usr/include/libxml2 -I/usr/local/ffmpeg_build/include -I. `pkg-config --cflags --cflags libv4l2` -std=c++11  -Dmain=__nomain -MMD -MP -MF $@.d -o ${OBJECTDIR}/capture_nomain.o capture.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/capture.o ${OBJECTDIR}/capture_nomain.o;\
	fi

${OBJECTDIR}/debug_nomain.o: ${OBJECTDIR}/debug.o debug.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/debug.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} $@.d;\
	    $(COMPILE.cc) -g -I/usr/include/libxml2 -I/usr/local/ffmpeg_build/include -I. `pkg-config --cflags --cflags libv4l2` -std=c++11  -Dmain=__nomain -MMD -MP -MF $@.d -o ${OBJECTDIR}/debug_nomain.o debug.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/debug.o ${OBJECTDIR}/debug_nomain.o;\
	fi

${OBJECTDIR}/libavcodec_util_nomain.o: ${OBJECTDIR}/libavcodec_util.o libavcodec_util.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/libavcodec_util.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} $@.d;\
	    $(COMPILE.cc) -g -I/usr/include/libxml2 -I/usr/local/ffmpeg_build/include -I. `pkg-config --cflags --cflags libv4l2` -std=c++11  -Dmain=__nomain -MMD -MP -MF $@.d -o ${OBJECTDIR}/libavcodec_util_nomain.o libavcodec_util.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/libavcodec_util.o ${OBJECTDIR}/libavcodec_util_nomain.o;\
	fi

${OBJECTDIR}/main_nomain.o: ${OBJECTDIR}/main.o main.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/main.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} $@.d;\
	    $(COMPILE.cc) -g -I/usr/include/libxml2 -I/usr/local/ffmpeg_build/include -I. `pkg-config --cflags --cflags libv4l2` -std=c++11  -Dmain=__nomain -MMD -MP -MF $@.d -o ${OBJECTDIR}/main_nomain.o main.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/main.o ${OBJECTDIR}/main_nomain.o;\
	fi

${OBJECTDIR}/myconverters_nomain.o: ${OBJECTDIR}/myconverters.o myconverters.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/myconverters.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} $@.d;\
	    $(COMPILE.cc) -g -I/usr/include/libxml2 -I/usr/local/ffmpeg_build/include -I. `pkg-config --cflags --cflags libv4l2` -std=c++11  -Dmain=__nomain -MMD -MP -MF $@.d -o ${OBJECTDIR}/myconverters_nomain.o myconverters.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/myconverters.o ${OBJECTDIR}/myconverters_nomain.o;\
	fi

${OBJECTDIR}/mycurl_nomain.o: ${OBJECTDIR}/mycurl.o mycurl.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/mycurl.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} $@.d;\
	    $(COMPILE.cc) -g -I/usr/include/libxml2 -I/usr/local/ffmpeg_build/include -I. `pkg-config --cflags --cflags libv4l2` -std=c++11  -Dmain=__nomain -MMD -MP -MF $@.d -o ${OBJECTDIR}/mycurl_nomain.o mycurl.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/mycurl.o ${OBJECTDIR}/mycurl_nomain.o;\
	fi

${OBJECTDIR}/mypcm_nomain.o: ${OBJECTDIR}/mypcm.o mypcm.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/mypcm.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} $@.d;\
	    $(COMPILE.cc) -g -I/usr/include/libxml2 -I/usr/local/ffmpeg_build/include -I. `pkg-config --cflags --cflags libv4l2` -std=c++11  -Dmain=__nomain -MMD -MP -MF $@.d -o ${OBJECTDIR}/mypcm_nomain.o mypcm.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/mypcm.o ${OBJECTDIR}/mypcm_nomain.o;\
	fi

${OBJECTDIR}/mystdlib_nomain.o: ${OBJECTDIR}/mystdlib.o mystdlib.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/mystdlib.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} $@.d;\
	    $(COMPILE.cc) -g -I/usr/include/libxml2 -I/usr/local/ffmpeg_build/include -I. `pkg-config --cflags --cflags libv4l2` -std=c++11  -Dmain=__nomain -MMD -MP -MF $@.d -o ${OBJECTDIR}/mystdlib_nomain.o mystdlib.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/mystdlib.o ${OBJECTDIR}/mystdlib_nomain.o;\
	fi

${OBJECTDIR}/myxml_nomain.o: ${OBJECTDIR}/myxml.o myxml.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/myxml.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} $@.d;\
	    $(COMPILE.cc) -g -I/usr/include/libxml2 -I/usr/local/ffmpeg_build/include -I. `pkg-config --cflags --cflags libv4l2` -std=c++11  -Dmain=__nomain -MMD -MP -MF $@.d -o ${OBJECTDIR}/myxml_nomain.o myxml.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/myxml.o ${OBJECTDIR}/myxml_nomain.o;\
	fi

${OBJECTDIR}/test-echo_nomain.o: ${OBJECTDIR}/test-echo.o test-echo.c 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/test-echo.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} $@.d;\
	    $(COMPILE.c) -g -I/usr/local/ffmpeg_build/include -I. `pkg-config --cflags --cflags libv4l2`   -Dmain=__nomain -MMD -MP -MF $@.d -o ${OBJECTDIR}/test-echo_nomain.o test-echo.c;\
	else  \
	    ${CP} ${OBJECTDIR}/test-echo.o ${OBJECTDIR}/test-echo_nomain.o;\
	fi

# Run Test Targets
.test-conf:
	@if [ "${TEST}" = "" ]; \
	then  \
	    ${TESTDIR}/TestFiles/f2 || true; \
	    ${TESTDIR}/TestFiles/f1 || true; \
	    ${TESTDIR}/TestFiles/f3 || true; \
	else  \
	    ./${TEST} || true; \
	fi

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/remotedevicecontroller

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
