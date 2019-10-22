#include "../inc/hmm.h"
//#include <cmath>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <sstream> // use stringstream to preserve double precision during output

using namespace std;

int main(int argc, char *argv[])
{
/*
	HMM hmms[5];
	load_models( "modellist.txt", hmms, 5);
	dump_models( hmms, 5);
*/
	if (argc != 4){
		cout << "You need to pass exact '3' arguments\n- model list path, seq path, result.txt path\n";
		return 1;
	}

	string modelLstPath = argv[1];
	string seqPath = argv[2];
	string resultPath = argv[3];

	// load all models in modellist.txt
	//ifstream modelLstFile(modelLstPath);

	int numModel = 0;
	string modelPath = "";
	//HMM *modelLst = (HMM*)malloc(5 * sizeof(*modelLst));
	HMM modelLst[5];
	numModel = load_models(modelLstPath.c_str(), modelLst, 5);

	// load seq
	ifstream seqFile(seqPath);
	vector<string> seqlst;
	string thisSample = "";
	int numSeq = 0;
	while(getline(seqFile, thisSample)){
		seqlst.push_back(thisSample);
		++numSeq;
	}
	seqFile.close();

	const int T = seqlst[0].length();
	string modelNamePrefix = "model_0";

	// Record each seq's best model name & its probability
	stringstream resultRec;

	int recOnlyIdx[2500];

	// iterate each seq
	for (int i=0;i<numSeq;i++){
		string seq = seqlst[i];
		double bestProb = 0;
		int bestModelIdx = -1;
		for(int m=0;m<numModel;m++){
			HMM thisModel = modelLst[m];
			double delta[T][6];
			// init delta
			char o1 = seq[0];
			for(int j=0;j<6;j++){
				delta[0][j] = thisModel.initial[j] * thisModel.observation[int(o1)-65][j];
			}

			for(int t=1;t<T;t++){
				for(int j=0;j<6;j++){
					double maxDelta_tminus1 = 0;
					for(int k=0;k<6;k++){
						double delta_tminus1 = delta[t-1][k] * thisModel.transition[k][j];
						if (delta_tminus1 > maxDelta_tminus1){
							maxDelta_tminus1 = delta_tminus1;
						}
					}
					char o_t = seq[t];
					delta[t][j] = maxDelta_tminus1 * thisModel.observation[int(o_t)-65][j];
				}
			}
			// Termination
			double P = 0;
			for(int j=0;j<6;j++){
				if (delta[T-1][j] > P){
					P = delta[T-1][j];
				}
			}

			// Compare with now best P
			if (P > bestProb){
				bestProb = P;
				bestModelIdx = m;
			}
		}

		// record best model of this seq
		resultRec << modelNamePrefix << bestModelIdx+1 << ".txt " << bestProb << '\n' ;
		recOnlyIdx[i] = bestModelIdx+1;
		//cout << bestProb << '\n';
	}


	// save
	ofstream resultFile(resultPath);
	resultFile << resultRec.str();
	resultFile.close();

	// check accuracy by comparing with 'test_lbl.txt'
	ifstream testLabel("./data/test_lbl.txt");
	string eachline = "";
	int correct = 0;
	int idx = 0;
	while(getline(testLabel, eachline)){
		char num = eachline[7];
		int label = num - '0';
		if (label == recOnlyIdx[idx]){
			correct += 1;
		}
		++idx;
	}
	testLabel.close();
	cout << "Accuracy: " << 1.0 * correct / 2500 << '\n';
	
	return 0;
}
