#include "../inc/hmm.h"
//#include <cmath>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <vector>
using namespace std;

int main(int argc, char *argv[])
{
/*
	HMM hmms[5];
	load_models( "modellist.txt", hmms, 5);
	dump_models( hmms, 5);
*/
	//ios::sync_with_stdio(false);
	if(argc != 5){
		cout << "You need to pass exact '4' arguments\n- #iter, init.txt path, train_seq.txt path, output.txt path\n";
		return 1;
	}

	/* Set up all arguments */
	int totalIter = atoi(argv[1]);
	string initPath = argv[2];
	string seqPath = argv[3];
	string outputPath = argv[4];

	HMM hmm_initial;
	loadHMM( &hmm_initial, initPath.c_str());

	const int NUMSTATE = hmm_initial.state_num;

	//cout << hmm_initial.state_num << endl;
	//dumpHMM( stderr, &hmm_initial );

	ifstream seq_1(seqPath);
	// Load each line of sample seq
	vector<string> seqlst;
	string thisSample = "";
	int numSeq = 0;
	while(getline(seq_1, thisSample)){
		seqlst.push_back(thisSample);
		++numSeq;
	}
	seq_1.close();
	//const int NUMSEQ = numSeq;

	const int T = seqlst[0].length();

	// Run #Iter times
	for(int iter=0;iter<totalIter;iter++){
		double gamma[MAX_SEQ][6] = {0};
		double gamma_k[6][6] = {0}; // 紀錄 number of observation O = k in state i: x-obser, y-state
		double epsilon[MAX_SEQ-1][6][6] = {0};

		// cout << "Epoch " << iter+1 << '\n';
		// iterate all samples
		for (int i=0;i<numSeq;i++){
			string thisSeq = seqlst[i];
			
			double alpha[MAX_SEQ][6];
			double beta[MAX_SEQ][6];

			// init alpha_1 & beta_T
			char o1 = thisSeq[0];
			for(int j=0;j<6;j++){
				alpha[0][j] = hmm_initial.initial[j] * hmm_initial.observation[int(o1)-65][j];
				beta[T-1][j] = 1;
			}

			// calcute alpha
			for (int t=1;t<T;t++){
				char o_tplus1 = thisSeq[t];
				for (int j=0;j<6;j++){
					double alpha_t = 0;
					for(int k=0;k<6;k++){
						alpha_t += alpha[t-1][k] * hmm_initial.transition[k][j];
					}
					alpha[t][j] = alpha_t * hmm_initial.observation[int(o_tplus1)-65][j];
				}
			}
			// calculate beta
			for (int t=T-2;t>=0;t--){
				char o_tplus1 = thisSeq[t+1];
				for(int k=0;k<6;k++){
					double beta_k = 0;
					for(int j=0;j<6;j++){
						beta_k += hmm_initial.transition[k][j] * hmm_initial.observation[int(o_tplus1)-65][j] * beta[t+1][j];
					}
					beta[t][k] = beta_k;
				}
			}
			// calculate gamma (accumulative)
			for (int t=0;t<T;t++){
				double numerator[6];
				double denominator = 0.0;
				char o_t = thisSeq[t];
				for(int j=0;j<6;j++){
					numerator[j] = alpha[t][j] * beta[t][j];
					denominator += numerator[j];
				}

				// notice: use "ADD" since accumative
				for(int j=0;j<6;j++){
					gamma[t][j] += numerator[j] / denominator;
					// for updating observation use
					gamma_k[int(o_t)-65][j] += numerator[j] / denominator;
				}
			}
			// calculate epsilon (accumulative)
			for (int t=0;t<T-1;t++){
				double numerator[6][6];
				double denominator = 0.0;
				char o_tplus1 = thisSeq[t+1];
				for(int k=0;k<6;k++){
					for(int j=0;j<6;j++){
						numerator[k][j] = alpha[t][k] * hmm_initial.transition[k][j] *
							hmm_initial.observation[int(o_tplus1)-65][j] * beta[t+1][j];
						denominator += numerator[k][j];
					}
				}
				// notice: use "ADD" since accumative
				for(int k=0;k<6;k++){
					for(int j=0;j<6;j++){
						epsilon[t][k][j] += numerator[k][j] / denominator;
					}
				}
			}

		}

		// update the parameters
		// update PI
		for(int i=0;i<6;i++){
			hmm_initial.initial[i] = gamma[0][i] / numSeq;
		}
		// update A
		// NOTICE: 這裡只iterate from t=1 ~ "T-1"
		for(int i=0;i<6;i++){
			double gamma_sum = 0;
			for(int t=0;t<T-1;t++){
				gamma_sum += gamma[t][i];
			}
			for(int j=0;j<6;j++){
				double epsilon_sum = 0;
				for(int t=0;t<T-1;t++){
					epsilon_sum += epsilon[t][i][j];
				}
				hmm_initial.transition[i][j] = epsilon_sum / gamma_sum;
			}
		}
		// Update B: GAMMA需多分開計算"每一種observation type"出現之機率和
		for(int i=0;i<6;i++){
			double gamma_sum = 0;
			for(int t=0;t<T;t++){
				gamma_sum += gamma[t][i];
			}
			for(int k=0;k<6;k++){
				hmm_initial.observation[k][i] = gamma_k[k][i] / gamma_sum;
			}
		}

		//dumpHMM( stderr, &hmm_initial );
		// check sum of row/column of A/B
		/*
		for(int i=0;i<6;i++){
			double A_row = 0;
			double B_row = 0;
			for(int j=0;j<6;j++){
				A_row += hmm_initial.transition[i][j];
				B_row += hmm_initial.observation[j][i];
			}
			cout << "1: " << A_row << " " << B_row << '\n';
		}
		*/
		// Test each epoch's HMM result
		//dumpHMM(stderr, &hmm_initial);
	}
	
	// Save output
	//ofstream output(outputPath);
	FILE *output = open_or_die(outputPath.c_str(), "w");
	dumpHMM(output, &hmm_initial);

	fclose(output);

	//printf("log(0.5) = %f\n", log(1.5) );
	return 0;
	
}
