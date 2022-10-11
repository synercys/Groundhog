FROM alpine:3.15
WORKDIR /usr/local/src

# Install cryptoTools
COPY ./cryptoTools ./cryptoTools
RUN apk add git gcc g++ make cmake bash openssl-dev boost1.77-static boost1.77-dev \
 && cd cryptoTools \
 && rm -f cryptoTools/Common/config.h \
 && python3 build.py --setup --relic -DFETCH_BOOST=OFF \
 && python3 build.py -DENABLE_RELIC=ON \
 && python3 build.py --install \
 && cd .. \
 && rm -rf cryptoTools \
 && apk del git gcc g++ make cmake bash openssl-dev boost1.77-static boost1.77-dev



# Install HTDiSE, development mode (faster iteration)
RUN apk add git gcc g++ make cmake libstdc++ openssl-dev boost1.77-static boost1.77-dev python3
# compile from GitHub to get some baseline .o files
RUN echo initial git build \
 && git clone "https://github.com/disha-agarwal/dise" repo \
 && mv repo/DiSE . \
 && cd DiSE \
 && cmake . -Wno-dev \
 && make -j `nproc` || echo "GitHub build failed! Continuing..."
# overwrite git files with any local development changes
COPY ./DiSE ./DiSE
# compile again, taking advantage of unchanged .o files to be faster
RUN echo incremental build \
 && cd DiSE \
 && cmake . -Wno-dev \
 && make -j `nproc` \
 && cp -r bin /usr/local



# Install HTDiSE, release mode (smaller image)
#COPY ./DiSE ./DiSE
#RUN apk add gcc g++ make cmake libstdc++ openssl-dev boost1.77-static boost1.77-dev python3
# && cd DiSE \
# && cmake . -Wno-dev \
# && make -j `nproc` \
# && cp -r bin /usr/local \
# && cd .. \
# && rm -rf DiSE \
# && apk del gcc g++ make cmake openssl-dev boost1.77-static boost1.77-dev libc-utils



#boost1.77-thread boost1.77-system
# Minimize packages for runtime
# Should leave: libstdc++, libgcc, musl, boost, relic, cryptotools
# Also leaving busybox for now for convenience.
#RUN apk add libstdc++ \
# && apk del libc-utils apk-tools alpine-baselayout alpine-keys \
# && rm -rf /usr/local/include /usr/local/cmake /usr/local/lib/cmake /var/cache \
# && rm -rf /lib/apk /etc/apk

# Test
WORKDIR /usr/local
COPY restart.py uptime_server.py /usr/local/bin/
COPY state.txt /usr/local
CMD ["/usr/local/bin/dEncFrontend", "-u"]