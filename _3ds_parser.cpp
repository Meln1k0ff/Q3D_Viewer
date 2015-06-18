#include "_3ds.h"
#include <GL/glew.h>
#include "mesh.h"
#include <QFile>
#include <QFileInfo>
#include <QString>
#include "vec.h"
#include "matrix.h"
#include <QGLContext>
#include <QDebug>
#include <QHash>
#include <deque>
#include <iostream>


/*С 3ds возникают проблемы- не отгружаются материалы*/

struct CHUNK_DATA
{
    u_int16_t	ID;
    u_int32_t	length;
};
class MATERIAL
{
public:
    QString	NAME;
    Vec4	DIFFUSE;
    Vec4	SPECULAR;
    float	SHININESS;
    QString	MAPNAME;

    MATERIAL()
    {
        DIFFUSE = Vec4(1.f,1.f,1.f,1.f);
        SPECULAR = Vec4(0.f,0.f,0.f,0.f);
        NAME.clear();
        MAPNAME.clear();
    }
};

struct POSITION
{
    u_int16_t frame_number;
    u_int32_t unknown;
    float position_x, position_y, position_z;
} __attribute__((__packed__()));

struct ROTATION
{
    u_int16_t frame_number;
    u_int32_t unknown;
    float rotation;         /* in radians */
    float axis_x, axis_y, axis_z;
} __attribute__((__packed__()));

struct SCALE
{
    u_int16_t frame_number;
    u_int32_t unknown;
    float scale_x, scale_y, scale_z;
} __attribute__((__packed__()));

struct TAG_HEADER
{
    u_int16_t flags;
    u_int16_t unknown[4];
    u_int16_t nb_keys;
    u_int16_t unknown2;
} __attribute__((__packed__()));

struct ObjectTransform
{
    QString name;
    Vec3f scale;
    Vec3f pivot;
    Vec3f position;
    Vec4f attitude;
    int16_t id;

    ObjectTransform()
        : scale(1.f,1.f,1.f),
          pivot(0.f,0.f,0.f),
          position(0.f,0.f,0.f),
          attitude(0.f,0.f,0.f,0.f),
          id(-1)
    {
    }
};

class Parser3DS
{
public:
    Parser3DS(const QString &filename) : file(filename)
    {
        file.open(QIODevice::ReadOnly);
        gl = const_cast<QGLContext*>(QGLContext::currentContext());
        filepath = QFileInfo(filename).absolutePath() + '/';
        master_scale = 1.f;
    }

    inline CHUNK_DATA read_chunk_data()
    {
        CHUNK_DATA chunk;
        file.read((char*)&chunk.ID, 2);
        file.read((char*)&chunk.length, 4);
        return chunk;
    }

    inline CHUNK_DATA peek_chunk_data()
    {
        const qint64 pos = file.pos();
        const CHUNK_DATA &chunk = read_chunk_data();
        file.seek(pos);
        return chunk;
    }

    Mesh *operator()()
    {
        if (!file.isOpen())
        {
            qDebug() << "could not open file " << file.fileName();
            return NULL;
        }
        const CHUNK_DATA &chunk = peek_chunk_data();
        switch(chunk.ID)
        {
        case MAIN3DS:
            return read_MAIN3DS();
        default:
            qDebug() << "3DS: unknown first chunk";
            return NULL;
        }
    }

    void skipChunk()
    {
        const CHUNK_DATA &chunk = read_chunk_data();
        file.seek(file.pos() + chunk.length - 6);
    }

