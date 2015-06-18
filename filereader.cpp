#include "filereader.h"

FileReader::FileReader(const std::string &filename) : file(filename, std::ios_base::in | std::ios_base::binary)
{
}
