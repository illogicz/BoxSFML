// TODO: Add different cross over methods

#pragma once
#include <map>
#include <string>
#include "sGene.h"

using namespace std;

class sGenome
{

public:

	sGenome()
	{
		m_useMutationRateGene = false;
	}
	// Adds a gene with name and range, fills it with random data
	// If it exist already, it simply returns the existing gene
	sGene& addGene(string name, double min, double max, int bits = 16)
	{
		if(m_genes.find(name) == m_genes.end()){
			sGene &gene = m_genes[name]; 
			gene.set(float((max - min) * 0.5), float(min), float(max), bits);
			gene.random();
			return gene;
		} else {
			return m_genes[name];
		}
	}
	sGene &getGene(string name)
	{
		assert(m_genes.find(name) != m_genes.end());
		return m_genes[name];
	}

	float getValue(const string &name)
	{
		assert(m_genes.find(name) != m_genes.end());
		return m_genes[name].getValue();
	}

	void setValue(const string &name, float value)
	{
		assert(m_genes.find(name) != m_genes.end());
		m_genes[name].setValue(value);
	}

	// not used yet
	void setUseMutationRateGene(bool mutationGene, float lower = 0, float upper = 0)
	{
		m_useMutationRateGene = mutationGene;
		if(mutationGene){
			addGene("mutationRate", lower, upper);
		}
	}
	bool getUseMutationRateGene()
	{
		return m_useMutationRateGene;
	}


	// TODO: handle different structured genomes
	void clone(sGenome &genome)
	{
		for(map<string, sGene>::iterator i = m_genes.begin(); i != m_genes.end(); ++i){
			i->second.clone(genome.getGene(i->first));
		}
	}

	// TODO: handle different structured genomes
	void mate(sGenome &genome1, sGenome &genome2, float mutationRate = 0)
	{
		if(m_useMutationRateGene){
			mutationRate = genome1.getValue("mutationRate");
		}
		for(map<string, sGene>::iterator i = m_genes.begin(); i != m_genes.end(); ++i){
			i->second.mate(genome1.getGene(i->first), genome2.getGene(i->first), mutationRate);
		}
		// test: mutation rate decay
		if(m_useMutationRateGene){
			//setValue("mutationRate", getValue("mutationRate") * 0.99);
		}
	}

	void copyGene(string dst, string src, sGeneModifier mod = GENE_RANDOM)
	{
		m_genes[dst] = m_genes[src];
		if(mod == GENE_RANDOM){
			m_genes[dst].random();
		} else if(mod == GENE_INVERSE){
			m_genes[dst].invert();
		}
	}

	void randomize()
	{
		for(map<string, sGene>::iterator i = m_genes.begin(); i != m_genes.end(); ++i){
			i->second.random();
		}
	}

	int size()
	{
		return m_genes.size();
	}

	void printStats()
	{
		printf("\nGenome Contains:\n    %i genes\n    %i bits of data\n\n", m_genes.size(), bitSize());
		
	}

	int bitSize()
	{
		int bits = 0;
		for(map<string, sGene>::iterator i = m_genes.begin(); i != m_genes.end(); ++i){
		//	printf("%s = 	%f\n", i->first.c_str(), getValue(i->first));
			bits += i->second.getBitCount();
		}
		return bits;
	}

	map<string, sGene>& getGenes()
	{
		return m_genes;
	}

private:

	map<string, sGene> m_genes;
	bool m_useMutationRateGene;


};