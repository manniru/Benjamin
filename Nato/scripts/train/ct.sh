#!/bin/bash
#Create corpus.txt and text file

AUDIO_DIR="$1"
DATA_DIR="$2"
SUB_DIR="$3"

TEXT_FILE="${DATA_DIR}/$SUB_DIR/text"
CORPUS_FILE="${DATA_DIR}/local/corpus.txt"

function convert2word()
{
	INDEX=$(($1+1))
	sed "${INDEX}q;d" word_list
}

find "$AUDIO_DIR" -type d > list_dir
sed -i '1d' list_dir #remove first line

while read p; do
	
	find "$p" -type f > list_file
	
	while read f; do
	
		FILENAME=$(echo "$f" | awk -F '/' '{print $NF}')
		FILEBASE=$(echo "$FILENAME" | awk -F '.' '{print $1}')
		SPEAK_ID=$(echo "$f" | awk -F '/' '{print $(NF-1)}')

		WAV_NAME="${SPEAK_ID}_${FILEBASE}"
		INDEX1=$(echo "$FILEBASE" | awk -F '_' '{print $1}' )
		INDEX2=$(echo "$FILEBASE" | awk -F '_' '{print $2}' )
		INDEX3=$(echo "$FILEBASE" | awk -F '_' '{print $3}' )

		WORD1=$(convert2word $INDEX1)
		WORD2=$(convert2word $INDEX2)
		WORD3=$(convert2word $INDEX3)
		
		echo "$WAV_NAME $WORD1 $WORD2 $WORD3" >> "$TEXT_FILE"
		echo "$WORD1 $WORD2 $WORD3" >> "$CORPUS_FILE"
		
	done <list_file
	
	rm list_file
		
done <list_dir

rm list_dir
