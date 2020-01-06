#include "Graph.hpp"

// External C++ Header files
#include <vector>
#include <algorithm>

// External C Header files
#include <cstring>

#define BIT_MAP_GET(map, pos) (map[pos/64] & (1 << pos%64))
inline void bitMapSet(uint64_t * const map, size_t const pos, bool const val) {val ? (map[pos/64] |= ((uint64_t)1 << pos%64)) : (map[pos/64] &= ~((uint64_t)1 << pos%64));}

Graph::Graph(uint const _nNodes, read_t const * const _nodes, uint const * const _trans) : numNodes(_nNodes), nodes(_nodes), transitions(_trans)
{
}

Graph::Graph(Graph && other) : numNodes(other.numNodes), nodes(other.nodes), transitions(other.transitions)
{
	other.numNodes = 0;
	other.nodes = 0;
	other.transitions = 0;
}

size_t Graph::doWalk(uint node, uint32_t walk_len, FastPRNG & prng, char * const out) const
{
	// Write start node to output string
	char * outPos = out;

	{
		unsigned const wrote = writeInt(outPos, node);
		outPos += wrote;
	}

	while(--walk_len)
	{
		// Determine the number of outgoing transitions (from the current node)
		uint const outgoingTransitions = getNumTransitions(node);
		if(!outgoingTransitions)
		{
			fprintf(stderr, "Node %d has no outgoing transitions\n", node);
			break;
		}

		// Choose a successor node
		node = getNode(node, prng.uniformInRange<uint>(outgoingTransitions - 1));

		// Write node number to output
		{
			*(outPos++) = ',';
			unsigned const wrote = writeInt(outPos, node);
			outPos += wrote;
		}
	}

	*outPos = '\0';

	return outPos - out;
}

void Graph::print(FILE * const out)
{
	fprintf(out, "%s", toString().c_str());
}

std::string Graph::toString() const
{
	unsigned const maxDigits = hasDigits(getNumNodes() - 1);

	char asString[maxDigits];

	std::string res;
	res.reserve((maxDigits * 2 + 5) * getNumTransitions());

	for(uint node = 0; node < numNodes; ++node)
	{
		uint const numTrans = getNumTransitions(node);

		for(uint trans = 0; trans < numTrans; ++trans)
		{
			// Add source node
			unsigned len = writeInt(asString, node);
			res.append(asString, len);
			// Add " ->"
			res.append(" -> ", 4);
			// Add destination node
			len = writeInt(asString, getNode(node, trans));
			res.append(asString, len);
			res.append(1, '\n');
		}
	}

	return res;
}

unsigned Graph::nonDigits(char const * const start, char const * end)
{
	char const * pos = start;
	while(pos < end && (*pos < '0' || *pos > '9')) ++pos;

	return pos - start;
}

uint Graph::readInt(char const * const start, char const * const end, unsigned & readLen)
{
	char const * pos = start;
	read_t result = 0;

	// Skip space characters
	while(pos < end && *pos > 0 && *pos <= ' ') ++pos;

	// Ensure that the first non-space character is a digit
	if(pos >= end || *pos < '0' || *pos > '9') {fprintf(stderr, "Not a number: %c\n", *pos); exit(1);}

	// Read digit by digit until a non-digit occurs
	for(; pos < end; ++pos)
	{
		char const c = *pos;
		if(c < '0' || c > '9') break;

		result *= 10;
		result += c - '0';

		if(result > ((uint)-1)) printError("Specified integer type is too small to hold this number.");
	}

	readLen = pos - start;

	// Return the number
	return (uint) result;
}

