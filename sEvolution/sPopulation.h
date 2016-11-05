// TODO: Add some selection methods
// Determanistic

#pragma once
#include <vector>
#include <algorithm>
#include "sPhenotype.h"

struct sGeneration
{
	float bestFitness;
	float worstFitness;
	float averageFitness;
	sGenome bestGenome;
};

class sPopulation
{


public:

	sPopulation()
	{
		minBreaders = 2;
		m_generations = 0;
		m_selectionBias = 2.f;
		m_mutationRate = 0.01f;
		m_breadingPoolFraction = 1.0f;
		m_winnerPerPrelim = 0;
		m_elites = 0;
	}

	void addPhenotype(sPhenotype *phenotype)
	{
		m_phenotypes.push_back(phenotype);
		phenotype->setIsElite(false);
		breadingDistribution.push_back(0);
		breaders.resize(m_phenotypes.size());
	}


	// Sort based on fitness 
	void sortPhenotypes()
	{
		sort(m_phenotypes.begin(), m_phenotypes.end(), sortPhenotypesFunction);
	}

	// Generates a new generation of genome
	void newGeneration()
	{
		
		sortPhenotypes();
		
		int valid_breaders = 1;

		// Store data about last generation
		sGeneration generation;

		// Store best performer genome
		generation.bestGenome = m_phenotypes[0]->genome;
		generation.bestFitness = generation.averageFitness = m_phenotypes[0]->getFitness();
		generation.worstFitness = m_phenotypes[m_phenotypes.size() - 1]->getFitness();

		if(prelimsComplete())m_phenotypes[0]->setIsElite(m_elites > 0);
		for(unsigned int i = 1; i < m_phenotypes.size(); i++){
			if(prelimsComplete()){
				m_phenotypes[i]->setIsElite(m_elites > i);
			}
			if(m_phenotypes[i]->getFitness() != 0){
				valid_breaders++;
			}
			generation.averageFitness += m_phenotypes[i]->getFitness();
		}
		generation.averageFitness /= m_phenotypes.size();



		printf("\nGeneration %i\n", m_generations);
		printf("fitness best = %f average = %f\n", generation.bestFitness, generation.averageFitness);
		printf("uniformity = %f\n", generation.averageFitness / generation.bestFitness);
		//generation.bestGenome.print();

		// Copy breader genomes
		
		// Preliminary rounds, for when initial states contain mostly invalid states
		// Maintains better starting diversity
		if(!prelimsComplete()){

			for(int i = 0; i < m_winnerPerPrelim; i++){

				// Add winners to seeding pool
				m_prelimWinners.push_back(m_phenotypes[i]->genome);

				if(prelimsComplete()){

					printf("Prelims complete, starting..\n");
					for(unsigned int i = 0; i < m_phenotypes.size(); i++){
						m_phenotypes[i]->genome.clone(m_prelimWinners[i]);
					}
					return;

				}

			}

			printf("%i / %i phenotypes, generating new random population\n", m_prelimWinners.size(), m_phenotypes.size());

			for(unsigned int i = 0; i < m_phenotypes.size(); i++){
				m_phenotypes[i]->genome.randomize();
				m_phenotypes[i]->neuralNet.randomize();
			}


		} else {

			m_generationHistory.push_back(generation);

			int num_breaders = int(m_breadingPoolFraction * float(size()));
			if(valid_breaders < num_breaders){
				num_breaders = valid_breaders;
			}
			if(num_breaders < minBreaders)num_breaders = minBreaders;
			for(int i = 0; i < num_breaders; i++){
				//breaders.push_back(m_phenotypes[i]->genome);
				breaders[i] = m_phenotypes[i]->genome;
			}
		
		
			// Breading selection range, selection bias 1.0 - 3.0, default 2.0
			float n2 = float(pow(num_breaders, m_selectionBias));
		

			// Don't change the first ones, they become elites 
			for(unsigned int i = m_elites; i < m_phenotypes.size(); i++){

				// Choose 2 breaders, better performers have better chance, no cloning;

				int i1 = num_breaders - int(pow(getRand(n2), 1.f / m_selectionBias)) - 1;
				int i2 = num_breaders - int(pow(getRand(n2), 1.f / m_selectionBias)) - 1;
				while(i1 == i2){
					i2 = num_breaders - int(pow(getRand(n2), 1.f / m_selectionBias)) - 1;
				}
				breadingDistribution[i1]++;
				breadingDistribution[i2]++;
				m_phenotypes[i]->genome.mate(breaders[i1], breaders[i2], m_mutationRate);
			}

			//printBreadingDistribution();
		
			// Increment generation counter
			m_generations++;

		}
		if(m_phenotypes[0]->genome.getUseMutationRateGene()){
			float t = 0;
			for(unsigned int i = 0; i < m_phenotypes.size(); i++){
				t += m_phenotypes[i]->genome.getValue("mutationRate");
			}
			printf("average mutation rate = %f\n", t / m_phenotypes.size());
		}
	}



