/*
*   Project name:   Convolutional encoder/decoder
*   Author:         Martin Krajči
*   Date:           16.12.2020
*/

#include "bms.h"

using namespace std;

Encoder *Encoder::encoder;
Decoder *Decoder::decoder;


/*
*   Constructor for copying object of same class.
*/
ViterbiState::ViterbiState(ViterbiState *state)
{
        actualBits = state->actualBits;
        totalHemmingWeight = state->totalHemmingWeight;
        input = state->input;
}

/*
*   Decault constructor for class ViterbiState
*/
ViterbiState::ViterbiState()
{
        actualBits;
        totalHemmingWeight = 0;
        input;
}

/*
*   Search through all the results and find the one with lowest hamming weight. If two or more
*   states have same hamming weight, the last one found is choosen. 
*/
string Decoder::find_possible_input()
{
    int state;
    int resState = 0;
    int resStateHemmingWeight = viterbiAlg[0]->totalHemmingWeight;
    int inputSize;
    string res;

    for(vector<ViterbiState*>::iterator stateIt = viterbiAlg.begin(); stateIt != viterbiAlg.end(); stateIt++)
    {
        state = distance(viterbiAlg.begin(), stateIt);
        if (resStateHemmingWeight >= viterbiAlg[state]->totalHemmingWeight)
        {
            resState = state;
            resStateHemmingWeight = viterbiAlg[state]->totalHemmingWeight;
        }
    }
    
    viterbiAlg[resState]->input = viterbiAlg[resState]->input.erase(0, delayBlocksNum);
    inputSize = viterbiAlg[resState]->input.size();

    for (int i = 0; i <= (inputSize/8); i++)
    {
        res += string(1, char(bitset<8>(viterbiAlg[resState]->input.substr(0, 8)).to_ulong()));
        viterbiAlg[resState]->input = viterbiAlg[resState]->input.erase(0, 8);
    }
    

    return res;
}

/*
*   Calculate hamming distance between two strings.
*/
int Decoder::hamming_distance(string possOutput, string recOutput)
{
    int distance = 0;
    for (size_t i = 0; i < possOutput.size(); i++)
    {
        distance += possOutput[i] != recOutput[i];
    }
    return distance;
}

/*
*   Find position of wanted state, defined by bit string, in trellis graph.
*/ 
int Decoder::find_state_in_trellis(string stateBits)
{  
    for(size_t state = 0; state < trellisGraph.size(); state++)
    {
        if(trellisGraph[state]->bits == stateBits)
        {
            return state;
        }
    }
    return -1;
}

/*
*   Make one step of viterbi algorithm. Take two bits in form of string as argument.
*/
void Decoder::viterbi_step(string output)
{
    ViterbiState *tempState;
    int stateTrell;
    int viterbiAlgSize = viterbiAlg.size();
    int actualHemmingWeight = 0;

    for(int state = 0; state < viterbiAlgSize; state++)
    {
        if(viterbiAlg[state]->totalHemmingWeight > 6)
        {
            continue;
        }
        stateTrell = find_state_in_trellis(viterbiAlg[state]->actualBits);
        tempState = new ViterbiState(viterbiAlg[state]);
        tempState->actualBits = trellisGraph[stateTrell]->nextState1->bits;
        actualHemmingWeight = hamming_distance(trellisGraph[stateTrell]->output1, output);
        tempState->totalHemmingWeight += actualHemmingWeight;
        tempState->errBitsTogether = actualHemmingWeight > 0 ? tempState->errBitsTogether + 1 : 0;
        tempState->input = trellisGraph[stateTrell]->input1 + tempState->input;
        viterbiAlg.push_back(tempState);
        tempState = new ViterbiState(viterbiAlg[state]);
        tempState->actualBits = trellisGraph[stateTrell]->nextState2->bits;
        actualHemmingWeight = hamming_distance(trellisGraph[stateTrell]->output2, output);
        tempState->totalHemmingWeight += actualHemmingWeight;
        tempState->errBitsTogether = actualHemmingWeight > 0 ? tempState->errBitsTogether + 1 : 0;
        tempState->input = trellisGraph[stateTrell]->input2 + tempState->input;
        viterbiAlg.push_back(tempState);
    }

    for (int state = 0; state < viterbiAlgSize; state++)
    {
        delete viterbiAlg.front();
        viterbiAlg.erase(viterbiAlg.begin());
    }
    
}

/*
*   Method is implementing the viterbi algorithm and return possible output of decoder.
*/
string Decoder::viterbi(string toDecode)
{
    int toDecodeSize = toDecode.size()/2;
    ViterbiState *initState = new ViterbiState;
    initState->actualBits = string(delayBlocksNum, '0');
    viterbiAlg.push_back(initState);
    for (int i = 0; i < toDecodeSize; i++)
    {
        viterbi_step(toDecode.substr(toDecode.size() - 2,toDecode.size()));
        toDecode.erase(toDecode.size() - 2,toDecode.size());
    }
    
    return find_possible_input();
}

