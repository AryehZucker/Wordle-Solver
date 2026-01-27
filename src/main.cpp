#include "dictionary.hpp"
#include "feedback.hpp"
#include "logging.hpp"
#include "tables.hpp"

#include <iostream>
#include <vector>

void calculateEliminations(const Dict &answers, const Dict &guesses, double *total_eliminations);

void calculateEliminationsForAnswer(const char *answer, const Dict &guesses, EliminationsCounter &eliminationsCounter, double *total_eliminations, Logger &logger);

int main(int argc, char *argv[]){
	double *total_eliminations;
	int best_words[100];
	double max_eliminations;

	const Dict answers(argv[1]);
	const Dict guesses(argv[2]);
	if(guesses.getLength() < 0 || answers.getLength() < 0){
		std::cerr << "Error loading word lists" << std::endl;
		return 1;
	}

	//allot space to store the total eliminations and initialize to zero
	total_eliminations = new double[guesses.getLength()]();

	std::cout << "Calculating eliminations..." << std::endl;
	calculateEliminations(answers, guesses, total_eliminations);
	std::cout << "Done" << std::endl;

	std::cout << "Finding best words...";
	max_eliminations = 0;
	for(int i=0; i<guesses.getLength(); i++)
		if(total_eliminations[i] > max_eliminations)
			max_eliminations = total_eliminations[i];

	unsigned int j = 0;
	for(int i=0; i<guesses.getLength() && j<(sizeof(best_words)/sizeof(int)); i++)
		if(total_eliminations[i] == max_eliminations)
			best_words[j++] = i;
	best_words[j] = -1; //mark end of list
	std::cout << "Done" << std::endl;

	//make the stored totals into averages
	for(int i=0; i<guesses.getLength(); i++)
		total_eliminations[i] /= answers.getLength() * (guesses.getLength()-1);

	std::cout << std::endl << "Best words:" << std::endl;
	for(int i=0; best_words[i] >= 0; i++){
		std::cout << guesses.getWord(best_words[i]) << std::endl;
	}

	delete[] total_eliminations;

	return 0;
}




void calculateEliminations(const Dict &answers, const Dict &guesses, double *total_eliminations){
	EliminationsCounter eliminationsCounter(answers);

	std::cout << "Beginning combinatorial calculations..." << std::endl;
	Logger logger(answers, guesses);
	for(int ans_index = 0; ans_index < answers.getLength(); ans_index++){
		const char *answer = answers.getWord(ans_index);
        calculateEliminationsForAnswer(answer, guesses, eliminationsCounter, total_eliminations, logger);
    }

	std::cout << std::endl;
}

void calculateEliminationsForAnswer(
	const char *answer,
	const Dict &guesses,
	EliminationsCounter &eliminationsCounter,
	double *total_eliminations,
	Logger &logger)
{
	std::vector<Feedback> data_table;
	data_table.reserve(guesses.getLength());

	genDataTable(answer, guesses, data_table);
	for (int g1_index = 0; g1_index < guesses.getLength() - 1; g1_index++)
	{
		for (int g2_index = g1_index + 1; g2_index < guesses.getLength(); g2_index++)
		{
			int eliminations = eliminationsCounter.getEliminations(data_table[g1_index] + data_table[g2_index]);
			total_eliminations[g1_index] += eliminations;
			total_eliminations[g2_index] += eliminations;
			logger.logCompletedIteration();
		}
		logger.displayProgress();
	}
}
