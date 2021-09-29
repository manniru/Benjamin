# Setting local system jobs (local CPU - no external clusters)
fstdraw
fstcompile lat_vis.ark lat_fst.fst
fstdraw lat_fst.fst --portrait=true --osymbols=data/lang/words.txt | dot -Tpdf > tmp.pdf
