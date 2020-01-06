#include "../WeightedGraph.hpp"
#include "HelpFuncs.hpp"

//#include <random>
//#include <ctime>
//#include <limits>
#include<cstdio>
#include<thread>

extern "C"
{
	//#include<sys/types.h>
	#include<sys/stat.h>
	#include<fcntl.h>
}

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE HelperFunctions
#include <boost/test/unit_test.hpp>

#define TEST_FILE_PREFIX "wALk_tESt_"
#define BUF_SIZE 128

using namespace std;

char const * const testFile = TEST_FILE_PREFIX "input_simple";

char * trim(char * from)
{
	while(*from <= ' ') ++from;
	
	char * to = from + strlen(from);
	
	if(to > from) --to;
	
	while(*to <= ' ') *(to--) = '\0';
	
	return from;
}

BOOST_AUTO_TEST_CASE(WGRAPH_SIMPLE)
{
	{
		WeightedGraph const g = WeightedGraph::readGraph("1,2,1", 0);
		string const str = g.toString();

		BOOST_CHECK_EQUAL(str, "1 -1-> 2\n");

		char walk[10];
		FastPRNG prng;
		g.doWalk(1, 2, prng, walk);

		BOOST_CHECK(!memcmp(walk, "1,2", 4));
	}

	{
		WeightedGraph const g = WeightedGraph::readGraph("1,2,1\n\n", 0);
		string const str = g.toString();

		BOOST_CHECK_EQUAL(str, "1 -1-> 2\n");

		char walk[10];
		FastPRNG prng;
		g.doWalk(1, 2, prng, walk);

		BOOST_CHECK(!memcmp(walk, "1,2", 4));
	}

	{
		WeightedGraph const g = WeightedGraph::readGraph("3,2,1\n0,1,3\n2,1,3\n1,3,2", 0);
		string const str = g.toString();
		
		BOOST_CHECK_EQUAL(str, "0 -3-> 1\n1 -2-> 3\n2 -3-> 1\n3 -1-> 2\n");

		char walk[20];
		FastPRNG prng;
		g.doWalk(0, 6, prng, walk);

		BOOST_CHECK(!memcmp(walk, "0,1,3,2,1,3", 12));
	}
}

BOOST_AUTO_TEST_CASE(WGRAPH_UNDIRECT)
{
	{
		WeightedGraph const g = WeightedGraph::readGraph("3,2,1\n0,1,3\n2,1,3\n1,3,2", Graph::flag_undirected);
		string const str = g.toString();

		BOOST_CHECK_EQUAL(str, "0 -3-> 1\n1 -2-> 3\n1 -3-> 0\n1 -3-> 2\n2 -3-> 1\n2 -1-> 3\n3 -1-> 2\n3 -2-> 1\n");

		char walk[10];
		FastPRNG prng;
		unsigned hit1 = 0;
		unsigned hit3 = 0;
		
		for(unsigned i = 0; i < 1000; ++i)
		{
			g.doWalk(2, 2, prng, walk);

			if(!memcmp(walk, "2,1", 4)) ++hit1;
			else if(!memcmp(walk, "2,3", 4)) ++hit3;
			else BOOST_CHECK(false);
		}
		
		// Check whether node 1 is hit about 3 of 4 times
		BOOST_WARN(hit1 >= 700 && hit1 <= 800);
		BOOST_CHECK(hit1 + hit3 == 1000);
	}

	{
		WeightedGraph const g = WeightedGraph::readGraph("1,2,2\n3,2,1\n2,3,3", Graph::flag_undirected);
		string const str = g.toString();

		BOOST_CHECK_EQUAL(str, "1 -2-> 2\n2 -3-> 3\n2 -2-> 1\n3 -1-> 2\n");
	}
}

BOOST_AUTO_TEST_CASE(WGRAPH_REFLECT)
{
	{
		WeightedGraph const g = WeightedGraph::readGraph("3,2,1\n0,1,3\n2,1,3\n1,3,2", Graph::flag_reflect);
		string const str = g.toString();

		BOOST_CHECK_EQUAL(str, "0 -3-> 1\n1 -2-> 3\n1 -3-> 0\n1 -3-> 2\n2 -3-> 1\n2 -1-> 3\n3 -1-> 2\n3 -2-> 1\n");
	}

	{
		WeightedGraph const g = WeightedGraph::readGraph("1,2,2\n3,2,1\n2,3,3", Graph::flag_reflect);
		string const str = g.toString();

		BOOST_CHECK_EQUAL(str, "1 -2-> 2\n2 -4-> 3\n2 -2-> 1\n3 -4-> 2\n");
	}
}

BOOST_AUTO_TEST_CASE(WGRAPH_OPTI_TRANS_ORDER_SIMPLE)
{
	{
		WeightedGraph const g = WeightedGraph::readGraph("1,2,1\n1,0,3", Graph::flag_optiTransOrder);
		string const str = g.toString();

		BOOST_CHECK_EQUAL(str, "1 -3-> 0\n1 -1-> 2\n");
	}
}

BOOST_AUTO_TEST_CASE(WGRAPH_MIX)
{
	{
		WeightedGraph const g = WeightedGraph::readGraph("1,2,2\n3,2,1\n2,3,3\n2,0,8", Graph::flag_undirected | Graph::flag_optiTransOrder);
		string const str = g.toString();

		BOOST_CHECK_EQUAL(str, "0 -8-> 2\n1 -2-> 2\n2 -8-> 0\n2 -3-> 3\n2 -2-> 1\n3 -1-> 2\n");
	}
}


