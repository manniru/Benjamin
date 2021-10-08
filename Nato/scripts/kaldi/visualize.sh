# Setting local system jobs (local CPU - no external clusters)
# cd ../..;source path.sh; cd scripts/kaldi/
# exp/$MODE/graph/words.txt
LAT_INPUT=$1
RESULT_PATH=$2

MODE="tri1"
#WORD_TABLE="exp/$MODE/graph/words.txt"
#WORD_TABLE="../../exp/$MODE/graph/words.txt"

VIS_ARK="$RESULT_PATH/vis.ark"
VIS_FST="$RESULT_PATH/vis.fst"
VIS_BIN="$RESULT_PATH/vis.bin"
VIS_PDF="$RESULT_PATH/vis.pdf"

lattice-to-fst --acoustic-scale=0.00005 ark:$LAT_INPUT ark,t:$VIS_ARK 2>/dev/null
tail -n +2 $VIS_ARK > $VIS_FST #remove first line

#round weight to 3 decimal
AWK_CMD='{printf("%s\t%s\t%s\t%s\t%.3f\n", $1, $2, $3, $4, $5)}'

rm "$VIS_ARK"
while read line; do
	
	LINE_CNT=$(echo "$line" | wc -w)
	if [[ "$LINE_CNT" == "5" ]]; then
	    echo "$line" | awk "$AWK_CMD" >> "$VIS_ARK"
	else
	    echo "$line" >> "$VIS_ARK"
	fi
	
done <$VIS_FST

fstcompile $VIS_ARK $VIS_BIN
fstdraw --portrait=true --isymbols=$WORD_TABLE --acceptor=true $VIS_BIN | dot -Tpdf > $VIS_PDF