/*
*   Auxiliary method. Find state defined by string of bits in trellis graph so all states can
*   connect to its possible next states.
*/
TrellisState *Decoder::find_next_state(string  nextStateBits)
{
    for(size_t state = 0; state < trellisGraph.size(); state++)
    {
        if(trellisGraph[state]->bits == nextStateBits)
        {
            return trellisGraph[state];
        }
    }
    return NULL;
}

/*
*   Calculate upper and downer feedback by xoring given string of bits. Position of xored bits
*   depends on arguments higherBits and lowerBits.
*/
string Decoder::xor_bin_str(string binary_str, string higherBits, string lowerBits)
{
    int higherNum = 0;
    int lowerNum = 0;
    string tempRes;
    for(size_t i = 0; i <= binary_str.size(); i++)
    {
        if (higherBits.c_str()[i] == '1')
        {
            if (binary_str.c_str()[i] == '1')
            {
                higherNum += 1;
            }
        }

        if(lowerBits.c_str()[i] == '1')
        {
            if (binary_str.c_str()[i] == '1')
            {
                lowerNum += 1;
            }
        }
    }
    tempRes = higherNum % 2 ? '1' : '0';
    tempRes += lowerNum % 2 ? '1' : '0';
    return tempRes;
}

/*
*   Generate trellis graph.
*/
void Decoder::gen_graph()
{
    for (int i = 0; i < pow(2, delayBlocksNum); i++)
    {
        trellisGraph.push_back(new TrellisState);
        trellisGraph[i]->bits = bitset<sizeof(int)*8>(i).to_string();
        trellisGraph[i]->bits = trellisGraph[i]->bits.substr(trellisGraph[i]->bits.size() - delayBlocksNum, string::npos);
    }

    for(vector<TrellisState*>::iterator state = begin(trellisGraph); state != end(trellisGraph); state++)
    {
        (*state)->output1 = xor_bin_str((*state)->input1 + (*state)->bits, higherBits, lowerBits);
        (*state)->nextState1 = find_next_state((*state)->input1 + (*state)->bits.substr(0, (*state)->bits.size() - 1));
        (*state)->output2 = xor_bin_str((*state)->input2 + (*state)->bits, higherBits, lowerBits);
        (*state)->nextState2 = find_next_state((*state)->input2 + (*state)->bits.substr(0, (*state)->bits.size() - 1));
    }
}

/*
*   Decode given string of bits. First generate trellis graph, then use viterbi algorithm
*   to find possible input.
*/
string Decoder::decode(string toDecode)
{
    gen_graph();
    string binary_string;
    string tempRes;
    for(char& character : toDecode)
    {
        if(character != '0' && character != '1') continue;
        binary_string += character;
    }
    if(binary_string.empty())
    {
        throw "No characters to decode. Input has to consist of characters [0 - 1]\n";
    }
    tempRes = viterbi(binary_string);
    for(char& character : tempRes)
    {
        if(!(('0' <= character && character <= '9') || ('A' <= character && character <= 'Z') || ('a' <= character && character <= 'z'))) continue;
        res += character;
    }
    for(size_t state = 0; state < trellisGraph.size(); state++)
    {
        delete trellisGraph[state];
    }
    for(size_t state = 0; state < viterbiAlg.size(); state++)
    {
        delete viterbiAlg[state];
    }
    return res;
}

/*
*   Method to get pointer to the only one pointer to object of Decoder class because singleton
*   is used.
*/
Decoder *Decoder::get_decoder(int delayBlocksNum, string higherBits, string lowerBits)
{
    if(decoder == 0)
    {
        decoder = new Decoder(delayBlocksNum, higherBits, lowerBits);
    }
    return decoder;
}

/*
*   Decoder constructor.
*/
Decoder::Decoder(int delayBlocksNum, string higherBits, string lowerBits)
{
    this->delayBlocksNum = delayBlocksNum;
    this->higherBits = higherBits;
    this->lowerBits = lowerBits;
}

/*
*   Calculate upper and downer feedback by xoring given string of bits. Position of xored bits
*   depends on arguments higherBits and lowerBits.
*/
string Encoder::xor_bin_str(string binary_str)
{
    int higherNum = 0;
    int lowerNum = 0;
    string tempRes;
    for(size_t i = 0; i <= binary_str.size(); i++)
    {
        if (higherBits.c_str()[i] == '1')
        {
            if (binary_str.c_str()[i] == '1')
            {
                higherNum += 1;
            }
        }

        if(lowerBits.c_str()[i] == '1')
        {
            if (binary_str.c_str()[i] == '1')
            {
                lowerNum += 1;
            }
        }
    }
    tempRes = higherNum % 2 ? '1' : '0';
    tempRes += lowerNum % 2 ? '1' : '0';
    return tempRes;
}

