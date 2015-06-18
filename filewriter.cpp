#include "filewriter.h"

FileWriter::FileWriter(const std::string &filename)
    : file(filename, std::ios_base::out | std::ios_base::binary)
{
}

FileWriter::~FileWriter()
{
    file.close();
}