    Mesh *read_MAIN3DS()
    {
        const CHUNK_DATA &chunk = read_chunk_data();
        const qint64 end = file.pos() + chunk.length - 6;
        Mesh *mesh = NULL;
        std::vector<ObjectTransform> objdesc;
        while(file.pos() < end)
        {
            const CHUNK_DATA &next_chunk = peek_chunk_data();
            switch(next_chunk.ID)
            {
            case EDIT3DS:
                mesh = read_EDIT3DS();
                break;
            case KEYF3DS:
                qDebug() << "keyframes chunk found";
                objdesc = read_KEYF3DS();
                break;
            default:
                skipChunk();
            }
        }
        if (!mesh)
            qDebug() << "3DS: not data found";
        else if (!objdesc.empty())
        {
            QHash<QString, ObjectTransform> desc;
            for(size_t i = 0 ; i < objdesc.size() ;++i)
                desc[objdesc[i].name] = objdesc[i];

            for(size_t i = 0 ; i < mesh->childs.size() ; ++i)
            {
                const QString obj_name(mesh->childs[i]->name.c_str());
                QHash<QString, ObjectTransform>::const_iterator it = desc.find(obj_name);
                for(size_t l = 0 ; l < mesh->childs[i]->childs.size() ; ++l)
                {
                    Mesh *cur = mesh->childs[i]->childs[l];
                    Mat44 mat = Mat44::identity();
                    const ObjectTransform *trsf = (it == desc.end()) ? NULL : &(it.value());
                    bool b_skip = false;
                    if (it == desc.end() || trsf->pivot.sq() <= std::numeric_limits<float>::epsilon())
                    {
                        b_skip = true;
                    }
                    else
                    {
                        mat = Mat44::scale(trsf->scale) * mat;
                        mat = Mat44::rotation(-trsf->attitude[3], Vec3(trsf->attitude.x(), trsf->attitude.y(), trsf->attitude.z())) * mat;
                        mat = Mat44::translation(trsf->position) * mat;

                        mat = mat * Mat44::translation(-trsf->pivot) * cur->local.inverse();
                    }

                    std::deque<Mesh*> qwork;
                    qwork.push_back(cur);

                    while(!qwork.empty())
                    {
                        cur = qwork.front();
                        cur->local = Mat44::identity();
                        qwork.pop_front();
                        for(size_t j = 0 ; j < cur->childs.size() ; ++j)
                            qwork.push_back(cur->childs[j]);
                        if (!b_skip)
                            for(size_t j = 0 ; j < cur->vertex.size() ; ++j)
                            {
                                Vec3 &vtx = cur->vertex[j];
                                vtx = mat * vtx;
                            }
                    }
                }
            }
        }
        else
        {
            for(size_t i = 0 ; i < mesh->childs.size() ; ++i)
                for(size_t j = 0 ; j < mesh->childs[i]->childs.size() ; ++j)
                    mesh->childs[i]->childs[j]->local = Mat44::identity();
        }
        return mesh;
    }

    std::vector<ObjectTransform> read_KEYF3DS()
    {
        const CHUNK_DATA &chunk = read_chunk_data();
        const qint64 end = file.pos() + chunk.length - 6;
        std::vector<ObjectTransform> objdesc;
        while(file.pos() < end)
        {
            const CHUNK_DATA &next_chunk = peek_chunk_data();
            switch(next_chunk.ID)
            {
            case KEYF_OBJDES:
                objdesc.push_back(read_KEYF_OBJDES());
                break;
            case KEYF_FRAMES:
            case KEYF_UNKNWN01:
            case KEYF_UNKNWN02:
            default:
                skipChunk();
            }
        }
        return objdesc;
    }

