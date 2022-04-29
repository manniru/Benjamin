#!/bin/bash

source scripts/functions/bt_mic.sh
source scripts/functions/bt_input.sh

STATUS=""

function bt_procInput()
{
	REP=$(bt_fastRead "0.1" "$REP")

	if [[ "$REP" ]]; then

		FILEBASE=$(bt_getFileName $FILE_NAME)
		printf "\r\e[K\r" # clear line
		printf "\e[K\r$FILEBASE, q: quit: "
		pkill -P $$ #Kill python recorder @ background
		read -rs -N1 REP </dev/tty

		printf "\r\e[K\r" # clear line
		printf "$FILEBASE removed\n"
		rm $FILE_NAME 2>/dev/null

		if [[ "$REP" == "q" ]]; then
			exit 2
		else
			exit 1
		fi

	fi
}

bt_InitMic # only for senheiser or focusrite

FILE_NAME="$1"
REC_TIME="$2" #second

bt_procInput

python3 scripts/decode/recorder.py "$FILE_NAME" $REC_TIME 2>/dev/null &

I_VAL=0
REC_TIME_MS=$(($REC_TIME * 1000))

bt_procInput

while [[ "$I_VAL" -lt "$REC_TIME_MS" ]]; do

	I_VAL=$(($I_VAL + 100))
	bt_procInput
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
echo "R=$REP"

exit 0