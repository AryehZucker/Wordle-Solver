#include "dictionary.hpp"
#include "feedback.hpp"
#include "logging.hpp"
#include "tables.hpp"

#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

void calculateEliminations(const Dict &answers, const Dict &guesses, double *total_eliminations);

void calculateEliminationsForAnswer(const char *answer, const Dict &guesses, EliminationsCounter &eliminationsCounter, double *total_eliminations, Logger &logger);

const char *nextAnswer(const Dict &answers);

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

	std::vector<std::thread> threads;
	unsigned int num_threads = std::thread::hardware_concurrency();
	std::cout << "Using " << num_threads << " threads" << std::endl;
	for (unsigned int i = 0; i < num_threads; i++)
	{
		threads.emplace_back([&answers, &guesses, &eliminationsCounter, total_eliminations, &logger]{
			const char *answer;
			while ((answer = nextAnswer(answers)) != nullptr)
			{
				calculateEliminationsForAnswer(answer, guesses, eliminationsCounter, total_eliminations, logger);
			}
		});
	}

	for (auto &thread : threads)
	{
		thread.join();
	}

	std::cout << std::endl;
}

const char *nextAnswer(const Dict &answers)
{
	static std::mutex mutex;
	static int next = 0;

	int current;
	{
		std::lock_guard<std::mutex> lock(mutex);
		current = next++;
	}

	if (current >= answers.getLength())
	{
		return nullptr;
	}
	
	return answers.getWord(current);
}

void calculateEliminationsForAnswer(
	const char *answer,
	const Dict &guesses,
	EliminationsCounter &eliminationsCounter,
	double *total_eliminations,
	Logger &logger)
{
	static std::mutex total_eliminations_mutex;
	std::vector<Feedback> data_table;
	data_table.reserve(guesses.getLength());

	genDataTable(answer, guesses, data_table);
	for (int g1_index = 0; g1_index < guesses.getLength() - 1; g1_index++)
	{
		for (int g2_index = g1_index + 1; g2_index < guesses.getLength(); g2_index++)
		{
			int eliminations = eliminationsCounter.getEliminations(data_table[g1_index] + data_table[g2_index]);
			{
				std::lock_guard<std::mutex> lock(total_eliminations_mutex);
				total_eliminations[g1_index] += eliminations;
				total_eliminations[g2_index] += eliminations;
			}
			logger.logCompletedIteration();
		}
		logger.displayProgress();
	}
}
