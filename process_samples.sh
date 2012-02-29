#!/bin/bash

images=(  \ 
    16-186 \
    32-080.1 \
    8-007.1 \
    E14-294 \
    16-188.1 \
    32-080.2 \
    8-007.2 \
    E14-295 \
    16-188.2 \
    46-1303 \
    8-111 \
    E25-125_Women \
    26-100.1 \
    56-191 \
    8-113 \
    E25-143 \
    26-100.2 \
    66-160.1 \
    8-140 \
    E25_FLOOR_1.1 \
    3-101 \
    66-160.2 \
    Building_4_FLOOR_1 \
    E25_FLOOR_1.2 \
    32-069T \
    76-100SC \
    E14-102.1 \
    Sala_De_Puerto_Rico \
    32-076 \
    76-155.1 \
    E14-102.1 \
    W20-205 \
    32-079 \
    76-155.2 \
    E14-102.2 \
)

for image in ${images[@]}
do
   echo processing $image
   ./find_text.py sampleImages/$image.jpg sampleImages/$image   
done