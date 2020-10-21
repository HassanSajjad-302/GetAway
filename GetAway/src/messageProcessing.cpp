#include <istream>

void extract(std::istream& is, char *charArray, int size)
{
is.get(charArray, size);
charArray[size - 1] = '\0';
}