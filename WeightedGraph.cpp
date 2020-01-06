#include "WeightedGraph.hpp"

// External C++ Header files
#include <tuple>
#include <algorithm>

// External C Header files
#include <cstring>

using namespace std;

WeightedGraph::WeightedGraph(uint const _nNodes, read_t const * const _nodes, uint const * const _trans, uint const * const _weights) : Graph(_nNodes, _nodes, _trans), weights(_weights)
{
}

WeightedGraph::WeightedGraph(WeightedGraph && other) : Graph(other.numNodes, other.nodes, other.transitions), weights(other.weights)
{
	other.numNodes = 0;
	other.nodes = 0;
	other.transitions = 0;
	other.weights = 0;
}

size_t WeightedGraph::doWalk(uint node, uint32_t walk_len, FastPRNG & prng, char * const out) const
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
		uint const accWeight = weights[nodes[node + 1] - 1];
		node = getNodeByWeight(node, prng.uniformInRange<uint>(accWeight - 1));

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

void WeightedGraph::print(FILE * const out)
{
	fprintf(out, "%s", toString().c_str());
}

std::string WeightedGraph::toString() const
{
	unsigned const maxDigitsNode = hasDigits(getNumNodes() - 1);
	unsigned const maxDigitsWeight = hasDigits(std::numeric_limits<uint>::max());

	char asString[max(maxDigitsNode, maxDigitsWeight)];

	std::string res;
	res.reserve((maxDigitsNode * 2 + maxDigitsWeight + 6) * getNumTransitions());

	for(uint node = 0; node < numNodes; ++node)
	{
		uint const numTrans = getNumTransitions(node);

		for(uint trans = 0; trans < numTrans; ++trans)
		{
			// Determine weight
			uint const weight = trans ? getWeight(node, trans) - getWeight(node, trans - 1) : getWeight(node, trans);

			// Add source node
			unsigned len = writeInt(asString, node);
			res.append(asString, len);

			// Add " -"
			res.append(" -", 2);

			// Add weight
			len = writeInt(asString, weight);
			res.append(asString, len);

			// Add "-> "
			res.append("-> ", 3);

			// Add destination node
			len = writeInt(asString, getNode(node, trans));
			res.append(asString, len);

			// Add new line symbol
			res.append(1, '\n');
		}
	}

	return res;
}

