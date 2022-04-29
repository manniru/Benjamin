#!/bin/bash
# helper functions for reading from input

function bt_fastRead()
{
	TIMEOUT="$1"
	LASTREP="$2"
	
	read -rs -N1 -t $TIMEOUT REP </dev/tty

	if [[ "$REP" ]]; then

		echo "$REP"

	else

		echo "$LASTREP"

	fi
	
}

function bt_getFileName()
{
	local FB=$(echo "$1" | awk -F '/' '{print $NF}')
	echo "$FB" | awk -F '.wav' '{print $1}'
}