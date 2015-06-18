#ifndef MESH_H
#define MESH_H

#include <cstddef>
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <QString>

#include "vec.h"
#include "matrix.h"
#include "filereader.h"

#include "stripify.h"
#include "utils.h"

#include "_3ds.h"


using namespace std;

/*Структура материалов - есть проблемы с отображением прозрачности */
class QGLShaderProgram;

    struct Material
    {
        Vec3 ambient;
        Vec4 diffuse;
        Vec4 specular;
        float shininess;
        QString texture_name;
        QString normal_texture_name;
        float transparency;

        Material()
        {
            diffuse = Vec4(1.f,1.f,1.f,1.f);
            specular = Vec4(0.f,0.f,0.f,0.f);
            shininess = 64.f;
            texture_name.clear();
            normal_texture_name.clear();
            transparency = 1.0f;
        }
    };

class Mesh
{

friend class MATERIAL;
public:

    Mesh();
    ~Mesh();

    void clear();

    void draw();
    void load(QString &filename);

    unordered_map<string, Material> mtllib;
    std::vector<string> mat_names;

    void save(QString &filename) const;
     std::vector<string> material_names;
    std::pair<Vec3, Vec3> getBoundingBox() const;
 unordered_map<std::string, Material> loadMTL(QString &filename);

    void optimize();

    void swap(Mesh *mesh);
    void recompute_normals();

    QString path;
    uint32_t vtx_cnt;
    uint32_t face_cnt;
    uint16_t mat_cnt;

    std::string name;
    std::vector<Vec3> vertex;
    std::vector<Vec3> normal;
    std::vector<Vec3ub> color;
    std::vector<Vec2> tcoord;
    std::vector<uint32_t> index;
    uint32_t primitive_type;
    uint32_t texture_handle;
    uint32_t normal_texture_handle;
    std::vector<Mesh*> childs;

    Mat44 local;

    Vec3 ambient;
    Vec4 diffuse;
    Vec4 specular;
    float shininess;
    float transparency;


protected:


    void loadOBJ(QString &filename);
   void load3DS(QString &filename);

    uint32_t vbo_vtx;
    uint32_t vbo_normal;
    uint32_t vbo_color;
    uint32_t vbo_tcoord;
    uint32_t vbo_index;




private:
    static QGLShaderProgram *pShader;
};

#endif // MESH_H
