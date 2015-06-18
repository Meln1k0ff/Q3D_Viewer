#include "material.h"
#include  <unordered_map>
Material::Material()
{

        diffuse = Vec4(1.f,1.f,1.f,1.f);
        specular = Vec4(0.f,0.f,0.f,0.f);
        shininess = 64.f;
        texture_name.clear();
        normal_texture_name.clear();

}

unordered_map<string, Material> loadMTL(const QString &filename)
{
    unordered_map<string, Material> materials;

    FileReader file(filename.toStdString());
    if (!file.isOpen())
        return materials;

Material *cur = &(materials["default"]);
    string word;
    while(!file.atEnd())
    {
        file.skipWhitespaces();
        int c = file.peek();
        switch(c)
        {
        case '#':
            file.skipLine();
            continue;
        case 'K':
            file >> word;
            if (word == "Ka")
                file.skipLine();
            else if (word == "Kd")
            {
                file >> cur->diffuse[0]
                     >> cur->diffuse[1]
                     >> cur->diffuse[2];
            }
            else if (word == "Ks")
            {
                file >> cur->specular[0]
                     >> cur->specular[1]
                     >> cur->specular[2];
            }
            else
                file.skipLine();
            break;
        case 'n':
            file >> word;
            if (word == "newmtl")
            {
                file.skipWhitespaces();
                word = file.getLine();
                cur = &(materials[word]);
            }
            else
                file.skipLine();
            break;
        case 'N':
            file >> word;
            if  (word == "Ns")
            {
                file >> cur->shininess;
                cur->shininess = std::max(1.f, cur->shininess);
            }
            else
                file.skipLine();
            break;
        case 'm':
            file >> word;
            if (word == "map_Kd")
                cur->texture_name = QString(file.getLine().c_str()).trimmed();
            else if (word == "map_bump" || word == "map_Bump")
                cur->normal_texture_name = QString(file.getLine().c_str()).trimmed();
            else
                file.skipLine();
            break;
        default:
            file.skipLine();
        }
    }

    //using namespace  std;

    return materials;
}
