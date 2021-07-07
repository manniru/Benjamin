#!/bin/bash
# Generate wav.scp and utt2spk from audio directory
# Usage: wav_scp <audio_dir> <output_dir>

#LEXICON_DIR="$1"
LEXICON_DIR="data/local/dict"

source path.sh

# remove begin word in each line
cut -d' ' -f2- "$LEXICON_DIR/lexicon.txt" > "$LEXICON_DIR/nonsilence_phones.txt"

# replace space with new line
cat "$LEXICON_DIR/nonsilence_phones.txt" | tr " " "\n" > "$LEXICON_DIR/tmp.txt"

$SD/sort.sh "$LEXICON_DIR/tmp.txt"

rm "$LEXICON_DIR/nonsilence_phones.txt"
mv "$LEXICON_DIR/tmp.txt" "$LEXICON_DIR/nonsilence_phones.txt"

#remove sil and spn
sed -i '/sil/d' "$LEXICON_DIR/nonsilence_phones.txt"
sed -i '/spn/d' "$LEXICON_DIR/nonsilence_phones.txt"
