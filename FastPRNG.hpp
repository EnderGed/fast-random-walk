#ifndef FAST_PRNG_HPP
#define FAST_PRNG_HPP

// External C++ Header files
#include <random>

// External C Header files
#include <ctime>

class FastPRNG
{
public:
	FastPRNG(std::mt19937::result_type seed = time(0)) : g1(seed), g2(g1()) {}

	template<typename T>
	T uniformInRange(T max);

private:
	// Create PRNG 1 (the slow one) with time as seed
	// PRNG version: Mersenne Twister 19937
	std::mt19937 g1;

	// Create PRNG 2 (the fast one) seeded with output of PRNG 1
	std::minstd_rand g2;
	
	unsigned prngResetCounter = 0;
};

template<typename T>
T FastPRNG::uniformInRange(T max)
{
	// If PRNG 2 was used 256 times ...
	if(++prngResetCounter > 0xffff)
	{
		prngResetCounter = 0;

		// Re-Seed PRNG 2 with output of PRNG 1
		g2.seed(g1());
	}

	return std::uniform_int_distribution<T>(0, max)(g2);
}

#endif
