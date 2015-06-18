#ifndef MATERIALS_H
#define MATERIALS_H

#include "vec.h"
#include "matrix.h"
#include "stripify.h"
#include "filereader.h"



#include <QString>

#include <unordered_map>

using namespace std;

class Material
{
public:
    Material();
    Vec4 specular;
    float shininess;
    QString texture_name;
    QString normal_texture_name;
    Vec4 diffuse;

unordered_map<string, Material> loadMTL(const QString &filename);



};

#endif // MATERIALS_H
