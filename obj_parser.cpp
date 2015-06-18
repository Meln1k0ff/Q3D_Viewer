
#include <unordered_map>

#include <QGLContext>
#include <QFileInfo>
#include <QDebug>
#include <QHash>

#include "mesh.h"

namespace std
{
    template<typename T, typename U>
    struct hash<pair<T,U> >
    {
    public:
        inline size_t operator()(const pair<T,U> &v) const
        {
            return (h0(v.first) * 31U) ^ h1(v.second);
        }
    private:
        std::hash<T> h0;
        std::hash<U> h1;
    };
}

unordered_map <std::string, Material> Mesh::loadMTL(QString &filename)
{
    qDebug() << "LoadMTL start\n";
    unordered_map<std::string, Material> materials;

        FileReader file(filename.toStdString());
        if (!file.isOpen())
            return materials;

        string word;
        Material *cur;
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
                    //file.skipLine();
                    file >> cur->ambient[0]
                         >>   cur->ambient[1]
                         >> cur->ambient[2];

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
            case 'T':
                    file >> word;
                    if (word == "Tr")
                    file >> cur->transparency;
                break;
            case 'd':
                file >> word;
                if (word == "d")
                file >> cur->transparency;
            case 'n':

                file >> word;
                if (word == "newmtl")
                {
                    file.skipWhitespaces();
                    word = file.getLine();
                    cur = &(materials[word]);
                    mat_names.push_back(word);
                }
                else
                    file.skipLine();
                break;
            case 'N':
                file >> word;
                if (word == "Ns")
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

        return materials;
    }

void Mesh::loadOBJ(QString &filename)
{
    FileReader file(filename.toStdString());

    uint32_t vtx_cnt;
    uint32_t face_cnt;

    qDebug() << file.isOpen();

    if (!file.isOpen())
        return;

     QString path = QFileInfo(filename).absolutePath() + '/';
    this->path = path;

    QGLContext *gl = const_cast<QGLContext*>(QGLContext::currentContext());

    primitive_type = GL_TRIANGLES;

    Mesh *cur = this;

    vector<Vec3> t_vtx;
    vector<Vec2> t_tcd;
    vector<char*> elts;
    vector<char*> components;
    unordered_map<pair<int, int>, size_t> elts_table;
    unordered_map<string, Material> mtllib;
    QHash<QString, GLuint> textures;
    Material *mtl = &(mtllib["usemtl"]);

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
        case 'u':
        case 'g':
        case 'o':
            file >> word;
            if (word == "o" || word == "g" || word == "usemtl")
            {
                if (word == "usemtl")
                {
                    file.skipWhitespaces();
                    word = file.getLine();
                    const Material *old_mtl = mtl;
                    if (mtllib.count(word))
                        mtl = &(mtllib[word]);
                    else
                        mtl = &(mtllib["default"]);
                    if (old_mtl == mtl)
                        continue;
                    word = cur->name + '_' + word;
                }
                else
                    word = file.getLine();
                childs.push_back(new Mesh);
                cur = childs.back();
                cur->name = word;
                cur->primitive_type = GL_TRIANGLES;
                elts_table.clear();
                cur->ambient = mtl->ambient;
                cur->diffuse = mtl->diffuse;
                cur->specular = mtl->specular;
                cur->shininess = mtl->shininess;
                cur->transparency = mtl->transparency;
                if (mtl->texture_name.isEmpty())
                    cur->texture_handle = 0;
                else
                {
                    const QString texture_filename = path + mtl->texture_name;
                    if (textures.contains(texture_filename))
                        cur->texture_handle = textures[texture_filename];
                    else
                    {
                        qDebug() << "loading " << texture_filename;
                        cur->texture_handle = gl->bindTexture(QImage(texture_filename), GL_TEXTURE_2D, GL_RGB8, QGLContext::LinearFilteringBindOption);
                        textures[texture_filename] = cur->texture_handle;
                    }
                }
                if (mtl->normal_texture_name.isEmpty())
                    cur->normal_texture_handle = 0;
                else
                {
                    const QString normal_texture_filename = path + mtl->normal_texture_name;
                    if (textures.contains(normal_texture_filename))
                        cur->normal_texture_handle = textures[normal_texture_filename];
                    else
                    {
                        qDebug() << "loading " << normal_texture_filename;
                        cur->normal_texture_handle = gl->bindTexture(QImage(normal_texture_filename), GL_TEXTURE_2D, GL_RGB8, QGLContext::LinearFilteringBindOption);
                        textures[normal_texture_filename] = cur->normal_texture_handle;
                    }
                }
            }
            break;
        case 'v':
            file >> word;
            if (word == "v")
            {
                Vec3 p;
                file >> p.x() >> p.y() >> p.z();
                t_vtx.push_back(p);
                vtx_cnt++;


            }
            else if (word == "vn")
            {
                Vec3 n;
                file >> n.x() >> n.y() >> n.z();
            }
            else if (word == "vt")
            {
                Vec2 t;
                float tmp;
                file >> t.x() >> t.y() >> tmp;
                t_tcd.push_back(t);
            }
            break;
        case 'm':
            file >> word;
            if (word == "mtllib")
            {
                word = file.getLine();
                 QString mtllib_filename = (path + QString(word.c_str()).trimmed()).trimmed();
                 if (mtllib_filename.endsWith(".mtl", Qt::CaseInsensitive)) {
                qDebug() << "loading mtl file " << mtllib_filename;
                mtllib = loadMTL(mtllib_filename);
                mat_cnt = mtllib.size();
                qDebug() << mat_cnt << " materials loaded";
                mtl = &(mtllib["default"]);
                 }
            }
            break;
        case 'f':
            file >> word;
            if (word == "f")
            {
                file.skipWhitespaces();
                char buf[1024];
                file.getSplitLine(buf, 1024);
                elts.clear();
                split(buf, ' ', elts, false);
                size_t a, b;
                for(size_t i = 0 ; i < elts.size() ; ++i)
                {
                    components.clear();
                    split(elts[i], '/', components);
                    int v_id(0);
                    int t_id(-1);
                    v_id = parse_int(components[0]) - 1;
                    if (components.size() > 1)	t_id = parse_int(components[1]) - 1;
                    const pair<int,int> p(v_id, t_id);
                    size_t idx;
                    unordered_map<pair<int, int>, size_t>::const_iterator it = elts_table.find(p);
                    if (it == elts_table.end())
                    {
                        idx = cur->vertex.size();
                        elts_table.insert(make_pair(p, idx));
                        cur->vertex.push_back(t_vtx[v_id]);
                        if (t_id == -1)
                            cur->tcoord.push_back(Vec2(.0f,.0f));
                        else
                            cur->tcoord.push_back(t_tcd[t_id]);
                    }
                    else
                        idx = it->second;
                    if (i == 0)
                        a = idx;
                    else if (i > 1)
                    {
                        cur->index.push_back(a);
                        cur->index.push_back(b);
                        cur->index.push_back(idx);
                    }
                    b = idx;
                }
            }
            face_cnt++;
            break;
        case 's':
            file >> word;
            if (word == "s")
            {
                file.skipLine();
            }
            break;
        default:
            file.skipLine();
        }
    }
    this->mtllib = mtllib;
    this->vtx_cnt = vtx_cnt / 3;
    this->face_cnt = face_cnt / 3;

    recompute_normals();
}