    ObjectTransform read_KEYF_OBJDES()
    {
        const CHUNK_DATA &chunk = read_chunk_data();
        const qint64 end = file.pos() + chunk.length - 6;
        ObjectTransform ret;
        while(file.pos() < end)
        {
            const CHUNK_DATA &next_chunk = peek_chunk_data();
            switch(next_chunk.ID)
            {
            case KEYF_OBJHIERARCH:
                {
                    read_chunk_data();
                    const QString &name = read_ASCIIZ();
                    int16_t a, b, id;
                    file.read((char*)&a, sizeof(a));
                    file.read((char*)&b, sizeof(b));
                    file.read((char*)&id, sizeof(id));
                    ret.name = name;
                    ret.id = id;
                }
                break;
            case KEYF_OBJPIVOT:
                read_chunk_data();
                {
                    float x, y, z;
                    file.read((char*)&x, sizeof(x));
                    file.read((char*)&y, sizeof(y));
                    file.read((char*)&z, sizeof(z));
                    ret.pivot = Vec3f(x,y,z) * master_scale;
                }
                break;
            case KEYF_OBJPOS:
                {
                    read_chunk_data();
                    TAG_HEADER header;
                    POSITION key;
                    file.read((char*)&header, sizeof(header));
                    for(size_t i = 0 ; i < header.nb_keys ; ++i)
                    {
                        file.read((char*)&key, sizeof(key));
                        if (key.frame_number == 0)
                            ret.position = Vec3f(key.position_x, key.position_y, key.position_z);
                    }
                }
                break;
            case KEYF_OBJROT:
                {
                    read_chunk_data();
                    TAG_HEADER header;
                    ROTATION key;
                    file.read((char*)&header, sizeof(header));
                    for(size_t i = 0 ; i < header.nb_keys ; ++i)
                    {
                        file.read((char*)&key, sizeof(key));
                        if (key.frame_number == 0)
                            ret.attitude = Vec4f(key.axis_x, key.axis_y, key.axis_z, key.rotation);
                    }
                }
                break;
            case KEYF_OBJSCL:
                {
                    read_chunk_data();
                    TAG_HEADER header;
                    SCALE key;
                    file.read((char*)&header, sizeof(header));
                    for(size_t i = 0 ; i < header.nb_keys ; ++i)
                    {
                        file.read((char*)&key, sizeof(key));
                        if (key.frame_number == 0)
                            ret.scale = Vec3f(key.scale_x, key.scale_y, key.scale_z);
                    }
                }
                break;
            case KEYF_OBJDUMMYNAME:
            case KEYF_OBJBOUNDBOX:
            case KEYF_OBJMORPH_SMOOTH:
            default:
                skipChunk();
            }
        }
        return ret;
    }

    Mesh *read_EDIT3DS()
    {
        const CHUNK_DATA &chunk = read_chunk_data();
        const qint64 end = file.pos() + chunk.length - 6;
        Mesh *mesh = new Mesh;
        while(file.pos() < end)
        {
            const CHUNK_DATA &next_chunk = peek_chunk_data();
            switch(next_chunk.ID)
            {
            case EDIT_OBJECT:
                mesh->childs.push_back(read_EDIT_OBJECT());
                break;
            case EDIT_MATERIAL:
                {
                    const MATERIAL &mat = read_EDIT_MATERIAL();
                    materials[mat.NAME] = mat;
                }
                break;
            case EDIT_MASTER_SCALE:
                read_chunk_data();
                file.read((char*)&master_scale, sizeof(master_scale));
                break;
            default:
                skipChunk();
            }
        }
        return mesh;
    }

    Mesh *read_EDIT_OBJECT()
    {
        const CHUNK_DATA &chunk = read_chunk_data();
        const qint64 end = file.pos() + chunk.length - 6;
        Mesh *mesh = new Mesh;
        mesh->primitive_type = GL_TRIANGLES;
        mesh->name = read_ASCIIZ().toStdString();
        while(file.pos() < end)
        {
            const CHUNK_DATA &next_chunk = peek_chunk_data();
            switch(next_chunk.ID)
            {
            case OBJ_TRIMESH:
                mesh->childs.push_back(read_OBJ_TRIMESH());
                break;
            default:
                skipChunk();
            }
        }
        return mesh;
    }

