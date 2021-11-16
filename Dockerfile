FROM alpine:latest
WORKDIR /usr/src
RUN apk add git gcc g++ make cmake python3 bash linux-headers openssl-dev

# Currently cryptoTools requires Boost >= 1.77 and the Alpine repos have 1.76.
# If this changes, add boost to the above list, and change the first build.py:
#	python3 build.py --setup --relic --noauto

# Install cryptoTools
RUN	git clone https://github.com/ladnir/cryptoTools \
 && cd cryptoTools \
 && git checkout 08deb9b06ad1ab76337223c9e70c2934ef79bd2b \
 && python3 build.py --setup --relic --boost \
 && python3 build.py -- -D ENABLE_RELIC=ON \
 && python3 build.py --install \
 && cd .. \
 && rm -rf cryptoTools

# Install HTDiSE
COPY . ./dise
RUN	cd dise \
 && cmake . \
 && make -j `nproc` \
 && cp -r bin /usr/local \
 && cd .. \
 && rm -rf dise

# Minimize packages for runtime
# Should leave: libstdc++, libgcc, musl, boost, relic, cryptotools
# Also leaving busybox for now for convenience
RUN	apk add libstdc++ \
 && apk del git gcc g++ make cmake python3 bash linux-headers openssl-dev \
 && apk del libc-utils apk-tools alpine-baselayout alpine-keys \
 && rm -rf /usr/local/include /usr/local/cmake /usr/local/lib/cmake /var/cache \
 && rm -rf /lib/apk /etc/apk

# Test
CMD ["/usr/local/bin/dEncFrontend", "-u"]