WeightedGraph WeightedGraph::readGraph(std::string const input, uint64_t const flags)
{
	bool isSorted = true;
	read_t numRows;
	uint * data;

	////	Read from file	////
	{
		char const * fileContent = input.c_str();
		char const * fileContentEnd = fileContent + input.size();

		// Trim file content
		while(fileContent < fileContentEnd && *fileContent <= ' ') ++fileContent;
		while(fileContent < fileContentEnd && *(fileContentEnd - 1) <= ' ') --fileContentEnd;

		// Count rows
		numRows = count(fileContent, fileContentEnd, '\n') + 1;

		// Malloc data array (3 dimensions)
		data = (uint *) malloc(numRows * 3 * sizeof(*data));

		// File content to array
		char const * pos = fileContent;
		read_t prev = 0;
		unsigned readLen = 0;

		// For every row/transition ...
		for(read_t i = 0; i < numRows; ++i)
		{
			// Read source node
			uint const from = readInt(pos, fileContentEnd, readLen);

			// Go to next number
			pos += readLen;
			pos += nonDigits(pos, fileContentEnd);

			// Read target node
			uint const to = readInt(pos, fileContentEnd, readLen);

			// Go to next number
			pos += readLen;
			pos += nonDigits(pos, fileContentEnd);

			// Read weight
			uint const weight = readInt(pos, fileContentEnd, readLen);

			// Go to next number/line (or end of file)
			pos += readLen;
			pos += nonDigits(pos, fileContentEnd);

			// Put into data array
			data[3 * i] = from;
			data[3 * i + 1] = to;
			data[3 * i + 2] = weight;

			if(from < prev) isSorted = false;

			prev = from;
		}
	}

	// Sort if necessary
	if(!isSorted) countSort2D(data, 3, numRows);

	////		Render node starts	////
	uint * weights = (uint*)malloc(numRows * sizeof(*weights));
	uint const numIndices = data[3 * numRows - 3] + 1;
	read_t * const indices = (read_t *) malloc((numIndices + 1) * sizeof(*indices));
	
	// Create adjacency list
	uint curr = 0;
	indices[0] = 0;

	for(read_t i = 0; i < numRows; ++i)
	{
		// Set target[curr] for all nodes until the start node of the i-th transition
		while(data[3 * i] > curr) indices[++curr] = i;

		// Copy target node id to new (shrinked) position
		data[i] = data[3 * i + 1];
		// Copy the weight of the transition to weights
		weights[i] = data[3 * i + 2];
	}

	// Set target[curr] to end of data-set for all remaining indices
	while(curr < numIndices) indices[++curr] = numRows;

	// Shrink the size of data to its real content
	data = (uint *) realloc(data, numRows * sizeof(*data));

	////		Convert the graph to an undirected graph	////
	if(flags & (flag_undirected | flag_reflect))
	{
		// Sort the transitions for each source node
		// (Target nodes in increasing order)
		for(uint i = 0; i < numIndices; ++i)
		{
			read_t const start = indices[i];
			read_t const count = indices[i + 1] - start;

			if(count > 1) countSort2D({data + start, weights + start}, count);
		}

		///		Count number of missing transitions for every node		///
		uint * const counters = (uint *) calloc(numIndices, sizeof(*counters));
		std::vector<bool> bitSet(numRows * 2, false);
		read_t acc = 0; // Sum of all counters

		// For every source node ...
		for(uint node = 0; node < numIndices; ++node)
		{
			// For every transition of the node ...
			for(read_t trans = indices[node]; trans < indices[node + 1]; ++trans)
			{
				uint const targetNode = data[trans];
				uint const * const from = data + indices[targetNode];
				uint const * const to = data + indices[targetNode + 1];
				uint const * const found = binaryFind(from, to, node);

				// If has no counterpart ...
				if(found == to || *found != node)
				{
					// Increment counter for target node
					++counters[targetNode];
					// Increment number of transitions
					++acc;
					// Set bit in bitSet
					bitSet[trans] = true;
				}
				else if(flags & flag_reflect)
				{
					read_t const other_trans = found - data;
					if(other_trans > trans) weights[trans] = weights[other_trans] = weights[other_trans] + weights[trans];
				}
			}
		}

		if(acc)
		{
			////	Adapt list (make it undirected)		////
			// Resize transitions lists (data and weights)
			numRows += acc;
			data = (uint *) realloc(data, numRows * sizeof(*data));
			weights = (uint *) realloc(weights, numRows * sizeof(*weights));

			// For every node (from last to first)
			for(uint node = numIndices - 1; node > 0; --node)
			{
				// Get number of transitions
				size_t const len = indices[node + 1] - indices[node];
				// Increase end by accumulative counter
				indices[node + 1] += acc;
				// Subtract own new transitions from accumulative counter
				acc -= counters[node];
				//Move transitions to new position
				memmove(data + indices[node] + acc, data + indices[node], len * sizeof(*data));
				memmove(weights + indices[node] + acc, weights + indices[node], len * sizeof(*weights));
				// Move bits in bitSet
				std::move_backward(bitSet.begin() + indices[node], bitSet.begin() + (indices[node] + len), bitSet.begin() + (indices[node] + len + acc));
				std::fill(bitSet.begin() + (indices[node] + acc - counters[node - 1]), bitSet.begin() + (indices[node] + acc), false);
			}

			// Node 0
			indices[1] += acc;

			////	Add new transitions		////
			// For every node ...
			for(uint node = 0; node < numIndices; ++node)
			{
				// For every transition of that node ...
				for(read_t trans = indices[node]; trans < indices[node + 1]; ++trans)
				{
					if(bitSet[trans])
					{
						uint const targetNode = data[trans];
						uint const weight = weights[trans];
						data[indices[targetNode + 1] - (counters[targetNode])] = node;
						weights[indices[targetNode + 1] - (counters[targetNode])] = weight;

						--counters[targetNode];
					}
				}
			}
		}
		else if(!(flags & flag_reflect)) printf("Graph is already undirected\n");

		free(counters);
	}

	////		Optimize transition order	////
	if((flags & flag_optiTransOrder) && numIndices > 0)
	{
		// Sort each node
		for(uint nodeId = 0; nodeId < numIndices; ++nodeId)
		{
			// Determine sort range
			read_t const start = indices[nodeId];
			read_t const count = indices[nodeId + 1] - start;

			// Sort node (lowest weight first)
			countSort2D({weights + start, data + start}, count);

			// Reverse order (highest weight first)
			std::reverse(data + start, data + start + count);
			std::reverse(weights + start, weights + start + count);
		}
	}

	////		Post-Processing		////
	bool errorFound = false;
	// Accumulate weights
	for(read_t const * posIndices = indices, * indicesEnd = indices + numIndices; posIndices < indicesEnd; ++posIndices)
	{ // For each node ...
		uint newWeight = weights[*posIndices];
	
		// Accumulate weights of this nodes transitions
		for(read_t posWeights = *posIndices + 1; posWeights < *(posIndices + 1); ++posWeights)
		{
			//if(__builtin_add_overflow(weights[posWeights], weights[posWeights - 1], &weights[posWeights])) std::cout << "!!!!!Problem!!!!!" << std::endl;
			newWeight = weights[posWeights] + weights[posWeights - 1];
			if(newWeight < weights[posWeights] || newWeight < weights[posWeights - 1])
			{
				printf("Integer Overflow!!!");
				exit(1);
			}
			weights[posWeights] = newWeight;
		}

		// Check whether sum of weights is 0
		if(newWeight == 0)
		{
			printf("Node %lu has an accumulative weight of 0!\n", posIndices - indices);
			errorFound = true;
		}
	}
	
	if(errorFound) exit(1);

	return WeightedGraph(numIndices, indices, data, weights);
}
