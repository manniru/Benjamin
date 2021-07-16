#!/bin/bash
#Print utterance

INDEX1=$(echo $1 | awk -F_ '{print $1}')
INDEX2=$(echo $1 | awk -F_ '{print $2}')
INDEX3=$(echo $1 | awk -F_ '{print $3}' | awk -F. '{print $1}')

NUMBER="$2"
COUNT="$3"

function convert2word()
{
	INDEX=$(($1+1))
	sed "${INDEX}q;d" word_list
}

WORD1=$(convert2word $INDEX1)
WORD2=$(convert2word $INDEX2)
WORD3=$(convert2word $INDEX3)

printf "%4s/$COUNT %15s %10s %10s %10s\n" $NUMBER $1 $WORD1 $WORD2 $WORD3

