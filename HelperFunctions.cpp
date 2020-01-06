#include "HelperFunctions.hpp"

// External C++ Header files
#include <algorithm>

// External C Header files
#include <cstdio>
#include <cstring>


/**
 * Counts the number of occurences of character c
 */
read_t count(char const * const str, char const * const strEnd, char const c) noexcept
{
	read_t res = 0;

	for(register char const * pos = (CCP) memchr(str, (int) c, strEnd - str); pos != 0; pos = (CCP) memchr(pos + 1, (int) c, strEnd - pos - 1)) ++res;

	return res;
}

void printError(char const * const str)
{
	fprintf(stderr, "%s\n", str);
	exit(1);
}

std::string readFile(char const * const fileName)
{
	// Open file
	FILE * const file = fopen(fileName, "r");
	if(!file) printError("File could not be opened for reading");

	// Determine file size
	fseek(file, 0L, SEEK_END);
	size_t const fileSize = ftell(file);
	fseek(file, 0L, SEEK_SET);

	// Create buffer array
	char * const result = (char *) malloc(fileSize * sizeof(*result) + 1);

	// Read whole file
	size_t const read = fread(result, 1, fileSize, file);
	result[read] = '\n';

	// Error if file size and number of bytes read are different
	if(fileSize != read)
	{
		fprintf(stderr, "File size: %lu, but read: %lu\n", (unsigned long) fileSize, (unsigned long) read);
		exit(1);
	}

	// Close file
	fclose(file);

	std::string const res = std::string(result, fileSize);
	
	free(result);
	
	return res;
}

void writeFile(char const * const fileName, char const * const content)
{
	size_t const len = strlen(content);

	// Open file
	FILE * const file = fopen(fileName, "w");
	if(!file) printError("File could not be opened for writing");

	// Write to file
	size_t const w = fwrite(content, sizeof(*content), len, file);
	if(w < len) printError("Could not write full file content");

	// Close file
	fclose(file);
}

uint32_t toInt(char const * str)
{
	register uint64_t res = 0;

	// For every character in str ...
	for(; *str != '\0'; ++str)
	{
		// Check if character is a number
		if(*str < '0' || *str > '9') printError("Argument is not a number");

		// Adapt result according to character
		res *= 10;
		res += *str - '0';

		// Check for integer overflow
		if(res > 0xffffffff) printError("Number in argument is too large");
	}

	return (uint32_t) res;
}

unsigned writeInt(char * const target, uint number)
{
	register char * pos = target;

	// For every decimal digit of number (at least once)
	do
	{
		// Add digicimal digit to output
		*pos++ = ((char)(number % 10)) + '0';
		// Remove decimal digit from number
		number /= 10;
	}while(number);

	// Reverse output
	std::reverse(target, pos);

	// Return number of written characters
	return pos - target;
}

void countSort2D(uint * const arr, size_t const size_x, size_t const size_y, size_t column)
{
	// Prepare break condition
	uint * const arrEnd = arr + size_x * size_y;

	// Determine maximum index
	uint maxIndex = 0;
	for(uint * pos = arr + column; pos < arrEnd; pos += size_x)
		if(*pos > maxIndex) maxIndex = *pos;

	// Allocate adjacency list
	read_t * const adj = (read_t*)calloc((maxIndex + 2), sizeof(*adj));
	read_t * const counts = adj + 1;

	if(!adj) printError("Memory allocation failed");

	// Count occurences of each index
	for(uint * pos = arr + column; pos < arrEnd; pos += size_x) ++(counts[*pos]);

	// Convert occurency counts to real adjacency list
	read_t * const adj_end = adj + maxIndex + 1;
	adj[0] = 0;
	for(read_t * pos_adj = adj; pos_adj < adj_end; ++pos_adj) pos_adj[1] += pos_adj[0];

	// Copy arr
	uint * arrCpy = (uint*)malloc(size_x * size_y * sizeof(*arrCpy));
	if(!arrCpy) printError("Memory allocation failed");
	memcpy(arrCpy, arr, size_x * size_y * sizeof(*arrCpy));
	uint const * const arrCpyEnd = arrCpy + size_x * size_y;

	// Move rows back to arr but to the correct position
	for(uint const * cpyPos = arrCpy; cpyPos < arrCpyEnd; cpyPos += size_x)
	{
		for(size_t i = 0; i < size_x; ++i) arr[adj[cpyPos[column]] * size_x + i] = cpyPos[i];
		++adj[cpyPos[column]];
	}

	// Clean up
	free(arrCpy);
	free(adj);
}

/**
 * Searches the maximum value in a list
 *
 * @param list List of values
 * @param size Number of values in the list
 *
 * @return Maximum value in \e list
 */
uint maxValue(uint const * list, size_t const size) noexcept
{
	register uint res = 0;
	for(uint const * const listEnd = list + size; list < listEnd; ++list) if(*list > res) res = *list;

	return res;
}

/**
 * Helper function to create adjacency list
 */
