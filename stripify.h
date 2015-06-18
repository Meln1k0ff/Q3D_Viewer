#ifndef STRIPIFY_H
#define STRIPIFY_H

#include <vector>

class Stripify
{
private:
    Stripify();

public:
    static void process(std::vector<unsigned int> &indices);
};

#endif // STRIPIFY_H
