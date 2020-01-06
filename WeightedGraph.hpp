#ifndef WEIGHTED_GRAPH_HPP
#define WEIGHTED_GRAPH_HPP

#include "Graph.hpp"

class WeightedGraph : public Graph
{
public:
	/**
	 * Move constructor
	 */
	WeightedGraph(WeightedGraph && other);
	/**
	 * Destructor
	 */
	virtual ~WeightedGraph() {free((void *) weights);}

	virtual size_t doWalk(uint node, uint32_t walk_len, FastPRNG & prng, char * const out) const;

	inline uint getWeight(uint const nodeId, uint const nodeTransition) const {return weights[nodes[nodeId] + (read_t) nodeTransition];}
	inline uint getNodeByWeight(uint const nodeId, uint const weight) const {uint index = nodes[nodeId]; while(weight >= weights[index]) ++index; return transitions[index];}

	virtual void print(FILE * out);
	virtual std::string toString() const;

protected:
	WeightedGraph(uint _nNodes, read_t const * _nodes, uint const * _trans, uint const * _weights);

	/**
	 * Stores weight for each transition
	 */
	uint const * weights;

public:
	static WeightedGraph readGraph(std::string input, uint64_t flags);
};

#endif
