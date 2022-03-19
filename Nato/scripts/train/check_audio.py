# written in python 3
# Author: MP Zanoosi Aug-2020

import random
import os
import sys
import subprocess # runing bash cmd
import math # floor float

AUD_PATH = "audio/train"
WRD_FILE = "word_list"

def printWord(id, word, count, state):
	if( state==1 ):
		os.system("tput setaf 2") #green
	elif( state==-1 ):
		os.system("tput setaf 1") #red

	cmd = f'printf "{id:<3}{word[0:5]:<6}{count:<6}"'
	os.system(cmd) #python print not work with color
	os.system("tput sgr0") #normal

base_cmd = "find "
base_cmd += AUD_PATH
base_cmd += " | grep -e [^0-9]"

word_file = open(WRD_FILE)
lexicon = word_file.read().splitlines()
word_count = len(lexicon)

# calc mean
sum = 0
samp_count = [] #count each word samples
for i in range(word_count):
	cmd  = base_cmd
	cmd += str(i)
	cmd += "[^0-9] | wc -l"
	cmd_out = subprocess.getoutput(cmd)
	samp_count.append(cmd_out)
	sum += int(samp_count[i])
mean = round(sum / word_count)

# calc variance
var = 0
for i in range(word_count):
	var += (int(samp_count[i])-mean)**2
var = var/word_count
var = math.sqrt(var)
var = round(var)

m = 0
for i in range(word_count):
	diff = abs(int(samp_count[i])-mean)
	state = 0
	m += 1

	if( diff>var ):
		if( int(samp_count[i])<mean ):
			state = -1
		else:
			state = 1
	
	# f: formatted :<3 means fixed 3 width
	printWord(str(i), lexicon[i], samp_count[i], state)

	if ( m>3 ):
		print("")
		m = 0

print("")
print("Mean = ", mean, "Variance = ", var)
cmd_out = subprocess.getoutput("scripts/train/check_audio.sh")
print(cmd_out)