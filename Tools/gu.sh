#! /bin/bash
inotifywait -q -m -e close_write graph_orig | 
while read -r filename event; do 
	dot -Tpng graph_orig  > out.png
done
