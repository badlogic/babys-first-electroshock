#!/bin/bash

if [ "$#" -ne 2 ]; then
  echo "Usage: $0 <ip_address> <file_name>"
  exit 1
fi

IP_ADDRESS=$1
FILE_NAME=$2

curl -X POST --data-binary @"$FILE_NAME" -H "Content-Type: application/octet-stream" "http://$IP_ADDRESS/audio"
