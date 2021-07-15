#!/bin/bash
#Print utterance

INDEX1="$1"
INDEX2="$2"
INDEX3="$3"

NUMBER="$4"
COUNT="$5"

function convert2word()
{
	INDEX=$(($1+1))
	sed "${INDEX}q;d" word_list
}

WORD1=$(convert2word $INDEX1)
WORD2=$(convert2word $INDEX2)
WORD3=$(convert2word $INDEX3)

printf "UTTERANCE %4s/$COUNT: %4s %4s %4s %10s %10s %10s\n" $NUMBER $INDEX1 $INDEX2 $INDEX3 $WORD1 $WORD2 $WORD3

