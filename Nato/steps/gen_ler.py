# written in python 3
# Author: Dr Akhavan JUN-2023
# Example: python3 steps/gen_ler.py exp/tri1/graph exp/tri1/decode

import random
import os
import sys
import subprocess # runing bash cmd
import math # floor float
import array

GRA = sys.argv[1] #1
DIR = sys.argv[2] #2
AUD_PATH = "audio/train"
WRD_FILE = "word_list"
SYM_FILE = f"{GRA}/words.txt"
OUT_FILE = f"{DIR}/scoring/ler_stat"
MIN_LMWT = 7
MAX_LMWT = 17

def levenshtein_distance(s1, s2):
	s1 = s1.split()
	s2 = s2.split()
	m = len(s1)
	n = len(s2)

	# Create a matrix to store the edit distances
	dp = [[0] * (n + 1) for _ in range(m + 1)]

	# Initialize the matrix
	for i in range(m + 1):
		dp[i][0] = i
	for j in range(n + 1):
		dp[0][j] = j

	# Fill in the matrix
	for i in range(1, m + 1):
		for j in range(1, n + 1):
			if s1[i - 1] == s2[j - 1]:
				dp[i][j] = dp[i - 1][j - 1]
			else:
				dp[i][j] = 1 + min(dp[i - 1][j],        # Deletion
								   dp[i][j - 1],        # Insertion
								   dp[i - 1][j - 1])    # Substitution

	# Trace back the operations to find the differences
	i = m
	j = n
	differences = []
	while i > 0 and j > 0:
		if s1[i - 1] == s2[j - 1]:
			i -= 1
			j -= 1
		elif dp[i][j] == 1 + dp[i - 1][j - 1]:    # Substitution
			differences.append(s1[i - 1])
			i -= 1
			j -= 1
		elif dp[i][j] == 1 + dp[i - 1][j]:        # Deletion
			differences.append(s1[i - 1])
			i -= 1
		elif dp[i][j] == 1 + dp[i][j - 1]:        # Insertion
			differences.append(s2[j - 1])
			j -= 1

	while i > 0:
		differences.append(s1[i - 1])
		i -= 1
	while j > 0:
		differences.append(s2[j - 1])
		j -= 1

	differences.reverse()
	return differences

def printWord(word):
	if( len(word)<2 ):
		return

	w_filename = word[0]
	w_filename = w_filename.split('.')[0]
	word_id = w_filename.split('_')

	orig_id = ""
	orig = "| "
	for i in range(1,len(word_id)):
		orig += lexicon[int(word_id[i])]
		orig += ","
		orig_id += word_id[i]
		orig_id += "_"
	orig = orig[:-1] #remove last char
	orig_id = orig_id[:-1] #remove last char

	det = " ->  "
	for i in range(1,len(word)):
		det += symbols[int(word[i])]
		det += ","
	det = det[:-1] #remove last char

	is_wrong = 0
	if( len(word)!=len(word_id) ):
		is_wrong = 1

	s1 = []
	s2 = []
	for i in range(1,len(word_id)):
		s1.append(lexicon[int(word_id[i])])
	for i in range(1,len(word)):
		s2.append(symbols[int(word[i])])
	
	s1 = ' '.join(s1)
	s2 = ' '.join(s2)
	
	dif = levenshtein_distance(s1, s2)
	
	for j in range(len(dif)):
		for i in range(len(lexicon)):
			if(lexicon[i]==dif[j]):
				ler[i]+=1

word_file = open(WRD_FILE)
syms_file = open(SYM_FILE)
out_file  = open(OUT_FILE, "w")
lexicon = word_file.read().splitlines()
symbols = syms_file.read().splitlines()
lex_len = len(lexicon)
ler = array.array('i', [0] * lex_len)


for i in range(1,len(symbols)):
	symbols[i] = symbols[i].split()[0]


for i in [16]:
	sc_filename = f"{DIR}/scoring/{i}.tra"
	for line in open(sc_filename):
		word = line.split()
		printWord(word)
		
	for i in range(lex_len):
		out_file.write(f"{lexicon[i]} {ler[i]}\n")
		if( ler[i]>0 ):
			print(lexicon[i], ler[i])
  
out_file.close()

#remove duplicates
os.system("sort -u -o input input")