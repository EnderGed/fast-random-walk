// External C Header files
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <memory>

// Local Header files
#include "WeightedGraph.hpp"
#include "Graph.hpp"
#include "Out.hpp"
#include "HelperFunctions.hpp"
#include "FastPRNG.hpp"

// Function Headers
void doWalk(Graph const & graph, uint from, uint to, uint32_t walk_len, uint32_t numWalks, Out &out);
void printHelp();
void printHelp(char const * str);
char const * readParam(int argc, char const * const * argv, char const * argName);

// SOURCE
int main(int const argc, char const * const argv[])
{
	uint64_t flags = 0;
	bool weighted = false;
	bool print = false;
	char const * input = 0;
	char const * output = 0;
	uint32_t walkLen = 0;
	uint32_t numWalks = 0;

	// Parse command line arguments
	for(int argi = 1; argi < argc; ++argi)
	{
		char const * arg = argv[argi];
		size_t len = strlen(arg);

		if(*arg == '-')
		{
			// Valid option
			++arg;
			--len;

			if(*arg == '-')
			{
				// Long option
				++arg;
				--len;

				char const * param = 0;
				int const remc = argc - argi;
				char const * const * const remv = argv + argi;

				if(!strcmp(arg, "optiTransOrder")) flags |= Graph::flag_optiTransOrder;
				else if(!strcmp(arg, "undirected")) flags |=  Graph::flag_undirected;
				else if(!strcmp(arg, "reflect")) flags |= Graph::flag_reflect;
				else if(!strcmp(arg, "weighted")) weighted = true;
				else if(!strcmp(arg, "print")) print = true;
				else if((param = readParam(remc, remv, "if"))) input = param;
				else if((param = readParam(remc, remv, "of"))) output = param;
				else if((param = readParam(remc, remv, "length"))) walkLen = toInt(param);
				else if((param = readParam(remc, remv, "walks"))) numWalks = toInt(param);
				else if(!strcmp(arg, "help"))
				{
					printHelp();
					exit(0);
				}
				else
				{
					printf("Unknown option --%s\n", arg);
					printHelp();
					exit(1);
				}
			}
			else
			{
				// Short option
				for(size_t i = 0; i < len; ++i)
				{
					switch(arg[i])
					{
						case 'o':
							flags |= Graph::flag_optiTransOrder;
							break;
						case 'p':
							print = true;
							break;
						case 'r':
							flags |= Graph::flag_reflect;
							break;
						case 'u':
							flags |= Graph::flag_undirected;
							break;
						case 'w':
							weighted = true;
							break;
						case 'h':
							printHelp();
							exit(0);
							break;
						default:
							printf("Unknown short option -%c\n", arg[i]);
							printHelp();
							exit(1);
					}
				}
			}
		}
		else
		{
			printf("Invalid option %s\n", arg);
			printHelp();
			exit(1);
		}
	}

	// Check whether all mandatory arguments are set
	if(!input) printHelp("No input file specified");
	else if(!output) printHelp("No output file specified");
	else if(!walkLen) printHelp("No walk length specified");
	else if(!numWalks) printHelp("Number of walks unspecified");

	//const uint64_t flags = Graph::flag_optiTransOrder | Graph::flag_undirected;
	std::string const fileContent(readFile(input));
	std::unique_ptr<Graph> graph(weighted ? new WeightedGraph(WeightedGraph::readGraph(fileContent, flags)) : new Graph(Graph::readGraph(fileContent, flags)));
	uint const to = graph->getNumNodes();

	if(print) printf("%s", graph->toString().c_str());

	Out out(output);
	doWalk(*graph, 0, to, walkLen, numWalks, out);

	return 0;
}

void doWalk(Graph const & graph, uint from, uint const to, uint32_t const walk_len, uint32_t const numWalks, Out &out)
{
	FastPRNG prng;

	// Malloc array for output string
	unsigned const maxDigits = hasDigits(graph.getNumNodes() - 1);
	char * const walk = (char *) malloc((maxDigits + 1) * sizeof(*walk) * walk_len);

	for(uint startNode = from; startNode < to; ++startNode) // For every assigned start node
	{
		// Ensure that the node really exists (has outgoing transitions)
		if(graph.getNumTransitions(startNode) == 0) continue;

		for(uint32_t i = 0; i < numWalks; ++i)
		{
			size_t out_len = graph.doWalk(startNode, walk_len, prng, walk);

			walk[out_len++] = '\n';

			// Write output string to file
			out.write(walk, out_len);
		}
	}

	free(walk);
}

void printHelp()
{
	printf("Usage:\n");
	printf("walk [options]\n\n");
	printf("Options (mandatory):\n");
	printf("--if <input file>\n");
	printf("--of <output file>\n");
	printf("--length <walk length>\n");
	printf("--walks <number of walks>\n");
	printf("\n");
	printf("Options (optional):\n");
	printf("-o, --optiTransOrder: Optimize order of transitions\n");
	printf("-p, --print: Print graph\n");
	printf("-r, --reflect: Reflects all transitions (even if counterpart exists)\n");
	printf("-u, --undirected: Converts the graph to an undirected graph\n");
	printf("-w, --weighted: Runs walks on weighted graph\n");
}

void printHelp(char const * const str)
{
	printf("%s\n\n", str);
	printHelp();
	exit(1);
}

char const * readParam(int argc, char const * const * const argv, char const * argName)
{
	size_t const argLen = strlen(argName);

	//    --argName <value>
	// or --argName=<value>
	if(!strcmp(argv[0] + 2, argName))
	{
		if(argc > 1) return argv[1];
		else
		{
			printf("Missing argument for option %s\n\n", argName);
			printHelp();
			exit(1);
		}
	}
	else if(!strncmp(argv[0] + 2, argName, argLen) && argv[0][argLen + 2] == '=') return argv[0] + argLen + 3;

	// If argument not found ... return NULL
	return 0;
}

