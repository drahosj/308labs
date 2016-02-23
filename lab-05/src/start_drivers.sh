#!/bin/bash

cd printer
rm -f drivers/*
killall virt-printer

./virt-printer -n printer0
./virt-printer -n printer1
./virt-printer -n printer2
./virt-printer -n printer3
./virt-printer -n printer4
./virt-printer -n printer5
