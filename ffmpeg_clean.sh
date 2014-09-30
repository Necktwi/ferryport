BDIR=$(pwd)
echo $BDIR
rm -rf ${BDIR}/ffmpeg_build

if [ -d ${BDIR}/ffmpeg_sources/x264 ]; then
cd ${BDIR}/ffmpeg_sources/x264
make distclean
fi

if [ -d ${BDIR}/ffmpeg_sources/fdk-aac ]; then
cd ${BDIR}/ffmpeg_sources/fdk-aac
make distclean
fi

if [ -d ${BDIR}/ffmpeg_sources/ffmpeg ]; then
cd ${BDIR}/ffmpeg_sources/ffmpeg
make distclean
fi
