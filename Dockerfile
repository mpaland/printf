# Build Stage
FROM --platform=linux/amd64 ubuntu:20.04 as builder

## Install build dependencies.
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y cmake clang

## Add source code to the build stage.
ADD . /printf
WORKDIR /printf

## TODO: ADD YOUR BUILD INSTRUCTIONS HERE.
WORKDIR /printf/build 
RUN CC=clang CXX=clang++ cmake ..
RUN make

#Package Stage
FROM --platform=linux/amd64 ubuntu:20.04

## TODO: Change <Path in Builder Stage>
COPY --from=builder /printf/build/printf /

