#ifndef HELPER_FUNCTIONS_HPP
#define HELPER_FUNCTIONS_HPP

// Specify the integer type to use, here
#ifndef INT_TYPE
 #define INT_TYPE uint32_t
#endif

// Determine the max. string length needed to represent an integer (including \0-termination)
#if INT_TYPE == uint32_t
 #define INT_REP_LEN 11
 #define INT_READ_TYPE uint64_t
 #define INT_READ_MASK 0xffffffff
#elif INT_TYPE == uint16_t
 #define INT_REP_LEN 6
 #define INT_READ_TYPE uint32_t
 #define INT_READ_MASK 0xffff
#else
 #error Unknown integer type
#endif

// External C++ Header files
#include <vector>
#include <algorithm>
#include <string>

// External C Header files
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <cstddef>
#include <cmath>

typedef INT_TYPE uint;
typedef INT_READ_TYPE read_t;
typedef char const * CCP;

// Function declarations
/**
 * Counts the occurences of a character in a string
 * 
 * @param str Pointer to the first character of the string string to search in
 * @param strEnd Pointer the the first byte \b behind the string (e.g. the address of the termination '\0' byte)
 * @param c Character whose occurences should be counted
 */
read_t count(char const * str, char const * strEnd, char c) noexcept;

/**
 * Binary search algorithm
 * 
 * @param first Pointer to first array element
 * @param last Pointer to end of array (behind last element)
 * @param val Value to search for
 * 
 * @return Pointer to an occurence of \e val or any reference in [first, last] if \e val was not found
 */
template<typename ITER, typename T>
inline ITER * binaryFind(ITER * const first, ITER * const last, T const val) noexcept
{
	size_t const off = (last - first) >> 1;
	ITER * const mid = last - off;

	if(off < 8) return std::lower_bound(first, last, val);

	return (*mid > val) ? binaryFind(first, mid, val) : binaryFind(mid, last, val);
}

/**
 * Calculates the number of decimal digits needed to represent a number
 *
 * @param Number that should be representable
 *
 * @return Number of digits necessary to represent \e nr
 */
inline unsigned hasDigits(uint const nr)
{
	return (unsigned) ceil(log10(nr));
}

/**
 * Calculates the number of decimal digits needed to represent a number
 *
 * @param Number that should be representable
 *
 * @return Number of digits necessary to represent \e nr
 */
inline unsigned hasDigits(read_t const nr)
{
	return (unsigned) ceil(log10(nr));
}

/**
 * Prints an error string to \e stderr and exits execution with status code 1
 * 
 * @param str String to print to \e stderr
 */
__attribute__((noreturn))
void printError(char const * str);

/**
 * Allocates a char array of sufficient length, reads the whole file content into the array,
 * writes the length of the read content to \e fileSize and returns the array.
 * 
 * @param filename Path of the file to read from
 *
 * @return Complete content of the file
 */
std::string readFile(char const * fileName);

/**
 * Writes some specified content to a file
 *
 * @param fileName Name of the file to write the content to
 * @param content Content to write to the file
 */
void writeFile(char const * fileName, char const * content);

/**
 * Converts a string to an integer (mostly like strtoi and stoi)
 * 
 * @param str String to read the number from
 * 
 * @return Integer representation of the string
 */
uint32_t toInt(char const * str);

/**
 * Converts an integer to a string and writes it to \e target
 * 
 * @param target Pointer to string position where \e number should be written to
 * @param number Number to write to the \e target string
 * 
 * @return Number of characters written to \e target
 */
unsigned writeInt(char * target, uint number);

/**
 * A variant of count sort that expects that every value occures at most once
 * 
 * @param list Array that should be sorted
 * @param size Numer of elements in the \e list
 */
void boolSort(uint * list, size_t size);

/**
 * Sorting algorithm for 2-dimensional arrays with rows of the form
 * | data_0 | ... | data_i-1 | index | data_i+1 | data_i+2 | ...\n
 * Rows are moved as a whole.\n
 * The function allocates memory of size
 * <em>max(index + 1) * max(sizeof(C), sizeof(T*)) + size_x * size_y * sizeof(T)</em>
 * where <em>max(index + 1)</em> is the maximum of all values of the first column plus 1
 * and \e T is the type of the array elements. Thus high values in the first column
 * result in high memory consumption and probably out-of-memory exception.
 * 
 * @param arr Pointer to the beginning of the 2-dimensional array
 * @param size_x Size of the array in x-direction (columns of a row)
 * @param size_y Size of the array in y-direction (number of rows)
 * @param column Column id with the index that should be sorted
 */
void countSort2D(uint * arr, size_t size_x, size_t size_y, size_t column = 0);

/**
 * Sorting algorithm that counts the occurences of each value
 * 
 * @param list Array of elements to sort
 * @param size Number of elements in \e list
 */
void countSort(uint * list, size_t size);

//void sort2D(std::vector<uint *> lists, size_t rows);

/**
 * Sorting algorithm for 2-multiple lists where every list represents a column of a table/an array.
 * Rows are moved as a whole.\n
 * The function allocates memory of size
 * <em>max(index + 1) * max(sizeof(C), sizeof(size_t)) + rows * sizeof(T)</em>
 * where <em>max(index + 1)</em> is the maximum of all values of the first list plus 1
 * and \e T is the type of the list elements. Thus high values in the first column
 * result in high memory consumption and probably out-of-memory exception.
 * 
 * @param lists Pointers to the beginnings of the columns/lists
 * @param rows Number of rows
 */
void countSort2D(std::vector<uint *> lists, size_t rows);

#endif
