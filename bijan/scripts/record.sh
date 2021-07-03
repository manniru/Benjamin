#!/bin/bash

pacmd set-default-source alsa_input.usb-Sennheiser_Communications_Sennheiser_USB_headset-00.mono-fallback #set source to Sennheier
pacmd set-source-volume alsa_input.usb-Sennheiser_Communications_Sennheiser_USB_headset-00.mono-fallback 52430 #set volume to 80% 

FILE_NAME="$1"
REC_TIME="$2" #second
CHANNELS=2
BUFFER=4000
SAMPLE_RATE=16000
SAMPLE_SIZE=16

rec -r $SAMPLE_RATE -b $SAMPLE_SIZE -c $CHANNELS --buffer $BUFFER "$FILE_NAME" trim 0 $REC_TIME & > /dev/null

I_VAL=0
REC_TIME_MS=$(($REC_TIME * 1000))

sleep 0.2

while [[ "$I_VAL" -lt "$REC_TIME_MS" ]]; do
	
	I_VAL=$(($I_VAL + 100))
	sleep 0.1
	T_VAL=$(($I_VAL / 1000))
	MS_VAL=$(($I_VAL % 1000))
	printf "\rTime = $T_VAL:$MS_VAL"
	
done

echo #echo newline

sleep 0.2

echo "FUUUUUUUUUUUUUUU"#echo newline
