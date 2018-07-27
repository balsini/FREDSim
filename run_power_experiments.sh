#!/bin/bash

RUN_EXP=${RUN_EXP:-0}

BIN_DIR="/home/alessio/git/rtlib2.0/build/examples/energy/"

DUMP="dump"
RESULTS="results"

OPP_little_freq=(200 300 400 500 600 700 800 900 1000 1100 1200 1300 1400)
OPP_big_freq=(200 300 400 500 600 700 800 900 1000 1100 1200 1300 1400 1500 1600 1700 1800 1900 2000)

run_experiment ()
{
    local OPP_LITTLE=$1
    local OPP_BIG=$2
    local WL=$3

    echo "OPP_little: $OPP_LITTLE"
    echo "OPP_BIG $OPP_BIG"
    echo "WL $WL"
    
    /home/alessio/git/rtlib2.0/build/examples/energy/energy $OPP_LITTLE $OPP_BIG $WL

    for f in $(ls power_*.txt | sort); do
        filename=$(basename $f)
        echo "$filename"
        cat $f | cut -f 2 -d ':' > $f.done
    done
    
    paste $(ls power_*.done | sort) | awk '{print $5+$6+$7+$8, $1+$2+$3+$4}' >  $RESULTS/opp-${OPP_little_freq[$OPP_LITTLE]}-${OPP_big_freq[$OPP_BIG]}-$WL.txt
}

# Run experiment

if [ "$RUN_EXP" != "0" ]; then

    rm -rf $DUMP
    mkdir -p $DUMP/$RESULTS
    cd $DUMP
    
    for wl in idle bzip2 hash encrypt decrypt cachekiller; do
        for opp_l in $(seq 0 12); do
            run_experiment $opp_l 0 $wl
        done
        for opp_b in $(seq 0 18); do
            run_experiment 0 $opp_b $wl
        done
    done
    
    cd -
fi

rm -rf $DUMP/$RESULTS/complete
mkdir -p $DUMP/$RESULTS/complete
cd $DUMP/$RESULTS

for wl in idle bzip2 hash encrypt decrypt cachekiller; do
    filename=complete/"power_"$wl"_LITTLE.txt"
    filename=$(echo $filename | sed -e "s/encrypt/des3_encrypt/g" -e "s/decrypt/des3_decrypt/g")
    echo
    echo $filename
    echo -n -e "" > $filename
    for opp_l in $(seq 0 12); do
        for f in $(ls *opp-${OPP_little_freq[$opp_l]}-200-$wl.txt); do
            echo $f
            curr_freq=${OPP_little_freq[$opp_l]}
            echo $(( curr_freq * 1000 )) $(cat $f | awk '{print $1}' | sort -u -r | head -n 1) >> $filename
        done
    done
    
    filename=complete/"power_"$wl"_big.txt"
    filename=$(echo $filename | sed -e "s/encrypt/des3_encrypt/g" -e "s/decrypt/des3_decrypt/g")
    echo
    echo $filename
    echo -n -e "" > $filename
    for opp_b in $(seq 0 18); do
         for f in $(ls *opp-200-${OPP_big_freq[$opp_b]}-$wl.txt); do
            echo $f
            curr_freq=${OPP_big_freq[$opp_b]}
            echo $(( curr_freq * 1000 )) $(cat $f | awk '{print $2}' | sort -u -r | head -n 1) >> $filename
        done
    done
done
