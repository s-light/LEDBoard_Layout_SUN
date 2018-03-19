#!/bin/bash

rootfs=~/mydata/acme/arietta/debian_8.10_jessie/target-rootfs
target_config="$rootfs/home/light/.ola/*"

# update local image
cp "./target" $target_config
# copy to remote device
scp  $target_config "light@arietta.local:/home/light/.ola/"
