BDIR=$(pwd)
echo ${BDIR}

if [ ! -d ${BDIR}/ffmpeg_sources ]; then
mkdir ${BDIR}/ffmpeg_sources
fi

cd ${BDIR}/ffmpeg_sources/x264
./configure --prefix="${BDIR}/ffmpeg_build" --bindir="${BDIR}/ffmpeg_build/bin" --enable-static
make
make install
make distclean

cd ${BDIR}/ffmpeg_sources/fdk-aac
autoreconf -fiv
./configure --prefix="${BDIR}/ffmpeg_build" --disable-shared
make
make install
make distclean

cd ${BDIR}/ffmpeg_sources/ffmpeg
PKG_CONFIG_PATH="${BDIR}/ffmpeg_build/lib/pkgconfig"
./configure --prefix="${BDIR}/ffmpeg_build"   --extra-cflags="-I${BDIR}/ffmpeg_build/include" --extra-ldflags="-L${BDIR}/ffmpeg_build/lib"   --bindir="${BDIR}/ffmpeg_build/bin" --extra-libs="-ldl" --enable-gpl --enable-libass --enable-libfdk-aac   --enable-libmp3lame --enable-libopus --enable-libtheora --enable-libvorbis --enable-libvpx --enable-libx264 --enable-nonfree 
make
make install
make distclean
hash -r
