#!/bin/bash

#MIC_NAME="Sennheiser_Communications_Sennheiser_USB_headset-00.mono-fallback"
#VOLUME="52430" #set volume to 80% 

MIC_NAME="Focusrite_iTrack_Solo-00.analog-stereo"
VOLUME="78500" #set volume to 110% 

pacmd set-default-source alsa_input.usb-$MIC_NAME #set source to Sennheier
pacmd set-source-volume alsa_input.usb-$MIC_NAME $VOLUME

FILE_NAME="$1"
REC_TIME="$2" #second

python3 scripts/decode/recorder.py "$FILE_NAME" $REC_TIME 2>/dev/null &

I_VAL=0
REC_TIME_MS=$(($REC_TIME * 1000))

sleep 0.1

while [[ "$I_VAL" -lt "$REC_TIME_MS" ]]; do
	
	I_VAL=$(($I_VAL + 100))
	sleep 0.1
	T_VAL=$(($I_VAL / 1000))
	MS_VAL=$(($I_VAL % 1000))
	printf "\rTime = $T_VAL:$MS_VAL"
	
done

#echo #echo newline

wait #wait for record to finish

BUFF=$(sox "$FILE_NAME" -n stat 2>&1 | grep "Maximum amp")
AMP_MAX=$(echo "$BUFF" | awk '{print $3}')
#Convert to log
AMP_MAX=$(echo "20*l($AMP_MAX)/l(10)" | bc -l)

printf "\rMax Amp=%.2fdB\n" $AMP_MAX