    Mesh *read_OBJ_TRIMESH()
    {
        const CHUNK_DATA &chunk = read_chunk_data();
        const qint64 end = file.pos() + chunk.length - 6;
        Mesh *mesh = new Mesh;
        mesh->primitive_type = GL_TRIANGLES;
        Mat44 local_matrix = Mat44::identity();
        while(file.pos() < end)
        {
            const CHUNK_DATA &next_chunk = peek_chunk_data();
            switch(next_chunk.ID)
            {
            case TRI_VERTEXL:
                read_chunk_data();
                {
                    u_int16_t nb_vtx;
                    file.read((char*)&nb_vtx, 2);
                    const size_t end = mesh->vertex.size();
                    mesh->vertex.resize(end + nb_vtx);
                    file.read((char*)(mesh->vertex.data() + end), sizeof(Vec3) * nb_vtx);
                }
                break;
            case TRI_MATERIAL:
                read_chunk_data();
                {
                    const QString &material_name = read_ASCIIZ();

                    u_int16_t nb_faces;
                    file.read((char*)&nb_faces, 2);

                    Mesh *cur = mesh;
                    if (nb_faces < mesh->index.size() / 3)
                    {
                        cur = new Mesh;
                        cur->primitive_type = GL_TRIANGLES;
                        mesh->childs.push_back(cur);
                        cur->index.reserve(3 * nb_faces);
                    }
                    MATERIAL* mat = materials.contains(material_name) ? &(materials[material_name]) : NULL;

                    if (mat)
                    {
                        if (!mat->MAPNAME.isEmpty())
                        {
                            const QString &texture_filename = mat->MAPNAME.trimmed();
                            if (textures.contains(texture_filename))
                                cur->texture_handle = textures[texture_filename];
                            else
                            {
                                cur->texture_handle = gl->bindTexture(QImage(texture_filename), GL_TEXTURE_2D, GL_RGBA, QGLContext::LinearFilteringBindOption);
                                textures[texture_filename] = cur->texture_handle;
                            }
                        }
                        cur->diffuse = mat->DIFFUSE;
                        cur->specular = mat->SPECULAR;
                        cur->shininess = mat->SHININESS;
                    }
                    else
                        qDebug() << "WARNING: material not found!!";

                    for (int i = 0 ; i < nb_faces ; ++i)
                    {
                        u_int16_t cur_face;
                        file.read((char*)&cur_face, 2);
                        if (cur != mesh)
                        {
                            if (mesh->index[quint32(cur_face) * 3] == 0xFFFFFFFFU
                                    || mesh->index[quint32(cur_face) * 3 + 1] == 0xFFFFFFFFU
                                    || mesh->index[quint32(cur_face) * 3 + 2] == 0xFFFFFFFFU)
                                continue;
                            cur->index.push_back(mesh->index[quint32(cur_face) * 3]);
                            cur->index.push_back(mesh->index[quint32(cur_face) * 3 + 1]);
                            cur->index.push_back(mesh->index[quint32(cur_face) * 3 + 2]);
                            mesh->index[quint32(cur_face) * 3] = 0xFFFFFFFFU;
                            mesh->index[quint32(cur_face) * 3 + 1] = 0xFFFFFFFFU;
                            mesh->index[quint32(cur_face) * 3 + 2] = 0xFFFFFFFFU;
                        }
                    }
                }
                break;
            case TRI_MAPPING:
                read_chunk_data();
                {
                    u_int16_t nb_vtx;
                    file.read((char*)&nb_vtx, 2);
                    mesh->tcoord.resize(nb_vtx);
                    file.read((char*)mesh->tcoord.data(), sizeof(Vec2) * nb_vtx);
                }
                break;
            case TRI_FACEL1:
                read_chunk_data();
                {
                    u_int16_t nb_index;
                    file.read((char*)&nb_index, 2);
                    mesh->index.resize(nb_index * 3);
                    for(int i = 0 ; i < nb_index * 3 ; i += 3)
                    {
                        u_int16_t idx[3];
                        file.read((char*)idx, 2 * 3);
                        mesh->index[i] = idx[0];
                        mesh->index[i+1] = idx[1];
                        mesh->index[i+2] = idx[2];
                        u_int16_t face_info;
                        file.read((char*)&face_info, 2);
                    }
                }
                break;
            case TRI_LOCAL:
                read_chunk_data();
                {
                    Vec3 local[4];
                    file.read((char*)&(local[0]), sizeof(Vec3));		// X
                    file.read((char*)&(local[1]), sizeof(Vec3));		// Y
                    file.read((char*)&(local[2]), sizeof(Vec3));		// Z
                    file.read((char*)&(local[3]), sizeof(Vec3));		// local origin
                    local_matrix = Mat44::identity();
                    for(int i = 0 ; i < 4 ; ++i)
                        for(int j = 0 ; j < 3 ; ++j)
                            local_matrix(j,i) = local[i][j];
                }
                break;
            case TRI_VISIBLE:
                qDebug() << "TRI_VISIBLE";
                skipChunk();
                break;
            default:
                skipChunk();
            }
        }
        mesh->local = local_matrix;
        for(size_t i = 0 ; i < mesh->childs.size() ; ++i)
        {
            mesh->childs[i]->vertex = mesh->vertex;
            mesh->childs[i]->tcoord = mesh->tcoord;
        }
        size_t e = 0;
        for(size_t i = 0 ; i + e < mesh->index.size() ; )
        {
            const quint32 idx = mesh->index[i + e];
            if (idx >= mesh->vertex.size())
            {
                ++e;
                continue;
            }
            if (e > 0)
                mesh->index[i] = idx;
            ++i;
        }
        mesh->index.resize(mesh->index.size() - e);
        return mesh;
    }

