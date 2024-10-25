#target: bthome-buildenv
FROM debian:bookworm
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive TZ=Etc/UTC \
    apt-get install -y --no-install-recommends \
    build-essential curl gcc-arm-none-eabi python3 libnewlib-arm-none-eabi
