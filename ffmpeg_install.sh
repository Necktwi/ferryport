            BDIR=$(pwd)
            echo $BDIR
            rm -rf ~/ffmpeg_build ~/bin/{ffmpeg,ffprobe,ffserver,vsyasm,x264,yasm,ytasm}
            sudo apt-get update
            sudo apt-get -y install autoconf automake build-essential git libass-dev libgpac-dev libtheora-dev libtool libvorbis-dev pkg-config texi2html zlib1g-dev yasm libmp3lame-dev libopus-dev libvpx-dev
            mkdir $BDIR/ffmpeg_sources

            cd $BDIR/ffmpeg_sources/x264
            git stash
            git pull
            ./configure --prefix="$BDIR/ffmpeg_build" --bindir="$BDIR/ffmpeg_build/bin" --enable-static
            make
            make install
            make distclean
            
            cd $BDIR/ffmpeg_sources/fdk-aac
            git stash
            git pull
            autoreconf -fiv
            ./configure --prefix="$BDIR/ffmpeg_build" --disable-shared
            make
            make install
            make distclean
            
            cd $BDIR/ffmpeg_sources/ffmpeg
            git stash
            git pull
            PKG_CONFIG_PATH="$BDIR/ffmpeg_build/lib/pkgconfig"
            ./configure --prefix="$BDIR/ffmpeg_build"   --extra-cflags="-I$BDIR/ffmpeg_build/include" --extra-ldflags="-L$BDIR/ffmpeg_build/lib"   --bindir="$BDIR/ffmpeg_build/bin" --extra-libs="-ldl" --enable-gpl --enable-libass --enable-libfdk-aac   --enable-libmp3lame --enable-libopus --enable-libtheora --enable-libvorbis --enable-libvpx --enable-libx264 --enable-nonfree 
            make
            make install
            make distclean
            hash -r
