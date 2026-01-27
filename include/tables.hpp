#ifndef TABLES_H
#define TABLES_H

#include "dictionary.hpp"
#include "feedback.hpp"

#include <mutex>
#include <unordered_map>
#include <vector>

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))

#define CAPPED 010U
#define EMPTY (-1)

struct Feedback;

//data on a letter for DataA
struct DataLA {
	//amount includes flag for CAPPED
	unsigned char amount, pos;
};

//data of an answer
struct DataA {
	int letters;
	struct DataLA letter_data[WORDLEN];
};


void genDataTable(const char *ans, const Dict &guesses, std::vector<Feedback> &table);

class EliminationsCounter
{
private:
	struct DataA *answers_data;
	int countEliminations(const Feedback &feedback);
	void wordToData(const char *word, struct DataA *data);
	std::unordered_map<Feedback, int, FeedbackHasher> cache;
	std::mutex cache_mutex;
public:
	int getEliminations(const Feedback &feedback);
	EliminationsCounter(const Dict &answers);
	~EliminationsCounter();
};

#endif
