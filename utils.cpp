//#include <GL/glew.h>
#include "utils.h"
#include <QGLPixelBuffer>
#include "GL/glu.h"
#include "mesh.h"

bool hit_sphere(const Vec3 &pos, const Vec3 &dir, const Vec3 &center, const float radius, Vec3 &hit_point)
{
    const Vec3 &OC = center - pos;
    const float l = OC | dir;
    const Vec3 &p = pos + l * dir;
    const float d2 = (p - center).sq();
    const float r2 = radius * radius;
    if (d2 > r2)
        return false;
    hit_point = pos + (l - std::sqrt(r2 - d2)) * dir;
    return true;
}

void render_thumbnail(QString &filename, const QString &thumbname, const int w, const int h)
{
    QGLPixelBuffer pbuffer(w, h, QGLFormat(QGL::DepthBuffer | QGL::Rgba));
    pbuffer.makeCurrent();

    //glewInit();

    glViewport(0,0,w,h);

    glClearColor(0.75,0.75,1.0,1.0);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);

    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    glEnable(GL_LIGHTING);
    const GLfloat ambient[] = { 0.f, 0.f, 0.f, 1.f };
    const GLfloat diffuse[] = { 1.f, 1.f, 1.f, 1.f };
    const GLfloat specular[] = { 1.f, 1.f, 1.f, 1.f };
    const GLfloat position[] = { 1.f / std::sqrt(3.f), 1.f / std::sqrt(3.f), 1.f / std::sqrt(3.f), 0.f };
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
    glLightfv(GL_LIGHT0, GL_POSITION, position);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
    glEnable(GL_LIGHT0);

    Mesh *mesh = new Mesh;
    mesh->load(filename);
    mesh->optimize();
    const std::pair<Vec3, Vec3> &bbox = mesh->getBoundingBox();
    const Vec3 center = 0.5f * (bbox.first + bbox.second);
    const float radius = 0.5f * (bbox.first - bbox.second).length();

    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glScalef(1.f,-1.f,1.f);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(70.0, double(w) / h, 0.75 * radius, 2.75 * radius);

    const Vec3 view_dir(0,0,-1);
    const Vec3 up(0,1,0);
    const Vec3 eye = center - 1.75f * radius * view_dir;

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(eye[0], eye[1], eye[2], center[0], center[1], center[2], up[0], up[1], up[2]);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    mesh->draw();

    pbuffer.toImage().convertToFormat(QImage::Format_RGB888).save(thumbname, "png");

    delete mesh;
}
