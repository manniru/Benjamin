#!/bin/bash

I_VAL="0"
M_VAL="0"
MAX_VAL="92"
MEAN_VAL="0"
VAR_VAL="0"
AUD_PATH="audio"

while [[ "$I_VAL" -lt "$MAX_VAL" ]]; do
	
	I_VAL=$(($I_VAL + 1))
	T_VAL=$(find $AUD_PATH | grep -e [^0-9]$I_VAL[^0-9] | wc -l)
	MEAN_VAL=$(($MEAN_VAL + $T_VAL))
	
done

MEAN_VAL=$(($MEAN_VAL / $MAX_VAL))
I_VAL="0"

while [[ "$I_VAL" -lt "$MAX_VAL" ]]; do
	
	I_VAL=$(($I_VAL + 1))
	T_VAL=$(find $AUD_PATH | grep -e [^0-9]$I_VAL[^0-9] | wc -l)
	VAR_VAL=$(($VAR_VAL + ($T_VAL - $MEAN_VAL)**2))
	
done

function printVals
{
	DIFF=$(( $T_VAL - $MEAN_VAL ))
	DIFF=${DIFF#-}
	#echo $DIFF $T_VAL - $MEAN_VAL
	if [[ $T_VAL -lt $MEAN_VAL ]]; then
		if [[ $DIFF -lt $VAR_VAL ]]; then
			printf "%2s: %4s         " $I_VAL $T_VAL
		else
			tput setaf 1 #red
			printf "%2s: %4s         " $I_VAL $T_VAL
		fi
	else
		if [[ $DIFF -lt $VAR_VAL ]]; then
			printf "%2s: %4s         " $I_VAL $T_VAL
		else
			tput setaf 2 #green
			printf "%2s: %4s         " $I_VAL $T_VAL
		fi
	fi
			tput sgr0 #normal
}

I_VAL="0"
VAR_VAL=$(($VAR_VAL / $MAX_VAL / 2 ))
VAR_VAL=$(echo "scale=0; sqrt($VAR_VAL)" | bc)

while [[ "$I_VAL" -lt "$MAX_VAL" ]]; do
	
	I_VAL=$(($I_VAL + 1))
	M_VAL=$(($M_VAL + 1))
	T_VAL=$(find $AUD_PATH | grep -e [^0-9]$I_VAL[^0-9] | wc -l)
	
	printVals
	
	if [[ "$M_VAL" -gt "3" ]]; then
		printf "\n"
		M_VAL="0"
	fi
done

echo Mean = $MEAN_VAL VAR_VAL = $VAR_VAL
