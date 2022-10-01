/*
*   Project name:   Convolutional encoder/decoder
*   Author:         Martin Krajči
*   Date:           16.12.2020
*/

#include <iostream>
#include <string>
#include <vector>
#include <bitset>
#include <algorithm>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>

using namespace std;

class TrellisState {
    public:
        string bits;
        string input1 = "0";
        string output1;
        TrellisState *nextState1;
        string input2 = "1";
        string output2;
        TrellisState *nextState2;
};

class ViterbiState {
    public:
        string actualBits;
        int totalHemmingWeight = 0;
        int errBitsTogether = 0;
        string input;

        ViterbiState();
        ViterbiState(ViterbiState *state);
};

class Decoder {
    static Decoder *decoder;
    int delayBlocksNum;
    string higherBits;
    string lowerBits;
    vector<TrellisState*> trellisGraph;
    vector<ViterbiState*> viterbiAlg;
    string res;

    Decoder(int delayBlocksNum, string higherBits, string lowerBits);
    void gen_graph();
    TrellisState *find_next_state(string nextStateBits);
    string xor_bin_str(string binary_str, string higherBits, string lowerBits);
    string viterbi(string toDecode);
    void viterbi_step(string output);
    int hamming_distance(string possOutput, string recOutput);
    string find_possible_input();
    int find_state_in_trellis(string stateBits);

    public:
        static Decoder *get_decoder(int delayBlocksNum, string higherBits, string lowerBits);
        string decode(string toDecode);

};

class Encoder {
        int delayBlocksNum;
        string higherBits;
        string lowerBits;
        string res;
        static Encoder *encoder;

        Encoder(int delayBlocksNum, string higherBits, string lowerBits);
        string xor_bin_str(string binary_str);

    public:
        static Encoder *get_encoder(int delayBlocksNum, string higherBits, string lowerBits);
        string encode(string toEncode);

};