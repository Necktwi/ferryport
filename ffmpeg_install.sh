BDIR=$(pwd)
echo ${BDIR}

if [ ! -d ${BDIR}/ffmpeg_sources ]; then
mkdir ${BDIR}/ffmpeg_sources
fi

if [ ! -d ${BDIR}/ffmpeg_sources/x264 ]; then
cd ${BDIR}/ffmpeg_sources/
git clone --depth 1 git://git.videolan.org/x264.git
fi
cd ${BDIR}/ffmpeg_sources/x264
git stash
git pull
./configure --prefix="${BDIR}/ffmpeg_build" --bindir="${BDIR}/ffmpeg_build/bin" --enable-static
make
make install
make distclean

if [ ! -d ${BDIR}/ffmpeg_sources/fdk-aac ]; then
cd ${BDIR}/ffmpeg_sources/
git clone --depth 1 git://github.com/mstorsjo/fdk-aac.git
fi
cd ${BDIR}/ffmpeg_sources/fdk-aac
git stash
git pull
autoreconf -fiv
./configure --prefix="${BDIR}/ffmpeg_build" --disable-shared
make
make install
make distclean

if [ ! -d ${BDIR}/ffmpeg_sources/ffmpeg ]; then
cd ${BDIR}/ffmpeg_sources/
git clone --depth 1 git://source.ffmpeg.org/ffmpeg
fi
cd ${BDIR}/ffmpeg_sources/ffmpeg
git stash
git pull
PKG_CONFIG_PATH="${BDIR}/ffmpeg_build/lib/pkgconfig"
./configure --prefix="${BDIR}/ffmpeg_build"   --extra-cflags="-I${BDIR}/ffmpeg_build/include" --extra-ldflags="-L${BDIR}/ffmpeg_build/lib"   --bindir="${BDIR}/ffmpeg_build/bin" --extra-libs="-ldl" --enable-gpl --enable-libass --enable-libfdk-aac   --enable-libmp3lame --enable-libopus --enable-libtheora --enable-libvorbis --enable-libvpx --enable-libx264 --enable-nonfree 
make
make install
make distclean
hash -r
