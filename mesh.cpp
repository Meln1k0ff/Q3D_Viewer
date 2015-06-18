#include <GL/glew.h>
#include <GL/gl.h>

//#include "utils.h"
//#include "stripify.h"
#include <QGLShaderProgram>
#include <QGLShader>

#include <QGLContext>
#include <QFileInfo>
#include <QDebug>
#include <QHash>
#include "mesh.h"


#define STRINGIFY(X)	#X
using namespace  std;


namespace
{
    static const char * const vertex_shader_code = STRINGIFY(
        varying vec3 normal;
        varying vec2 uv;
        varying vec3 viewpos;
        varying vec4 color;
        varying vec3 tangent;
        void main()
        {
            gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
            vec4 tmp = gl_ModelViewMatrix * gl_Vertex;
            viewpos = tmp.xyz / tmp.w;
            uv = gl_MultiTexCoord0.xy;
            uv.y *= -1.0;
            normal = normalize(gl_NormalMatrix * gl_Normal);
            color = gl_Color;

            vec3 c1 = cross(gl_Normal, vec3(0.0, 0.0, 1.0));
            vec3 c2 = cross(gl_Normal, vec3(0.0, 1.0, 0.0));

            if(length(c1) > length(c2))
                tangent = c1;
            else
                tangent = c2;
            tangent = normalize(gl_NormalMatrix * tangent);
        }
                                                       );
    static const char * const fragment_shader_code = STRINGIFY(
        varying vec3 normal;
        varying vec2 uv;
        varying vec3 viewpos;
        varying vec4 color;
        varying vec3 tangent;
        uniform sampler2D diffuse_texture;
        uniform sampler2D normal_texture;
        uniform bool bUseTexture;
        uniform bool bUseColor;
        uniform bool bUseNormalTexture;
        void main()
        {
            vec3 n = normalize(normal);
            if (bUseNormalTexture)
            {
                vec3 t = normalize(tangent);
                vec3 b = normalize(cross(n, t));
                vec3 vn = 2.0 * texture2D(normal_texture, uv).xyz - 1.0;
                n = normalize(vn.x * t + vn.y * b + vn.z * n);
            }
            if (!gl_FrontFacing)
                n = -n;
            vec3 viewdir = normalize(viewpos);
            float diffuse_term = max(0.0,dot(gl_LightSource[0].position.xyz, n));
            float specular_term = pow(max(0.0,dot(reflect(gl_LightSource[0].position.xyz, n), viewdir)), gl_FrontMaterial.shininess);
            vec4 diffuse_color = (bUseColor ? color : gl_FrontMaterial.diffuse) *  (bUseTexture ? texture2D(diffuse_texture, uv) : vec4(1.0));
            gl_FragColor = diffuse_term * diffuse_color + specular_term * gl_FrontMaterial.specular;
            gl_FragColor.a = 1.0;
        }
                                                       );
}


QGLShaderProgram *Mesh::pShader = NULL;

Mesh::Mesh()
{
    texture_handle = 0;
    normal_texture_handle = 0;
    vbo_index = 0;
    vbo_vtx = 0;
    vbo_color = 0;
    vbo_normal = 0;
    vbo_tcoord = 0;
    clear();
      //unordered_map<string, Materials::Material> _mtllib;
}

Mesh::~Mesh()
{
    clear();
}
/*Очищает меш*/
void Mesh::clear()
{
    for(size_t i = 0 ; i < childs.size() ; ++i)
        delete childs[i];
    childs.clear();
    name.clear();
    vertex.clear();
    normal.clear();
    color.clear();
    tcoord.clear();
    index.clear();
    primitive_type = 0;
    if (texture_handle)
    {
        GLuint id = texture_handle;
        glDeleteTextures(1, &id);
    }
    texture_handle = 0;
    if (normal_texture_handle)
    {
        GLuint id = normal_texture_handle;
        glDeleteTextures(1, &id);
    }
    normal_texture_handle = 0;

    if (vbo_vtx)	glDeleteBuffers(1, &vbo_vtx);
    if (vbo_color)	glDeleteBuffers(1, &vbo_color);
    if (vbo_normal)	glDeleteBuffers(1, &vbo_normal);
    if (vbo_tcoord)	glDeleteBuffers(1, &vbo_tcoord);
    if (vbo_index)	glDeleteBuffers(1, &vbo_index);
    vbo_index = 0;
    vbo_vtx = 0;
    vbo_color = 0;
    vbo_normal = 0;
    vbo_tcoord = 0;

    local = Mat44::identity();

    diffuse = Vec4(1.f,1.f,1.f,1.f);
    specular = Vec4(0.f,0.f,0.f,0.f);
    shininess = 1.f;
    transparency = 0.f;
}