read_t * prepareAdjList(uint const * const list, size_t const size, uint maxVal)
{
	// Allocate adjacency list
	read_t * const adj = (read_t*) calloc((maxVal + 2), sizeof(*adj));
	if(!adj) printError("Memory allocation failed");
	read_t * const counts = adj + 1;

	// Count occurences of each index
	for(register uint const * pos = list; pos < (list + size); ++pos) ++(counts[*pos]);

	// Convert occurency counts to real adjacency list
	read_t * const adjEnd = adj + maxVal + 1;
	adj[0] = 0;
	for(register read_t * adjPos = adj; adjPos < adjEnd; ++adjPos) adjPos[1] += adjPos[0];

	// Return adjacency list
	return adj;
}

void countSort(uint * const list, size_t const size)
{
	// Determine maximum list value
	uint const maxVal = maxValue(list, size);

	// Allocate counter list
	read_t * const counts = (read_t*) calloc((maxVal + 1), sizeof(*counts));
	if(!counts) printError("Memory allocation failed");

	// Count occurences of each index
	for(uint const * pos = list; pos < (list + size); ++pos) ++(counts[*pos]);

	// Write counted elements back to list
	register uint val = 0;
	for(register read_t pos = 0; pos < size; ++pos)
	{
		// Go to next count greater zero
		while(!counts[val]) ++val;
		
		// Add value to list
		list[pos] = val;
		--counts[val];
	}
	
	free(counts);
}

void boolSort(uint * const list, size_t const size)
{
	// Determine maximum list value
	uint const maxVal = maxValue(list, size);

	std::vector<bool> bitSet(maxVal + 1, false);

	// Set the corresponding bit in bitSet for every value in the list
	for(uint const * pos = list; pos < list + size; ++pos)
	{
		// If bit is already set ...
		if(bitSet[*pos])
		{
			// Print warning and call countSort as fallback
			printf("Value occures multiple times. Fallback tack countSort.\n");
			countSort(list, size);
			return;
		}
		else bitSet[*pos] = true;
	}

	// Write the found values back to the list
	register uint val = 0;
	for(register uint * pos = list; pos < list + size; ++pos)
	{
		while(!bitSet[val]) ++val;
		*pos = val++;
	}
}

void print(read_t const * const list, size_t const size)
{
	for(uint i = 0; i < size; ++i) printf("%u: %lu\n", i, list[i]);
}

class Iter2D final
{
public:
	class value_type final
	{
	public:
		value_type(std::vector<uint *> & _lists, read_t const _index) : lists(_lists), index(_index) {}

	private:
		std::vector<uint *> & lists;
		read_t index;
	};

	// Constructors
	Iter2D(std::vector<uint *> const _lists, read_t const _index) : lists(_lists), index(_index){}
	Iter2D(Iter2D const & other) = default;
	Iter2D(Iter2D && other) : lists(std::move(other.lists)), index(other.index) {}

	// ComparisonoOperators
	inline bool operator ==(Iter2D const & other){return index == other.index;}
	inline bool operator !=(Iter2D const & other){return index != other.index;}

	// Arithmetic opertors
	Iter2D operator +(read_t const offset) {return Iter2D(lists, index + offset);}
	Iter2D operator -(read_t const offset) {return Iter2D(lists, index - offset);}
	inline Iter2D & operator +=(read_t const offset) {index += offset; return *this;}
	inline Iter2D & operator -=(read_t const offset) {index -= offset; return *this;}

	// Member/Pointer operators
	Iter2D::value_type operator *() {return value_type(lists, index);}

	// Copy/Move operators
	Iter2D & operator =(Iter2D const & other) = default;
	Iter2D & operator =(Iter2D && other) {lists = std::move(other.lists); index = other.index; return *this;}
	
private:
	std::vector<uint *> lists;
	read_t index;
};

void countSort2D(std::vector<uint *> const lists, size_t const rows)
{
	// Determine maximum index
	uint const maxIndex = maxValue(lists[0], rows);

	// Allocate adjacency list
	read_t * const adj = prepareAdjList(lists[0], rows, maxIndex);

	uint * const arrCpy = (uint*)malloc(rows * sizeof(*arrCpy));
	if(!arrCpy) printError("Memory allocation failed");

	// Sort all lists (except the first one) with respect to values in first list
	for(size_t i = 1; i < lists.size(); ++i)
	{
		// Copy arr
		memcpy(arrCpy, lists[i], rows * sizeof(*arrCpy));

		// Move rows back to arr but to the correct position
		for(register size_t pos = 0; pos < rows; ++pos)
		{
			uint const targetRow = adj[lists[0][pos]];
			lists[i][targetRow] = arrCpy[pos];
			++adj[lists[0][pos]];
		}

		memmove(adj + 1, adj, (maxIndex + 1) * sizeof(*adj));
		adj[0] = 0;
	}

	// Sort the first list
	memcpy(arrCpy, lists[0], rows * sizeof(*arrCpy));

	for(register size_t pos = 0; pos < rows; ++pos)
	{
		uint const targetRow = adj[arrCpy[pos]];
		lists[0][targetRow] = arrCpy[pos];
		++adj[arrCpy[pos]];
	}

	// Clean up
	free(arrCpy);
	free(adj);
}