Graph Graph::readGraph(std::string const input, uint64_t const flags)
{
	bool isSorted = true;
	read_t numTransitions;
	uint * data;

	////	Read from file	////
	{
		char const * fileContent = input.c_str();
		char const * fileContentEnd = fileContent + input.size();

		// Trim file content
		while(fileContent < fileContentEnd && *fileContent <= ' ') ++fileContent;
		while(fileContent < fileContentEnd && *(fileContentEnd - 1) <= ' ') --fileContentEnd;

		// Count rows
		numTransitions = count(fileContent, fileContentEnd, '\n') + 1;

		// Malloc data array (2 dimensions)
		data = (uint *) malloc(numTransitions * 2 * sizeof(*data));

		// File content to array
		char const * pos = fileContent;
		read_t prev = 0;
		unsigned readLen = 0;

		// For every row/transition ...
		for(read_t i = 0; i < numTransitions; ++i)
		{
			// Read source node
			uint const from = readInt(pos, fileContentEnd, readLen);

			// Go to next number
			pos += readLen;
			pos += nonDigits(pos, fileContentEnd);

			// Read target node
			uint const to = readInt(pos, fileContentEnd, readLen);

			// Go to next number/line (or end of file)
			pos += readLen;
			pos += nonDigits(pos, fileContentEnd);

			// Put into data array
			data[2 * i] = from;
			data[2 * i + 1] = to;

			if(from < prev) isSorted = false;

			prev = from;
		}
	}

	// Sort by source node if necessary
	if(!isSorted) countSort2D(data, 2, numTransitions);

	////	Render node starts	////
	uint const numNodes = data[2 * numTransitions - 2] + 1;
	read_t * const indices = (read_t *) malloc((numNodes + 1) * sizeof(*indices));
	
	// Create adjacency list
	uint node_id = 0;
	indices[0] = 0;

	for(read_t i = 0; i < numTransitions; ++i)
	{
		// Set target[node_id] for all nodes until the start node of the i-th transition
		while(data[2 * i] > node_id) indices[++node_id] = i;

		// Copy target node id to new (shrinked) position
		data[i] = data[2 * i + 1];
	}

	// Set target[node_id] to end of data-set for all remaining node_ids
	while(node_id < numNodes) indices[++node_id] = numTransitions;

	// Resize data to the shrinked size
	data = (uint *) realloc(data, numTransitions * sizeof(*data));

	// Convert the graph to an undirected graph
	if(flags & (flag_undirected | flag_reflect))
	{
		// Sort the transitions for each source node
		for(uint i = 0; i < numNodes; ++i)
			if(indices[i + 1] - indices[i] > 1)
				std::sort(data + indices[i], data + indices[i + 1]);

		////	Count number of missing transitions for every node	////
		uint * const counters = (uint *) calloc(numNodes, sizeof(*counters));
		std::vector<bool> bitSet(numTransitions * 2, false);
		read_t acc = 0; // Sum of all counters

		// For every source node ...
		for(uint node = 0; node < numNodes; ++node)
		{
			// For every transition of the node ...
			for(read_t trans = indices[node]; trans < indices[node + 1]; ++trans)
			{
				uint const targetNode = data[trans];
				bool const found = std::binary_search(data + indices[targetNode], data + indices[targetNode + 1], node);

				// If has no counterpart ...
				if(!found)
				{
					// Increment counter for target node
					++counters[targetNode];
					// Increment number of transitions
					++acc;
					// Set bit in bitSet
					//bitMapSet(bitSet, node, true);
					bitSet[trans] = true;
				}
			}
		}

		if(acc)
		{
			////	Adapt list	////
			// Resize transitions list (data)
			numTransitions += acc;
			data = (uint *) realloc(data, numTransitions * sizeof(*data));

			// For every node (from last to first) ...
			for(uint node = numNodes - 1; node > 0; --node)
			{
				// Get number of transitions
				size_t const len = indices[node + 1] - indices[node];
				// Increase end by accumulative counter
				indices[node + 1] += acc;
				// Subtract own new transitions from accumulative counter
				acc -= counters[node];
				// Move transitions to new position
				memmove(data + indices[node] + acc, data + indices[node], len * sizeof(*data));
				// Move bits in bitSet
				std::move_backward(bitSet.begin() + indices[node], bitSet.begin() + (indices[node] + len), bitSet.begin() + (indices[node] + len + acc));
				std::fill(bitSet.begin() + (indices[node] + acc - counters[node - 1]), bitSet.begin() + (indices[node] + acc), false);
			}

			// Modify last element (first node) (that is not part of the loop anymore)
			indices[1] += acc;

			////	Add new transitions	////
			// For every node ...
			for(uint node = 0; node < numNodes; ++node)
			{
				// For every transition of that node ...
				for(read_t trans = indices[node]; trans < indices[node + 1]; ++trans)
				{
					if(bitSet[trans])
					{
						uint const targetNode = data[trans];
						data[indices[targetNode + 1] - (counters[targetNode]--)] = node;
					}
				}
			}
		}
		else printf("Graph is already undirected\n");
		
		free(counters);
	}

	return Graph(numNodes, indices, data);
}
