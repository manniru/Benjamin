find "$1" -type f > list_file
DIR="$1"

while read f; do

    #cut can't get the last field
    FILENAME=$(echo "$f" | awk -F/ '{print $NF}')
    WORD_ID=$(echo "$FILENAME" | cut -d "." -f 1)
    
    ID1=$(echo "$WORD_ID" | cut -d "_" -f 1)
    ID2=$(echo "$WORD_ID" | cut -d "_" -f 2)
    ID3=$(echo "$WORD_ID" | cut -d "_" -f 3)

    WORD1=$(cat word_list | head -$((ID1+1)) | tail -1)
    WORD2=$(cat word_list | head -$((ID2+1)) | tail -1)
    WORD3=$(cat word_list | head -$((ID3+1)) | tail -1)
    
    echo "$ID1 $ID2 $ID3 $WORD1 $WORD2 $WORD3"
#
    mv $f $DIR/${WORD1}_${WORD2}_${WORD3}.wav


done <list_file

rm list_file
