#include <iostream>
#include <fstream>

#include <map>
#include <utility>

#include <string>
#include <sstream>
#include <cstring>
#include <vector>

using namespace std;

map<string, vector<string> > init_vocabulary(string corpus_path);
map<pair<string, string>, int> count_pairs(map<string, vector<string> > split_chars);
map<string, int> create_vocabulary(map<pair<string, string>, int>& pair_freq, map<string, vector<string> >& split_chars, int num_tokens);
pair<pair<string, string>, int> get_most_freq_pair(map<pair<string, string>, int> pair_freq);
string merge(map<pair<string, string>, int>& pair_freq, map<string, vector<string> >& split_chars);
void to_json(string output_file, map<string, int> vocabulary);

// debugging functions
void print_split_chars(map<string, vector<string> >& split_chars);
void print_pair_freq(map<pair<string, string>, int>& pair_freq);

int main()
{
    map<string, vector<string> > split_chars = init_vocabulary("corpus.txt");
    map<pair<string, string>, int> pair_freq = count_pairs(split_chars);
    map<string, int> vocabulary = create_vocabulary(pair_freq, split_chars, 50000);

    to_json("tokenized.json", vocabulary);
}

map<string, vector<string> > init_vocabulary(string corpus_path)
{
    ifstream corpus_file(corpus_path);
    if(!corpus_file.is_open()){
        throw "Error opening file";
    }

    map<string, vector<string> > word_map;
    string word_key;

    while(corpus_file >> word_key){
        stringstream ss(word_key);
        int word_size = word_key.length();
        vector<string> split_word;

        for(int i = 0; i < word_size; i++){
            if (i == word_size - 1){
                split_word.push_back("</w>");
            } else {
                char holder;
                ss >> holder;
                split_word.push_back(string(1, holder));
            }
        }
        word_map[word_key] = split_word;
        ss.clear();
    }

    corpus_file.close();
    return word_map;
}

map<pair<string, string>, int> count_pairs(map<string, vector<string> > split_chars){
    map<pair<string, string>, int> pair_freq;
    map<string, vector<string> >::iterator itr;
    for(itr = split_chars.begin(); itr != split_chars.end(); itr++){
        vector<string> word_arry = itr->second;
        int word_size = itr->first.length();

        for(int i = 0; i < word_arry.size() - 1; i++){
            pair<string, string> char_pair(word_arry[i], word_arry[i + 1]);
            pair_freq[char_pair]++;
        }
    }
    return pair_freq;
}

map<string, int> create_vocabulary(map<pair<string, string>, int>& pair_freq, map<string, vector<string> >& split_chars, int num_tokens){
    string initial_vocab = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.,?!:;'\"-()$&*+-=/";

    map<string, int> vocabulary;
    int token_idx = 0;
    
    // create initial vocabulary
    vocabulary[" "] = token_idx;
    vocabulary["<unk>"] = ++token_idx;
    vocabulary["</w>"] = ++token_idx;
    for(int i = 0; i < initial_vocab.length(); i++){
        string word_key = string(1, initial_vocab[i]);
        vocabulary[word_key] = ++token_idx;
    }

    if(num_tokens < token_idx){
        cout << "Invalid token count" << endl;
        throw("Invalid token count");
    }

    // merge iteratively
    num_tokens -= token_idx;

    for(int i = 0; i < num_tokens; i++){
        cout << i << "/" << num_tokens << "completed!" << endl;
        string new_vocab_word = merge(pair_freq, split_chars);
        vocabulary[new_vocab_word] = token_idx++;
    }

    return vocabulary;
}

string merge(map<pair<string, string>, int>& pair_freq, map<string, vector<string> >& split_chars){
    pair<pair<string, string>, int> most_freq_pair = get_most_freq_pair(pair_freq);

    string string1 = most_freq_pair.first.first;
    string string2 = most_freq_pair.first.second;

    string new_subword = string1 + string2;
    
    // first merge in the split_chars map
    map<string, vector<string> >::iterator itr;
    for(itr = split_chars.begin(); itr != split_chars.end(); itr++){
        vector<string> word_arr = itr->second;
        int word_size = itr->first.length();

        for(int i = 0; i < word_arr.size() - 1; i++){
            if(word_arr[i] == string1 && word_arr[i + 1] == string2){
                word_arr[i] = new_subword;
                word_arr.erase(word_arr.begin() + i + 1);
                split_chars[itr->first] = word_arr;
            }
        }
    }

    // set pair_freq again
    pair_freq = count_pairs(split_chars);

    return new_subword;
}

pair<pair<string, string>, int> get_most_freq_pair(map<pair<string, string>, int> pair_freq){
    int max_freq = 0;
    pair<pair<string, string>, int> return_pair;

    map<pair<string, string>, int>::iterator itr;
    for(itr = pair_freq.begin(); itr != pair_freq.end(); itr++){
        if(itr->second > max_freq){
            max_freq = itr->second;
            return_pair = *itr;
        }
    }
    return return_pair;
}

void to_json(string output_file, map<string, int> vocabulary){
    ofstream output_json(output_file);
    output_json << "{\n";

    map<string, int>::iterator itr;
    for(itr = vocabulary.begin(); itr != vocabulary.end(); itr++){
        string formatted_string = "\t\"" + itr->first + "\": ";
        if(itr->first == "\""){
            formatted_string = "\t\"\\" + itr->first + "\": ";
        }
        output_json << formatted_string << itr->second << (next(itr) == vocabulary.end() && itr != vocabulary.end() ? "\n" : ",\n");
    }
    
    output_json << "}";
}

// debugging functions
void print_pair_freq(map<pair<string, string>, int>& pair_freq){
    map<pair<string, string>,int>::iterator itr1;
    for(itr1 = pair_freq.begin(); itr1 != pair_freq.end(); itr1++){
        cout << itr1->first.first << itr1->first.second << ": " << itr1->second << endl;
    }
}

void print_split_chars(map<string, vector<string> >& split_chars){
    map<string, vector<string> >::iterator itr2;
    for(itr2 = split_chars.begin(); itr2 != split_chars.end(); itr2++){
        cout << itr2->first << ": ["; 
        for(int i = 0; i < itr2->second.size(); i++){
            cout << itr2->second[i] << ", "; 
        }
        cout << "]" << endl;
    }
}