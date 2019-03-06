FROM ubuntu:18.04
LABEL maintainer="Artur Balanuta"
MAINTAINER "artur@cmu.edu"

RUN	apt-get update && \
	apt-get install -y --no-install-recommends \
		htop nano aria2 build-essential ca-certificates m4 less libjpeg62 libxmu-dev && \
	rm -rf /var/lib/apt/lists/*

ENV WORK_DIR /root
WORKDIR ${WORK_DIR}

ENV PIN_VER "pin-3.7-97619-g0d0c92f4f-gcc-linux"
ENV PIN_URL "https://software.intel.com/sites/landingpage/pintool/downloads/$PIN_VER.tar.gz"
ENV PIN_HOME "/root/$PIN_VER"

ENV PARSEC "parsec-2.1-core"
ENV PARSEC_SIM "parsec-2.1-sim"
ENV PARSEC_BIN "parsec-2.1-amd64-linux"
ENV PARSEC_URL "http://parsec.cs.princeton.edu/download/2.1/$PARSEC.tar.gz"
ENV PARSEC_SIM_URL "http://parsec.cs.princeton.edu/download/2.1/$PARSEC_SIM.tar.gz"
ENV PARSEC_BIN_URL "http://parsec.cs.princeton.edu/download/2.1/binaries/$PARSEC_BIN.tar.gz"

RUN aria2c -x 16 --summary-interval=1 $PIN_URL && tar -xvf $PIN_VER.tar.gz && rm $PIN_VER.tar.gz && \
	aria2c -x 16 --summary-interval=1 $PARSEC_URL && tar -xvf $PARSEC.tar.gz && rm $PARSEC.tar.gz && \
	aria2c -x 16 --summary-interval=1 $PARSEC_SIM_URL && tar -xvf $PARSEC_SIM.tar.gz && rm $PARSEC_SIM.tar.gz && \
	aria2c -x 16 --summary-interval=1 $PARSEC_BIN_URL && tar -xvf $PARSEC_BIN.tar.gz && rm $PARSEC_BIN.tar.gz

VOLUME /${WORK_DIR}/src