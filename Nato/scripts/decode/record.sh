#!/bin/bash

ALIN="alsa_input.usb-"
S_NAME="Sennheiser_Communications_Sennheiser"
F_NAME="Focusrite_iTrack_Solo"
SENNHEISER=$(pactl list short sources | grep "${ALIN}${S_NAME}")
FOCUSRITE=$(pactl list short sources | grep "${ALIN}${F_NAME}")

if [[ "$SENNHEISER" ]]; then

	MIC_NAME="${ALIN}${S_NAME}_USB_headset-00.mono-fallback"
	VOLUME="52430" #set volume to 80% 

elif [[ "$FOCUSRITE" ]]; then

	MIC_NAME=$(echo "$FOCUSRITE" | cut -f 2)
	VOLUME="100500" #set volume to 130% 

fi

if [[ "$MIC_NAME" ]]; then
	pacmd set-default-source $MIC_NAME
	pacmd set-source-volume $MIC_NAME $VOLUME
fi

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
	printf "\r                            Time = $T_VAL:$MS_VAL"

done

wait #wait for record to finish

BUFF=$(sox "$FILE_NAME" -n stat 2>&1 | grep "Maximum amp")
AMP_MAX=$(echo "$BUFF" | awk '{print $3}')
#Convert to log
AMP_MAX=$(echo "20*l($AMP_MAX)/l(10)" | bc -l)

printf "\rMax Amp=%.2fdB\n" $AMP_MAX