void Mesh::draw()
{
    if (!pShader && QGLShaderProgram::hasOpenGLShaderPrograms())
    {
        pShader = new QGLShaderProgram;
        pShader->addShaderFromSourceCode(QGLShader::Vertex, vertex_shader_code);
        pShader->addShaderFromSourceCode(QGLShader::Fragment, fragment_shader_code);
    }

    if (pShader)
    {
        pShader->bind();
    }

    glEnableClientState(GL_VERTEX_ARRAY);
    if (!vbo_vtx)
        glVertexPointer(3, GL_FLOAT, 0, vertex.data());
    else
    {
        glBindBuffer(GL_ARRAY_BUFFER, vbo_vtx);
        glVertexPointer(3, GL_FLOAT, 0, 0);
    }
    if (!color.empty())
    {
        glEnable(GL_COLOR_MATERIAL);
        glEnableClientState(GL_COLOR_ARRAY);
        if (!vbo_color)
            glColorPointer(3, GL_UNSIGNED_BYTE, 0, color.data());
        else
        {
            glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
            glColorPointer(3, GL_UNSIGNED_BYTE, 0, 0);
        }
        if (pShader)
            pShader->setUniformValue("bUseColor", true);
    }
    else
    {
        glDisable(GL_COLOR_MATERIAL);
        glDisableClientState(GL_COLOR_ARRAY);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, (const GLfloat*)&diffuse);
        if (pShader)
            pShader->setUniformValue("bUseColor", false);
    }
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, (const GLfloat*)&specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
    if (!normal.empty())
    {
        glEnableClientState(GL_NORMAL_ARRAY);
        if (!vbo_normal)
            glNormalPointer(GL_FLOAT, 0, normal.data());
        else
        {
            glBindBuffer(GL_ARRAY_BUFFER, vbo_normal);
            glNormalPointer(GL_FLOAT, 0, 0);
        }
    }
    else
        glDisableClientState(GL_NORMAL_ARRAY);
    if (!tcoord.empty() && texture_handle)
    {
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        if (!vbo_tcoord)
            glTexCoordPointer(2, GL_FLOAT, 0, tcoord.data());
        else
        {
            glBindBuffer(GL_ARRAY_BUFFER, vbo_tcoord);
            glTexCoordPointer(2, GL_FLOAT, 0, 0);
        }

        glActiveTexture(GL_TEXTURE1);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, normal_texture_handle);
        glActiveTexture(GL_TEXTURE0);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texture_handle);
        if (pShader)
        {
            pShader->setUniformValue("bUseTexture", true);
            pShader->setUniformValue("bUseNormalTexture", normal_texture_handle != 0);
            pShader->setUniformValue("diffuse_texture", (GLuint)0);
            pShader->setUniformValue("normal_texture", (GLuint)1);
        }
    }
    else
    {
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glActiveTexture(GL_TEXTURE1);
        glDisable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE0);
        glDisable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
        if (pShader)
        {
            pShader->setUniformValue("bUseTexture", false);
            pShader->setUniformValue("bUseNormalTexture", false);
        }
    }

    glPushMatrix();
    glMultMatrixf((const GLfloat*)local.data());

    if (!vbo_index)
        glDrawRangeElements(primitive_type, 0, vertex.size() - 1, index.size(), GL_UNSIGNED_INT, index.data());
    else
    {
        if (glPrimitiveRestartIndex)
        {
            glEnable(GL_PRIMITIVE_RESTART);
            glPrimitiveRestartIndex(0xFFFFFFFFU);
        }
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_index);
        glDrawRangeElements(primitive_type, 0, vertex.size() - 1, index.size(), GL_UNSIGNED_INT, 0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    for(size_t i = 0 ; i < childs.size() ; ++i)
        childs[i]->draw();
    glPopMatrix();
}

void Mesh::recompute_normals()
{
    normal.clear();
    normal.resize(vertex.size());
    memset(normal.data(), 0, normal.size() * sizeof(Vec3));
    switch(primitive_type)
    {
    case GL_TRIANGLES:
        for(size_t i = 0 ; i + 2 < index.size() ; i += 3)
        {
            const uint32_t ida = index[i];
            const uint32_t idb = index[i + 1];
            const uint32_t idc = index[i + 2];
            const Vec3 &a = vertex[ida];
            const Vec3 &b = vertex[idb];
            const Vec3 &c = vertex[idc];
            const Vec3 &n = (b - a) ^ (c - a);
            normal[ida] += n;
            normal[idb] += n;
            normal[idc] += n;
        }
        break;
    case GL_TRIANGLE_STRIP:
        for(size_t i = 0 ; i + 2 < index.size() ; ++i)
        {
            const uint32_t ida = index[i];
            const uint32_t idb = index[i + 1];
            const uint32_t idc = index[i + 2];
            const Vec3 &a = vertex[ida];
            const Vec3 &b = vertex[idb];
            const Vec3 &c = vertex[idc];
            const Vec3 &n = (b - a) ^ (c - a);
            normal[ida] += n;
            normal[idb] += n;
            normal[idc] += n;
        }
        break;
    case GL_QUADS:
        for(size_t i = 0 ; i + 3 < index.size() ; i += 4)
        {
            const uint32_t ida = index[i];
            const uint32_t idb = index[i + 1];
            const uint32_t idc = index[i + 2];
            const uint32_t idd = index[i + 3];
            const Vec3 &a = vertex[ida];
            const Vec3 &b = vertex[idb];
            const Vec3 &c = vertex[idc];
            const Vec3 &d = vertex[idd];
            const Vec3 &n0 = (b - a) ^ (c - a);
            const Vec3 &n1 = (c - a) ^ (d - a);
            normal[ida] += n0 + n1;
            normal[idb] += n0;
            normal[idc] += n0 + n1;
            normal[idd] += n1;
        }
        break;
    }
#pragma omp parallel for
    for(std::vector<Vec3>::iterator i = normal.begin() ; i < normal.end() ; ++i)
        i->normalize();

#pragma omp parallel for
    for(size_t i = 0 ; i < childs.size() ; ++i)
        childs[i]->recompute_normals();
}


