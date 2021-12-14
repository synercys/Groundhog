FROM alpine:3.15
WORKDIR /usr/local/src

# Install cryptoTools
RUN apk add git gcc g++ make cmake bash openssl-dev boost1.77-static boost1.77-dev \
 && git clone https://github.com/ladnir/cryptoTools \
 && cd cryptoTools \
 && git checkout 1f9d4708f96bdda5ecb1ac92b2005ffb567017b0 \
 && python3 build.py --setup --relic -DFETCH_BOOST=OFF \
 && python3 build.py -DENABLE_RELIC=ON \
 && python3 build.py --install \
 && cd .. \
 && rm -rf cryptoTools \
 && apk del git gcc g++ make cmake bash openssl-dev boost1.77-static boost1.77-dev

# Install HTDiSE
COPY . ./dise
RUN apk add gcc g++ make cmake libstdc++ openssl-dev boost1.77-static boost1.77-dev \
 && cd dise \
 && cmake . \
 && make -j `nproc` \
 && cp -r bin /usr/local \
 && cd .. \
 && rm -rf dise \
 && apk del gcc g++ make cmake openssl-dev boost1.77-static boost1.77-dev libc-utils

#boost1.77-thread boost1.77-system
# Minimize packages for runtime
# Should leave: libstdc++, libgcc, musl, boost, relic, cryptotools
# Also leaving busybox for now for convenience.
#RUN apk add libstdc++ \
# && apk del libc-utils apk-tools alpine-baselayout alpine-keys \
# && rm -rf /usr/local/include /usr/local/cmake /usr/local/lib/cmake /var/cache \
# && rm -rf /lib/apk /etc/apk

# Test
CMD ["/usr/local/bin/dEncFrontend", "-u"]
