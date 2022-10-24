FROM node:17.3.1-bullseye-slim

ARG DEBIAN_FRONTEND=noninteractive
ENV TZ=Europe/Prague

RUN apt-get update -q -y && \
    apt-get install -q -y tzdata libssl-dev libusb-1.0-0-dev cmake make build-essential curl libsystemd-dev clang-format

WORKDIR /app
VOLUME /app

ADD ./build-in-docker.sh /root/build-in-docker.sh
RUN chmod +x /root/build-in-docker.sh

ENTRYPOINT /root/build-in-docker.sh
