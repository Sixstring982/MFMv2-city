#!/bin/bash

declare -a flags=("NOT_USED" "CANAL_ROUTING" "RANDOM_ROUTING")

for i in ${flags[@]}
do
    make -C ~/Documents/Git/MFMv2-city/ realclean
    echo make $i=1 -C ~/Documents/Git/MFMv2-city/ realclean
    make $i=1 -C ~/Documents/Git/MFMv2-city/
    for j in `seq 1 10`
    do
        echo ~/Documents/Git/MFMv2-city/bin/mfms --ignorethreadbugs --run --haltafteraeps 30000 --startminimal -cp /nfs/student/t/tsmall1/Desktop/city.mfs > ~/Desktop/mfm-city-$i-$j.txt
        ~/Documents/Git/MFMv2-city/bin/mfms --ignorethreadbugs --run --haltafteraeps 30000 --startminimal -cp /nfs/student/t/tsmall1/Desktop/city.mfs > ~/Desktop/mfm-city-$i-$j.txt
    done
done
