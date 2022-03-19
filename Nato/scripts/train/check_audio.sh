#!/bin/bash

AUD_PATH="audio/train"

find "$AUD_PATH" -type d > list_dir
sed -i '1d' list_dir #remove first line

while read DIR_NAME; do
	
	COUNT=$(find "$DIR_NAME" -type f | wc -l)
	printf "%s: %4s\n" $DIR_NAME $COUNT
		
done <list_dir

rm list_dir
