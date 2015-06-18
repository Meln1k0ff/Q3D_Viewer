#ifndef FILEWRITER_H
#define FILEWRITER_H

#include <fstream>
#include <string>
#include <cstring>
#include <cmath>
#include "utils.h"

class FileWriter
{
public:
    FileWriter(const std::string &filename);
    ~FileWriter();

    inline bool isOpen() const	{	return file.is_open();	}

    inline void write(const void * const data, const size_t len)
    {
        file.write((const char*)data, len);
    }

    template<typename T>
    inline void write(const T &v)
    {
        write(&v, sizeof(T));
    }

    inline void write(const char &c)
    {
        file.put(c);
    }

    inline void write(const std::string &str)
    {
        write(str.data(), str.size());
    }

    inline FileWriter &operator<<(const std::string &str)
    {
        write(str);
        return *this;
    }

    inline FileWriter &operator<<(const char *str)
    {
        write(str, strlen(str));
        return *this;
    }

    inline FileWriter &operator<<(char c)
    {
        write(c);
        return *this;
    }

    inline FileWriter &operator<<(int i)
    {
        if (i == 0)
        {
            write('0');
            return *this;
        }
        char buf[10];
        if (i < 0)
        {
            i = -i;
            write('-');
        }
        const int w = 1 + ilog10(i);
        for(int j = w - 1 ; j >= 0 ; --j)
        {
            const int o = i;
            i /= 10;
            buf[j] = '0' + (o - i * 10);
        }
        write(buf, w);
        return *this;
    }

    inline FileWriter &operator<<(unsigned int i)
    {
        if (i == 0)
        {
            write('0');
            return *this;
        }
        char buf[10];
        const int w = 1 + ilog10(i);
        for(int j = w - 1 ; j >= 0 ; --j)
        {
            const unsigned int o = i;
            i /= 10U;
            buf[j] = '0' + (o - i * 10U);
        }
        write(buf, w);
        return *this;
    }

    inline FileWriter &operator<<(float f)
    {
        if (f == 0.f)
            return *this << '0';
        if (f < 0.f)
        {
            f = -f;
            write('-');
        }
        const int e = ilog10d(f);
        unsigned int uf;
        if (e >= 0 && e < 6 && (uf = (unsigned int)f) == f)
            return *this << uf;
        double d = double(f) * ipow10d(-e);
        const int i = (int)d;
        write(char('0' + i));
        d -= i;
        if (d >= 1e-5)
        {
            write('.');
            for(size_t j = 0 ; j < 5 ; ++j)
            {
                d *= 10.0;
                const int id = (int)d;
                write(char('0' + id));
                d -= id;
            }
        }
        if (e != 0)
            *this << 'e' << e;
        return *this;
    }

private:
    std::fstream file;
};

#endif // FILEWRITER_H
