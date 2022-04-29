#!/bin/bash

# Set Default Microphone And Volume
function bt_InitMic()
{
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
}