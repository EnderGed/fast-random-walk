#ifndef GRAPH_HPP
#define GRAPH_HPP

// External C++ Header files

// External C Header files
#include <cstdio>

// Local Headers
#include "HelperFunctions.hpp"
#include "FastPRNG.hpp"

class Graph
{
public:
	Graph(Graph && other);
	virtual ~Graph() {free((void *) nodes); free((void *) transitions);}

	/**
	 * Performs a single walk
	 * 
	 * @param node The node from where to start the walk
	 * @param walk_len Number of nodes that the walk should pass (including start node)
	 * @param prng Reference to the PRNG to calculate random successor nodes
	 * @param out String to write the string representation of the walk to
	 */
	virtual size_t doWalk(uint node, uint32_t walk_len, FastPRNG & prng, char * out) const;

	inline uint getNumNodes() const {return numNodes;}
	inline read_t const * getNodes() const {return nodes;}
	inline uint getNode(uint const nodeId, uint const nodeTransition) const {return transitions[nodes[nodeId] + (read_t) nodeTransition];}
	inline read_t getNumTransitions() const {return nodes[numNodes];}
	inline uint getNumTransitions(uint const id) const {return id < numNodes ? nodes[id + 1] - nodes[id] : 0;}
	inline uint const * getTransitions() const {return transitions;}
	inline uint getTransition(read_t const id) const {return transitions[id];}

	/**
	 * Prints the graph
	 * 
	 * @param out Stream to write the output to
	 */
	virtual void print(FILE * out);
	/**
	 * Creates a string representation of the grap
	 *
	 * @return String representation of the grap
	 */
	virtual std::string toString() const;
protected:
	// Constructor used by \e readGraph
	Graph(uint _nNodes, read_t const * _nodes, uint const * _trans);

	uint numNodes;
	read_t const * nodes;
	uint const * transitions;

	/**
	 * Reads an integer (uint) from a C-String or char array
	 * 
	 * @param start Pointer to the beginning of the C-String or char array
	 * @param end Pointer to the end of (behind the) C-String or char array
	 * @param readLen Location where to place the number of characters that contained the integer
	 */
	static uint readInt(char const * pos, char const * end, unsigned & readLen);
	/**
	 * Counts the number of non-digit characters (ASCII)
	 * 
	 * @param start Pointer to the beginning of the C-String or char array
	 * @param end Pointer to the end of (behind the) C-String or char array
	 */
	static unsigned nonDigits(char const * pos, char const * end);

public:
	/**
	 * Flag to make the graph undirected
	 */
	static const uint64_t flag_undirected = 0x1;
	/**
	 * Flag to indicate that the order of transitions should be optimized
	 */
	static const uint64_t flag_optiTransOrder = 0x2;
	/**
	 * Flag to indicate that the order of transitions should be ordered by target node id
	 */
	//static const uint64_t flag_sortByTarget = 0x4;
	/**
	 * Flag to reflect all transitions
	 * (add the reverse of all transitions even for those that already have counterparts)
	 */
	static const uint64_t flag_reflect = 0x8;
	/*
	 * Reads a graph from an input string
	 * 
	 * @param input String to read the graph from
	 * @param flags Flags especially for enabling optimization strategies
	 */
	static Graph readGraph(std::string input, uint64_t flags);
};

#endif