void Mesh::load(QString &filename)
{
    clear();
    qDebug() << "mesh_load";

    if (filename.endsWith(".obj", Qt::CaseInsensitive))
    {
        loadOBJ(filename);
        return;
    }
    else if (filename.endsWith(".3ds",Qt::CaseInsensitive)){
        load3DS(filename);
        return;

    }
}

std::pair<Vec3, Vec3> Mesh::getBoundingBox() const
{
   Vec3 m(.0f,.0f,.0f);
    Vec3 M(.0f,.0f,.0f);
    if (!vertex.empty())
        m = M = vertex.front();
    for(std::vector<Vec3>::const_iterator v = vertex.begin() ; v != vertex.end() ; ++v)
    {
        m[0] = std::min(m[0], (*v)[0]);
        m[1] = std::min(m[1], (*v)[1]);
        m[2] = std::min(m[2], (*v)[2]);
        M[0] = std::max(M[0], (*v)[0]);
        M[1] = std::max(M[1], (*v)[1]);
        M[2] = std::max(M[2], (*v)[2]);
    }

    for(size_t i = 0 ; i < childs.size() ; ++i)
    {
        const std::pair<Vec3, Vec3> &bbox = childs[i]->getBoundingBox();
        if (vertex.empty() && i == 0)
        {
            m = bbox.first;
            M = bbox.second;
        }

        for(size_t j = 0 ; j < 3 ; ++j)
        {
            m[j] = std::min(m[j], bbox.first[j]);
            M[j] = std::max(M[j], bbox.second[j]);
        }
    }
    return std::make_pair(m, M);
}

void Mesh::optimize()
{
    if (!glewIsSupported("GL_ARB_vertex_buffer_object"))
        return;

    glGenBuffers(1, &vbo_vtx);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_vtx);
    glBufferData(GL_ARRAY_BUFFER, vertex.size() * sizeof(vertex.front()), vertex.data(), GL_STATIC_DRAW);
    if (!color.empty())
    {
        glGenBuffers(1, &vbo_color);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
        glBufferData(GL_ARRAY_BUFFER, color.size() * sizeof(color.front()), color.data(), GL_STATIC_DRAW);
    }
    if (!normal.empty())
    {
        glGenBuffers(1, &vbo_normal);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_normal);
        glBufferData(GL_ARRAY_BUFFER, normal.size() * sizeof(normal.front()), normal.data(), GL_STATIC_DRAW);
    }
    if (!tcoord.empty())
    {
        glGenBuffers(1, &vbo_tcoord);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_tcoord);
        glBufferData(GL_ARRAY_BUFFER, tcoord.size() * sizeof(tcoord.front()), tcoord.data(), GL_STATIC_DRAW);
    }

    glGenBuffers(1, &vbo_index);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_index);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index.size() * sizeof(index.front()), index.data(), GL_STATIC_DRAW);

    for(size_t i = 0 ; i < childs.size() ; ++i)
        childs[i]->optimize();
}

void Mesh::swap(Mesh *mesh)
{
    name.swap(mesh->name);
    vertex.swap(mesh->vertex);
    normal.swap(mesh->normal);
    color.swap(mesh->color);
    tcoord.swap(mesh->tcoord);
    index.swap(mesh->index);
    std::swap(primitive_type, mesh->primitive_type);
    std::swap(texture_handle, mesh->texture_handle);
    childs.swap(mesh->childs);

    std::swap(vbo_vtx, mesh->vbo_vtx);
    std::swap(vbo_color, mesh->vbo_color);
    std::swap(vbo_normal, mesh->vbo_normal);
    std::swap(vbo_tcoord, mesh->vbo_tcoord);
    std::swap(vbo_index, mesh->vbo_index);

    std::swap(local, mesh->local);

    std::swap(diffuse, mesh->diffuse);
    std::swap(specular, mesh->specular);
    std::swap(shininess, mesh->shininess);
}

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



