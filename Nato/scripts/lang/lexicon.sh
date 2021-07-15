#!/bin/bash
# Generate wav.scp and utt2spk from audio directory
# Usage: wav_scp <audio_dir> <output_dir>

#LEXICON_DIR="$1"
LEXICON_DIR="$1"
WORD_LIST="word_list"
DICT="$2"

source path.sh

echo "!SIL sil" > "$LEXICON_DIR/lexicon.txt"
echo "<UNK> spn" >> "$LEXICON_DIR/lexicon.txt"

while read WORD; do
	
	PRN_LINE=$(cat $DICT | grep "^$WORD ")
	
	if [[ -z "$PRN_LINE" ]]; then
		
		echo "Error: $WORD"
		
	fi
	# to lower case
	PRN_LINE=$(echo "$PRN_LINE" | tr '[:upper:]' '[:lower:]')
	echo "$PRN_LINE" >> "$LEXICON_DIR/lexicon.txt"
		
done <$WORD_LIST

$SD/sort.sh "$LEXICON_DIR/lexicon.txt"
