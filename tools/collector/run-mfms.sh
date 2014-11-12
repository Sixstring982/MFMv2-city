#!/bin/bash

declare -a flags=("NOT_USED" "CANAL_ROUTING" "RANDOM_ROUTING")

for i in ${flags[@]}
do
    make -C ~/Documents/Git/MFMv2-city/ realclean
    make $i=1 -C ~/Documents/Git/MFMv2-city/
    echo make $i=1 -C ~/Documents/Git/MFMv2-city/ realclean
    for j in `seq 1 10`
    do
        ~/Documents/Git/MFMv2-city/bin/mfms --run -cp /home/sixstring982/Desktop/city.mfs > ~/Desktop/mfm-city-$i-$j.txt
        echo ~/Documents/Git/MFMv2-city/bin/mfms --run -cp /home/sixstring982/Desktop/city.mfs > ~/Desktop/mfm-city-$i-$j.txt
    done
done
