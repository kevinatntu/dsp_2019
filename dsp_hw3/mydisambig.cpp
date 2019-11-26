#include <stdio.h>
#include "Ngram.h"
#include <cstring>

/*
./mydisambig $1 $2 $3 $4
$1 segemented file to be decoded
$2 ZhuYin-Big5 mapping
$3 language model
$4 output file
*/

int main(int argc, char *argv[])
{
    if(argc != 5){
        cout << "Incorrect #param\n";
    } 

    string seg_file_path = argv[1];
    string zhuYin2Big5_map_path = argv[2];
    string lm_path = argv[3];
    string output_path = argv[4];

    int ngram_order = 2; // Bi-gram
    Vocab voc;
    Ngram lm( voc, ngram_order );

    {
        const char lm_filename[] = lm_path;
        File lmFile( lm_filename, "r" );
        lm.read(lmFile);
        lmFile.close();
    }

    VocabIndex wid = voc.getIndex("Ê¨");
    if(wid == Vocab_None) {
        printf("No word with wid = %d\n", wid);
        printf("where Vocab_None is %d\n", Vocab_None);
    }

    wid = voc.getIndex("ÄÑ");
    VocabIndex context[] = {voc.getIndex("©Ô"), Vocab_None};
    printf("log Prob(ÄÑ|©Ô) = %f\n", lm.wordProb(wid, context));
}