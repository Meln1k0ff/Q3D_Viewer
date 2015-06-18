#ifndef FILEREADER_H
#define FILEREADER_H

#include <fstream>
#include <string>
#include <cmath>
#include <math.h>
class FileReader
{
public:
    FileReader(const std::string &filename);

    inline bool isOpen() const	{	return file.is_open();	}
    inline void read(void * const ptr, const size_t len)	{	file.read((char*)ptr, len);	}
    inline bool atEnd() const	{	return !file;	}
    inline int get()	{	return file.get();	}
    inline int peek()	{	return file.peek();	}
    inline float pow10f(float n){return pow(10.0,n);}

    inline void skipWhitespaces()
    {
        for(int c = file.get() ; isspace(c) && c != -1 ; c = file.get())
        {}
        file.unget();
    }

    inline void skipLine()
    {
        for(int c = file.get() ; c != '\n' && c != -1 ; c = file.get())
        {}
        file.unget();
    }

    inline std::string getLine()
    {
        std::string line;
        for(int c = file.get() ; c != '\n' && c != -1 ; c = file.get())
            line += char(c);
        return line;
    }

    inline size_t getLine(char *buf, size_t len)
    {
        size_t pos(0);
        --len;
        for(int c = file.get() ; c != '\n' && c != -1 && pos < len ; c = file.get())
            buf[pos++] = char(c);
        buf[pos] = 0;
        return pos;
    }

    inline size_t getSplitLine(char *buf, size_t len)
    {
        size_t pos(0);
        --len;
        bool b_escape = false;
        for(int c = file.get() ; (c != '\n' || b_escape) && c != -1 && pos < len ; c = file.get())
        {
            if (c == '\r')
                continue;
            if (c == '\n')
                buf[pos - 1] = ' ';
            else
                buf[pos++] = char(c);
            b_escape = c == '\\';
        }
        buf[pos] = 0;
        return pos;
    }

    inline FileReader& operator>>(unsigned int &v)
    {
        int c;
        while(isspace(c = file.get()))	{}
        v = 0U;
        for(; isdigit(c) ; c = file.get())
            v = v * 10U + (c - '0');
        if (!isspace(c))
            file.unget();
        return *this;
    }

    inline FileReader& operator>>(int &v)
    {
        int c;
        while(isspace(c = file.get()))	{}
        const bool b_negative = c == '-';
        if (b_negative)
            c = file.get();
        v = 0;
        for(; isdigit(c) ; c = file.get())
            v = v * 10 + (c - '0');
        if (b_negative)
            v = -v;
        if (!isspace(c))
            file.unget();
        return *this;
    }

    inline FileReader& operator>>(std::string &s)
    {
        int c;
        while(isspace(c = file.get()))	{}
        s.clear();
        for(; !isspace(c) ; c = file.get())
            s += char(c);
        return *this;
    }

    inline FileReader& operator>>(float &v)
    {
        int c;
        while(isspace(c = file.get()))	{}
        const bool b_negative = c == '-';
        if (b_negative)
            c = file.get();
        v = .0f;
        for(; isdigit(c) ; c = file.get())
            v = v * 10.f + float(c - '0');
        if (c == '.' || c == ',')
        {
            c = file.get();
            for(float h = .1f; isdigit(c) ; c = file.get(), h *= .1f)
                v += float(c - '0') * h;
        }
        if (b_negative)
            v = -v;
        if (c == 'e' || c == 'E')
        {
            if (peek() == '+')
                get();
            int e(0);
            *this >> e;
            v *= pow10f(float(e));
        }
        else if (!isspace(c))
            file.unget();
        return *this;
    }

private:
    std::ifstream file;
};

#endif // FILEREADER_H
