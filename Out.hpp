#ifndef OUT_HPP
#define OUT_HPP

// External C Headers
#include <cstdio>

// External C++ Headers
#include <mutex>


class Out
{
public:
	//TODO: Handle errors while opening/closing file
	Out(char const * fileName) : outFile(fopen(fileName, "w")) {}
	Out(Out const &) = delete;
	~Out() {if(outFile) fclose(outFile);}

	FILE * getFD() const {return outFile;}

	void write(char const * outString, size_t len)
	{
		std::unique_lock<std::mutex> lock(mut);
		fwrite(outString, sizeof(char), len, outFile);
	}

private:
	FILE * const outFile;
	std::mutex mut;
};

#endif
