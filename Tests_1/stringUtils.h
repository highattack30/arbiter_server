#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include "typeDefs.h"
#include <string>

bool stringStartsWith(std::string str, std::string with, bool ignoreWhiteSpaces = true);
std::string string_split_get_right(std::string, char);
std::vector<int> string_split_skill(std::string, char);


#endif