	void setElites(size_t elites)
	{
		if(elites < 0)elites = 0;
		if(elites > m_phenotypes.size())elites = m_phenotypes.size();
		m_elites = elites;
	}
	unsigned int getElites()
	{
		return m_elites;
	}

	void setSelectionBias(float selectionBias)
	{
		m_selectionBias = selectionBias;
	}

	void setMutationRate(float mutationRate)
	{
		if(mutationRate < 0.f)mutationRate = 0.f;
		if(mutationRate > 1.f)mutationRate = 1.f;
		m_mutationRate = mutationRate;
	}
	float getMutationRate()
	{
		return m_mutationRate;
	}	
	void increaseMutationRate()
	{
		if(m_mutationRate < 0.000001f){
			m_mutationRate = 0.000001f;
		}
		setMutationRate(m_mutationRate * 1.1f);
	}
	void decreaseMutationRate()
	{
		if(m_mutationRate < 0.000001f){
			m_mutationRate = 0.000000f;
		}
		setMutationRate(m_mutationRate / 1.1f);
	}

	void setBreadingPoolFraction(float breadingPoolFraction)
	{
		m_breadingPoolFraction = breadingPoolFraction;
	}

	int getGenerationCount()
	{
		return m_generationHistory.size();
	}
	sGeneration& getGeneration(int index)
	{
		return m_generationHistory[index];
	}

	void setWinnersPerPrelim(int winnerPerPrelim)
	{
		m_winnerPerPrelim = winnerPerPrelim;
	}

	int getPrelimWinnerCount()
	{
		return m_prelimWinners.size();
	}

	bool prelimsComplete()
	{
		return m_prelimWinners.size() == m_phenotypes.size() || m_winnerPerPrelim == 0;
	}

	size_t size()
	{
		return m_phenotypes.size();
	}

	sPhenotype* operator[] (int x) {
          return m_phenotypes[x];
    }
	
	void printStats()
	{
		int s = m_phenotypes.size();
		int bits = m_phenotypes[0]->genome.bitSize() * s;
		printf("\nPopulation Contains:\n    %i phenotypes\n    %i genes (%3.1f kb)\n",
			s, m_phenotypes[0]->genome.size() * s, float(bits) / 8000.f);
	}


	int minBreaders;

private:

	float getRand(float max)
	{
		return sRandom::getFloat(0,max);
	}

	vector<int> sel_dist;

	int m_generations;
	vector<sGeneration> m_generationHistory;

	static bool sortPhenotypesFunction(sPhenotype *lifeform1, sPhenotype *lifeform2)
	{
		return lifeform1->getFitness() > lifeform2->getFitness();
	}

	float m_mutationRate;
	float m_selectionBias;
	float m_breadingPoolFraction;
	unsigned int m_elites;
	int m_winnerPerPrelim;
	vector<sGenome> m_prelimWinners;


	vector<sGenome> breaders;
	vector<sPhenotype*> m_phenotypes;

	vector<int> breadingDistribution;
	void printBreadingDistribution()
	{
		for(size_t i = 0; i < size(); i++){
			printf("%i : %i\n", i, breadingDistribution[i]);
		}
	}


};



