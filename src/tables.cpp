/*Algorithm:
    Store information by letter.
    Process information told to us (i.e. directly given by the score).
    Process inferred information repeatedly until there are no more inferences to be made.

    Information on a letter:
        amount:    at least how many of the letter are in the word

        capped:    1) we are told it's capped - we have a gray
                   2) we know all 5 letters (sum of amounts == 5)
                   3) there are no more spots for this letter to go

        known_pos: 1) we see a green
                   2) there are just enough possible spots for the amount

        bad_pos:   1) it's yellow or gray
                   2) if we know everything about this letter,
                      i.e. the amount is capped and we know all the positions for this letter,
                      then all the rest are bad (i.e. bad_pos == known_pos)
                   3) any spot that must have some other letter there
                      (eg: L='a', 'b'.ppos:10001, 'c'.ppos:'10001'; first and last spots must have a&c)
                      (eg: L='a', there's a green 'b' in the first spot)
*/

#include "tables.hpp"
#include "dictionary.hpp"
#include "feedback.hpp"
#include "utils.hpp"

#include <vector>

void EliminationsCounter::wordToData(const char *word, struct DataA *data){
	static struct DataLA letter_data[26];

	//initialize data to none
	for(int i=0; i<26; i++)
		letter_data[i].amount = letter_data[i].pos = 0;

	//mark all the positions of each letter
	for(int i=0; i<WORDLEN; i++){
		letter_data[word[i]-'a'].amount++;
		letter_data[word[i]-'a'].pos |= 1<<i;
	}

	//load all non-empty data into "data"
	data->letters = 0;
	struct DataLA *letter_data_ptr = data->letter_data;
	for(int i=0; i<26; i++){
		if(letter_data[i].amount != 0){
			data->letters |= 1<<i;
			*letter_data_ptr++ = letter_data[i];
		}
	}
}


//generate a list of DataS, where the positions correspond to a word
void genDataTable(const char *ans, const Dict &guesses, std::vector<Feedback> &table){
	for(int i=0; i < guesses.getLength(); i++)
		table.emplace_back(guesses.getWord(i), ans);
}

int EliminationsCounter::getEliminations(const Feedback &feedback){
	{
		std::lock_guard<std::mutex> lock(cache_mutex);
		if (cache.find(feedback) != cache.end())
			return cache[feedback];
	}

	int eliminations = countEliminations(feedback);

	{
		std::lock_guard<std::mutex> lock(cache_mutex);
		cache[feedback] = eliminations;
	}

	return eliminations;
}


EliminationsCounter::EliminationsCounter(const Dict &answers){
	answers_data = new struct DataA[answers.getLength()+1];
	for(int i=0; i<answers.getLength(); i++)
		wordToData(answers.getWord(i), &answers_data[i]);
	answers_data[answers.getLength()].letters = 0;
}

EliminationsCounter::~EliminationsCounter(){
	delete answers_data;
}

int EliminationsCounter::countEliminations(const Feedback &feedback){
	struct DataA *ans_data = answers_data;
	int eliminations = 0;
	while(ans_data->letters){
		if(!feedback.fits(*ans_data++))
			eliminations++;
	}
	return eliminations;
}
