#!/bin/bash

FILE="$1"
# remove empty lineu
sed '/^[[:space:]]*$/d' $FILE > tmp

# fix if
sed -i 's/\[\[ //g' tmp
sed -i 's/\]\]; //g' tmp

# fix elif
sed -i 's/elif/elseif/g' tmp

# fix fi
sed -i 's/fi/end/g' tmp

## add return
sed -i 's/OUTPUT=/return /g' tmp

## replace $WORD and send to cliboard
sed 's/"$WORD"/word/g' tmp | xclip -selection c
rm tmp