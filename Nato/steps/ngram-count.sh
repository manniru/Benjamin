#! /bin/sh

L_COUNT=$(wc -l word_list | awk '{ print $1 }')
L_COUNT=$(($L_COUNT+1))

echo ""
echo "\\data\\"
echo "ngram 1=$L_COUNT"
echo ""
echo "\\1-grams:"

echo "-0.4771212	</s>"
echo "-99	<s>"

PROB=$(echo "l((1-1/3)/$L_COUNT)/l(10)" | bc -l)

printf "%.3f\n" "$PROB"