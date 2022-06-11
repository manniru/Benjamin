#ifndef BT_CONFIG_H
#define BT_CONFIG_H

#define KAL_NATO_DIR      "../Nato/"
#define KAL_DECODE_DIR    KAL_NATO_DIR"decode/"
#define KAL_SI_DIR        KAL_NATO_DIR"scripts/interpreter/"
#define KAL_SD_DIR        KAL_NATO_DIR"scripts/decode/"
#define KAL_SK_DIR        KAL_NATO_DIR"scripts/kaldi/"
#define KAL_AU_DIR        KAL_NATO_DIR"audio/"
#define KAL_RESULT_DIR    KAL_DECODE_DIR"result/"
#define KAL_WAV_DIR       KAL_DECODE_DIR"wav/"
#define KAL_MODE          "tri1"

#define BT_WORDS_PATH     KAL_NATO_DIR "exp/" KAL_MODE "/graph/words.txt"
#define BT_OAMDL_PATH     KAL_NATO_DIR "exp/" KAL_MODE "_online/final.oalimdl" //Online Alignment
#define BT_FST_PATH       KAL_NATO_DIR "exp/" KAL_MODE "/graph/HCLG.fst"
#define BT_GCMVN_PATH     KAL_NATO_DIR"exp/tri1_online/global_cmvn.stats"
#define BT_WORDLIST_PATH  KAL_NATO_DIR"word_list"
#define BT_BAR_RESULT     "Resources/bar_result"
#define BT_GRAPH_PATH     "graph"

#define KAL_CONF_TRESHOLD  0.9
#define KAL_UDET_TRESHOLD  0.7  //Utterance Total Detection
#define KAL_UCON_TRESHOLD  0.9  //Utterance Total Confidence
#define KAL_HARD_TRESHOLD  0.5  //word will be removed

#define BT_REC_RATE       16000 //Recording Sample Rate
#define BT_BUF_SIZE       100   //Buffer Size Second(Max Utterance Len)
#define BT_FFT_SIZE       512   //Rounded Window Size to Power of 2 (400->512)
#define BT_DEC_TIMEOUT    0.5   //Decode Timeout in Second

#define BT_WAV_MAX        100 // Max recording number(circular)
#define BT_TRA_MAX        5   // Max number of same sequence train

#endif // BT_CONFIG_H


//sample_rate: 16000 channel: 2 chunk_size: 57344
//Check Reset " E:15ms"
//Check Reset "S:0ms F:0ms C:30ms R:27ms P:68ms D:7ms E:193ms"
//>>>  1.93 194 "delta oscar mike " 3
//Check Reset "S:0ms F:0ms C:57ms R:50ms P:126ms D:11ms E:297ms"
//>>>  2.93 294 "delta oscar mike super one " 5
//Check Reset "S:0ms F:0ms C:87ms R:78ms P:193ms D:17ms E:436ms"
//>>>  3.43 394 "delta oscar mike super one november " 6
//Check Reset "S:0ms F:0ms C:108ms R:97ms P:243ms D:24ms E:523ms"
//------------Reset Sil 343 51 394

//sample_rate: 16000 channel: 2 chunk_size: 57344
//Check Reset " E:15ms"
//Check Reset "S:0ms C:7ms R:32ms P:76ms D:12ms E:189ms"
//>>>  1.93 194 "delta(110) oscar(147) mike(193) " 3
//Check Reset "S:0ms C:12ms R:57ms P:134ms D:12ms E:266ms"
//>>>  2.93 294 "delta(110) oscar(147) mike(211) super(247) one(281) november(293) " 6
//Check Reset "S:0ms C:17ms R:86ms P:209ms D:16ms E:384ms"
//>>>  3.44 394 "delta(110) oscar(147) mike(211) super(247) one(281) november(344) " 6
//Check Reset "S:0ms C:19ms R:109ms P:262ms D:25ms E:461ms"
//------------Reset Sil 344 50 394
//"<2> 3.94 400 tango(87,1) hotel(112,1) india(160,1) sierra(206,1) sim(238,1) delta(260,0.960216) echo(294,1) mike(321,1) oscar(354,1) sim(394,1) " 10
//Check Reset "S:0ms C:6ms R:118ms P:528ms D:28ms E:749ms"
//--------------------11-June-2022---------------
// "<2> 3.94 400 tango(87,1) hotel(112,1) india(160,1) sierra(206,1) sim(238,1) delta(260,0.960216) echo(294,1) mike(321,1) oscar(354,1) sim(394,1) " 10
// Reset Succ "S:0ms C:5ms A:34ms R:77ms P:511ms D:26ms E:725ms"
