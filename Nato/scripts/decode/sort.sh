awk '!a[$0]++' "$1" > input_file
sort input_file > output_file
rm "$1"
mv output_file "$1"
rm input_file

