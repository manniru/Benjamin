#!/usr/bin/env bash
# Copyright 2012  Johns Hopkins University (Author: Daniel Povey)
# Apache 2.0

[ -f ./path.sh ] && . ./path.sh

# begin configuration section.
cmd=run.pl
min_lmwt=7
max_lmwt=17
#end configuration section.

[ -f ./path.sh ] && . ./path.sh
. parse_options.sh || exit 1;

if [ $# -ne 3 ]; then
  echo "Usage: local/score.sh [--cmd (run.pl|queue.pl...)] <data-dir> <lang-dir|graph-dir> <decode-dir>"
  echo " Options:"
  echo "    --cmd (run.pl|queue.pl...)      # specify how to run the sub-processes."
  echo "    --min_lmwt <int>                # minumum LM-weight for lattice rescoring "
  echo "    --max_lmwt <int>                # maximum LM-weight for lattice rescoring "
  exit 1;
fi

data=$1
lang_or_graph=$2
dir=$3

symtab=$lang_or_graph/words.txt

for f in $symtab $dir/lat.1.gz $data/text; do
  [ ! -f $f ] && echo "score.sh: no such file $f" && exit 1;
done

mkdir -p $dir/scoring/log

cat $data/text | sed 's:<NOISE>::g' | sed 's:<SPOKEN_NOISE>::g' > $dir/scoring/test_filt.txt

$cmd LMWT=$min_lmwt:$max_lmwt $dir/scoring/log/best_path.LMWT.log \
  lattice-best-path --lm-scale=LMWT --word-symbol-table=$symtab \
    "ark:gunzip -c $dir/lat.*.gz|" ark,t:$dir/scoring/LMWT.tra || exit 1;

# Note: the double level of quoting for the sed command
$cmd LMWT=$min_lmwt:$max_lmwt $dir/scoring/log/score.LMWT.log \
   cat $dir/scoring/LMWT.tra \| \
    utils/int2sym.pl -f 2- $symtab \| sed 's:\<UNK\>::g' \| \
    compute-wer --text --mode=present \
     ark:$dir/scoring/test_filt.txt  ark,p:- ">&" $dir/wer_LMWT || exit 1;

# Show results
for i in $(seq $min_lmwt $max_lmwt); do

    echo "####### lmwt=$i #######";
    
    egrep '(WER)|(SER)' < $dir/wer_$i
	
	while read f; do
	
		WORD_COUNT=$(echo "$f" | wc -w)
        FILENAME=$(echo "$f" | cut -d " " -f 1 | cut -d "_" -f 2-)
        
        SYM1=$(echo "$f" | cut -d " " -f 2)
        SYM2=$(echo "$f" | cut -d " " -f 3)
        SYM3=$(echo "$f" | cut -d " " -f 4)
        
        WORD1=$(cat $symtab | head -$((SYM1+1)) | tail -1 | cut -d " " -f 1)
        WORD2=$(cat $symtab | head -$((SYM2+1)) | tail -1 | cut -d " " -f 1)
        WORD3=$(cat $symtab | head -$((SYM3+1)) | tail -1 | cut -d " " -f 1)
        
        ID1=$(cat word_list | grep -n -w "$WORD1" | cut -d ":" -f 1)
        ID2=$(cat word_list | grep -n -w "$WORD2" | cut -d ":" -f 1)
        ID3=$(cat word_list | grep -n -w "$WORD3" | cut -d ":" -f 1)
        
        ID1=$((ID1-1))
        ID2=$((ID2-1))
        ID3=$((ID3-1))

        if [[ "$WORD_COUNT" != "4" ]]; then
            
            echo "WORD_SIZE_WRONG $FILENAME ${WORD1} ${WORD2} ${WORD3}"
            
        elif [[ "$FILENAME" != "${ID1}_${ID2}_${ID3}" ]]; then
        
	        echo "$FILENAME ${WORD1} ${WORD2} ${WORD3}"
	        
        fi
		
	done <$dir/scoring/$i.tra

done

exit 0;
