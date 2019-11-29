#include <stdio.h>
#include "Ngram.h"
#include <cstring>
#include <fstream>
#include <map>
#include <vector>
#include <sstream>
#include <tuple>
#include <algorithm>

using namespace std;

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
        //const char lm_filename[] = lm_path.c_str();
        File lmFile( lm_path.c_str(), "r" );
        lm.read(lmFile);
        lmFile.close();
    }

    // loading .map file
    ifstream zhuYin2Big5_file(zhuYin2Big5_map_path);
    map<string, vector<string>> zhuYin2Big5_dic;
    ofstream output(output_path);

    string eachline = "";
    while(getline(zhuYin2Big5_file, eachline)){
        int tab_idx = eachline.find('\t');
        string key = eachline.substr(0, tab_idx);
        string value_substr = eachline.substr(tab_idx+1);
        istringstream value_stream(value_substr);
        string value = "";
        vector<string> value_lst;
        while(value_stream >> value){
            value_lst.push_back(value);
        }
        
        zhuYin2Big5_dic[key] = value_lst;
        /*
        if(idx < 5){
            output << key << " ";
            for(int i=0;i<zhuYin2Big5_dic[key].size();i++){
                output << zhuYin2Big5_dic[key][i] << " ";
            }
            output << '\n';
        }
        ++idx;
        */
    }

    zhuYin2Big5_file.close();

    // Do Viterbi
    ifstream input(seg_file_path);

    while(getline(input, eachline)){
        // start token
        output << "<s> " ;
        // load each word
        istringstream eachword(eachline);
        string word = "";

        // 2d viterbi
        /*
        tuple content:
        state word, its vocabindex, its logProb, its max prev word
        */
        vector<vector<tuple<string, VocabIndex, double, string>>> delta;
        
        // first word
        eachword >> word;

        vector<tuple<string, VocabIndex, double, string>> word_state_1;
        vector<string> candidate_lst = zhuYin2Big5_dic[word];

        for(int i=0;i<candidate_lst.size();i++){
            string word_candidate = candidate_lst[i];
            VocabIndex wid = voc.getIndex(word_candidate.c_str());
            if(wid == Vocab_None){
                wid = voc.getIndex(Vocab_Unknown);
            }
            VocabIndex context[] = {Vocab_None};
            double word_logprob = lm.wordProb(wid, context);
            tuple<string, VocabIndex, double, string> t = make_tuple(word_candidate, wid, word_logprob, "");
            word_state_1.push_back(t);

            
        }
        delta.push_back(word_state_1);

        // record prev state
        vector<tuple<string, VocabIndex, double, string>> word_state_prev = word_state_1;

        while(eachword >> word){
            vector<tuple<string, VocabIndex, double, string>> word_state_now;
            vector<string> candidate_lst = zhuYin2Big5_dic[word];

            for(int i=0;i<candidate_lst.size();i++){
                string word_candidate = candidate_lst[i];
                VocabIndex wid = voc.getIndex(word_candidate.c_str());
                if(wid == Vocab_None){
                    wid = voc.getIndex(Vocab_Unknown);
                }
                
                // search previous state and take out their prob
                double maxprob = 0;
                string maxword = "";
                for(int j=0;j<word_state_prev.size();j++){
                    string prev_word = get<0>(word_state_prev[j]);
                    VocabIndex prev_wid = get<1>(word_state_prev[j]);
                    double prev_word_logprob = get<2>(word_state_prev[j]);
                    
                    VocabIndex context[] = {prev_wid, Vocab_None};

                    // try multiplyting with -1
                    double word_logprob = -1 * lm.wordProb(wid, context) * prev_word_logprob;

                    if(j == 0 || word_logprob > maxprob){
                        maxprob = word_logprob;
                        maxword = prev_word;
                    }

                }

                tuple<string, VocabIndex, double, string> t = make_tuple(word_candidate, wid, maxprob, maxword);
                word_state_now.push_back(t);
            }

            delta.push_back(word_state_now);
            word_state_prev = word_state_now;

        }

        // reverse-iterate the delta vector
        vector<string> reverse_result;
        double maxProb = 0;
        string maxWord = "";
        string maxPrevWord = "";
        // last word
        vector<tuple<string, VocabIndex, double, string>> last_word_state = delta[delta.size()-1];
        for(int i=0;i<last_word_state.size();i++){
            tuple<string, VocabIndex, double, string> t = last_word_state[i];
            if(i == 0 || get<2>(t) > maxProb){
                maxProb = get<2>(t);
                maxWord = get<0>(t);
                maxPrevWord = get<3>(t);
            }
        }
        reverse_result.push_back(maxWord);
        reverse_result.push_back(maxPrevWord);

        //output << " " << maxWord << " " << maxPrevWord;

        for(int i=delta.size()-2;i>=1;i--){
            vector<tuple<string, VocabIndex, double, string>> this_word_state = delta[i];
            for(int j=0;j<this_word_state.size();j++){
                tuple<string, VocabIndex, double, string> t = this_word_state[j];
                if(get<0>(t) == maxPrevWord){
                    maxPrevWord = get<3>(t);
                    reverse_result.push_back(maxPrevWord);
                    
                    //output << " " << maxPrevWord;

                    break;
                }
            }

        }

        // reverse it
        // cannot use this stl since for big5 encoding str, each word's size is not '1' char, 2 instead
        //reverse(reverse_result.begin(), reverse_result.end());
        for(int i=reverse_result.size()-1;i>=0;i--){
            output << reverse_result[i] << " ";
        }
        
        output << "</s>\n";

    }

    output.close();

    return 0;
    
}