    MATERIAL read_EDIT_MATERIAL()
    {
        const CHUNK_DATA &chunk = read_chunk_data();
        const qint64 end = file.pos() + chunk.length - 6;
        MATERIAL mat;
        while(file.pos() < end)
        {
            const CHUNK_DATA &next_chunk = peek_chunk_data();
            switch(next_chunk.ID)
            {
            case MAT_NAME:
                read_chunk_data();
                mat.NAME = read_ASCIIZ();
                break;
            case MAT_DIFFUSE:
                read_chunk_data();
                read_color_chunk(mat.DIFFUSE);
                break;
            case MAT_SPECULAR:
                read_chunk_data();
                read_color_chunk(mat.SPECULAR);
                break;
            case MAT_SHININESS:
                read_chunk_data();
                mat.SHININESS = std::max(1.f, read_percent_chunk());
                break;
            case MAT_TEXMAP:
                mat.MAPNAME = read_TEXMAP();
                break;
            default:
                skipChunk();
            }
        }
        return mat;
    }

    QString read_TEXMAP()
    {
        const CHUNK_DATA &chunk = read_chunk_data();
        const qint64 end = file.pos() + chunk.length - 6;
        QString filename;
        while(file.pos() < end)
        {
            const CHUNK_DATA &next_chunk = peek_chunk_data();
            switch(next_chunk.ID)
            {
            case MAT_MAPNAME:
                read_chunk_data();
                filename = read_ASCIIZ();
                break;
            default:
                skipChunk();
            }
        }
        return filename;
    }

    QString read_ASCIIZ()
    {
        char name[1024];
        int i = 0;
        unsigned char c = 0;
        while (file.getChar((char*)&c) && c != 0)
            name[i++] = c;
        name[i] = 0;
        return QString(name);
    }

    void read_color_chunk(Vec4 &color)
    {
        const CHUNK_DATA &chunk = read_chunk_data();
        switch(chunk.ID)
        {
        case COL_RGB:
        case COL_RGB2:
            file.read((char*)&color, sizeof(float) * 3);
            break;
        case COL_TRU:
        case COL_TRU2:
            for(int i = 0 ; i < 3 ; ++i)
            {
                unsigned char c;
                file.getChar((char*)&c);
                color[i] = c / 255.0f;
            }
            break;
        default:
            file.read(chunk.length - 6);
        };
    }

    float read_percent_chunk()
    {
        CHUNK_DATA chunk;
        file.read((char*)&chunk.ID, 2);
        file.read((char*)&chunk.length, 4);
        switch(chunk.ID)
        {
        case PER_INT:
            {
                u_int16_t percent;
                file.read((char*)&percent, 2);
                return percent;
            }
        case PER_FLOAT:
            {
                float percent;
                file.read((char*)&percent, 4);
                return percent;
            }
        default:
            file.read(chunk.length - 6);
        };
        return 0.0f;
    }

private:
    QFile file;
    QHash<QString, MATERIAL> materials;

    QGLContext * gl;
    QString filepath;
    QHash<QString, GLuint> textures;
    float master_scale;
};

void Mesh::load3DS(QString &filename)
{
    Mesh *mesh = Parser3DS(filename)();
    if (mesh)
    {
        swap(mesh);
        delete mesh;
        recompute_normals();
    }
    else
        qDebug() << "3DS: loading " << filename << " failed";
}




