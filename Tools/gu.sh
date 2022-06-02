#! /bin/bash
inotifywait -q -m -e close_write graph | 
while read -r filename event; do 
	dot -Tpng graph  > out.png
done
