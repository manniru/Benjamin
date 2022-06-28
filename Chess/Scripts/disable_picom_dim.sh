#!/bin/bash
# get all window in the current desktop and disable picom
# dim on them by using picom-trans

DESKTOP_ID=$(xdotool get_desktop)

while read -r line; do
	# get window desktop ID
    WD_ID=$(echo "$line" | awk '{print $2}')
	if [[ "$WD_ID" == "$DESKTOP_ID" ]]; then
		WIN_ID=$(echo "$line" | awk '{print $1}')
		echo $WIN_ID
		picom-trans -w $WIN_ID --toggle 100
	fi
done < <(wmctrl -l)