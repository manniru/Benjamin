# written in python 3
# Create corpus.txt and text file

import random
import os
import sys
import subprocess # runing bash cmd
import math # floor float

w_filename = sys.argv[1] #1

WRD_FILE    = "word_list"

word_file = open(WRD_FILE)
lexicon = word_file.read().splitlines()

w_filename = w_filename[:-4] #remove .wav
utt_word   = w_filename.split('.')[0]
word_id    = utt_word.split('_')

TEXT_BUF   = ""

for i in range(0,len(word_id)):
	TEXT_BUF   += lexicon[int(word_id[i])]
	TEXT_BUF   += " "

TEXT_BUF   = TEXT_BUF[:-1] #remove last char
print(TEXT_BUF)