/*
*   Encode given string of ASCII characters.
*/
string Encoder::encode(string toEncode)
{
    string binary_string;
    for(char& character : toEncode)
    {
        if(!(('0' <= character && character <= '9') || ('A' <= character && character <= 'Z') || ('a' <= character && character <= 'z'))) continue;
        binary_string += bitset<8>(character).to_string();
    }
    if(binary_string.empty())
    {
        throw "No characters to encode. Input has to consist of characters [a - z][A - Z][0 - 9]\n";
    }
    binary_string += string(delayBlocksNum, '0');
    for (size_t i = 0; i < binary_string.size(); i++)
    {
        res = xor_bin_str(binary_string.substr(binary_string.size() - delayBlocksNum - 1, string::npos)) + res;
        binary_string.pop_back();
        binary_string  = '0' + binary_string;
    }
    return res;
}

/*
*   Method to get pointer to the only one pointer to object of Encoder class because singleton
*   is used.
*/
Encoder *Encoder::get_encoder(int delayBlocksNum, string higherBits, string lowerBits)
{
    if(encoder == 0)
    {
        encoder = new Encoder(delayBlocksNum, higherBits, lowerBits);
    }
    return encoder;
}

/* 
*   Encoder constructor.
*/
Encoder::Encoder(int delayBlocksNum, string higherBits, string lowerBits)
{
    this->delayBlocksNum = delayBlocksNum;
    this->higherBits = higherBits;
    this->lowerBits = lowerBits;
}

int main(int argc, char** argv)
{
	int opt;
    int delayBlocksNum = 5;
    int higher_conn = 53;
    int lower_conn = 46;
    bool encode = false;
    bool decode = false;
    bool notDefault = false;
    string toEncode;
    string toDecode;
    string res;
    string temp_input;
    string higherBits;
    string lowerBits;
    Encoder *encoder;
    Decoder *decoder;

    while((opt = getopt(argc, argv, "edhb")) != -1)  
    {  
        switch(opt)  
        {  
            case 'e': 
                encode = true;              
            	break;
            case 'd':
                decode = true;
            	break;
            case 'h':
                cout << "This simple command line aplication is able to encode and decode\n";
                cout << "messages, which consist of ASCII characters. Use \"-e\" to encode\n";
                cout << "message, and \"-d\" to decode message. By default, convolutional\n";
                cout << "coder is set with 5 delay blocks, 53 as upper feedback and 46 as\n";
                cout << "lower feedback. If you want to use your own numbers for setting up\n";
                cout << "the encoder/decoder, use parameter -b. Example might look like\n";
                cout << "./bms -e -b 5 53 46 <<< A. Input is always taken from standard input\n";
                cout << "Resctrictions: Maximum number of delay blocks for decoder is 31.\n";
                return 0; 
                break;
            case 'b':
                notDefault = true;
                break;
            case '?':  
                return 1;
                break;  
        }  
    }
    

    if ((encode && decode) || (!encode && !decode))
    {
            cerr << "Operation not specified. Use \"./bms -h\" for help.\n";
            return 1;
    }
    
    if(optind < argc && notDefault)
    {
        if((argc - optind) != 3)
        {
            cerr << "Invalid parameters for -b option. Pass 3 numbers to setup coder or use default one.\n";
            return 1;
        }   
        try
        {
            delayBlocksNum = stoi(argv[optind++], NULL, 10);
            higher_conn = stoi(argv[optind++], NULL, 10);
            lower_conn = stoi(argv[optind++], NULL, 10);
        }
        catch(exception& e)
        {
            cerr << "Invalid parameters for -b option. Pass 3 numbers to setup coder or use default one.\n";
            return 1;
        }
    }
    else if (optind == argc && notDefault)
    {
        cerr << "Missing parameters for -b option. Use \"./bms -h\" for help.\n";
        return 1;
    }
    
    else if(optind < argc)
    {
        cerr << "Invalid arguments. Use \"./bms -h\" for help.\n";
        return 1;
    }
    

    try
    {
        higherBits = bitset<sizeof(int)*8>(higher_conn).to_string();
        higherBits = higherBits.substr(higherBits.size() - delayBlocksNum - 1, string::npos);
        lowerBits = bitset<sizeof(int)*8>(lower_conn).to_string();
        lowerBits = lowerBits.substr(lowerBits.size() - delayBlocksNum - 1, string::npos);
    }
    catch(...)
    {
        cerr << "Unknown error\n";
        return 1;
    }
    

    if (encode)
    {
        while(getline(cin, temp_input))
        {
            toEncode += temp_input;
        }

        encoder = Encoder::get_encoder(delayBlocksNum, higherBits, lowerBits);
        cin >> toEncode;
        try
        {
            res = (*encoder).encode(toEncode);   
        }
        catch(const char* err)
        {
            cerr << err;
            return 1;
        }
        catch(...)
        {
            cerr << "Unknown error\n";
            return 1;
        }

        cout << res << "\n";
    }

    if (decode)
    {
        while(getline(cin, temp_input))
        {
            toDecode += temp_input;
        }

        decoder = Decoder::get_decoder(delayBlocksNum, higherBits, lowerBits);
        cin >> toDecode;
        try
        {
            res = (*decoder).decode(toDecode);
        }
        catch(const char* err)
        {
            cerr << err;
            return 1;
        }
        catch(...)
        {
            cerr << "Unknown error";
            return 1;
        }

        cout << res << "\n";
    }

	return 0;
}