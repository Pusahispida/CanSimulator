#!/bin/bash
set -eu

USAGE="Usage: $0 <can interface> <bitrate>
  Example: $0 can0 500000"

if [ "$#" -ne 2 ]; then
  echo "$USAGE"
  exit 1
fi

case "$1" in
  can*) TYPE=can;;
  vcan*) TYPE=vcan; sudo modprobe vcan ;;
*) echo "Unknown interface type"; exit 1;;
esac

case "$2" in
  ''|*[!0-9]*) echo "$USAGE"; exit 1;;
esac

sudo ip link set $1 down
sudo ip link set $1 type $TYPE bitrate $2
sudo ip link set $1 up
