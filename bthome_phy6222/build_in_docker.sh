#!/bin/bash -e
build() {
  DEVICE=$1
  SWVER=$2
  make -s clean
  make -s -j PROJECT_NAME=$DEVICE$SWVER POJECT_DEF="-DDEVICE=DEVICE_$DEVICE"
  mv build/$DEVICE$SWVER.bin /bindir/
  make -s -j PROJECT_NAME=BOOT_$DEVICE$SWVER POJECT_DEF="-DDEVICE=DEVICE_$DEVICE" BOOT_OTA=1
  mv build/BOOT_$DEVICE$SWVER.bin /bindir/
  mv build/BOOT_$DEVICE$SWVER.hex /bootdir/

}

if [ "$1" != "innerbuild" ];then
    self_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd )"
    SWVER=_v18
    BINDIR=${BINDIR:-${self_dir}/bin}
    BOOTDIR=${BOOTDIR:-${self_dir}/boot}
    mkdir -p $BINDIR
    mkdir -p $BOOTDIR
    docker run --user $UID -t --rm -w /src -v ${self_dir}:/src -v $BINDIR:/bindir -v $BOOTDIR:/bootdir bthome-buildenv ./build_in_docker.sh innerbuild $SWVER $@
else
    SWVER=$2
    if [ $# -gt 2 ] && [ "$3" != "all" ];then
      build $3 $SWVER
    else
      for DEVICE in THB2 BTH01 TH05 TH05D TH05F THB1 THB3 TH05SC;do
        build $DEVICE $SWVER
      done
    fi
fi
