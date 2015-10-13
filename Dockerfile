FROM ubuntu:trusty
MAINTAINER Jamie Alessio <jamie@stoic.net>

RUN apt-get update \
    && apt-get install -y \
    	autoconf \
    	build-essential \
    	libtool \
    && rm -rf /var/lib/apt/lists/*

COPY . /opt/epa-stormwater-management-model

WORKDIR /opt/epa-stormwater-management-model

# RUN autoreconf --install && ./configure && make && make install
