
#include "view3d.h"

#include <QGLFormat>
#include <QMouseEvent>
//#include "utils.h"
#include <QDebug>
#include "utils.h"

namespace
{
    const QGLFormat gl_format(QGL::DoubleBuffer | QGL::Rgba | QGL::DepthBuffer | QGL::DirectRendering);
}

/*Класс, отвечающий за виджет opengl окна*/

View3d::View3d(QWidget *parent) :
    QGLWidget(gl_format, parent)
{

    mesh = new Mesh;
    x_angle = 30;
     y_angle = 30;
    z_angle = 30;

    center = Vec3(0,0,0);
    radius = 1.f;
    scaleRatio = 1.0f;

    makeCurrent();

    glewInit();
    setFormat(QGLFormat(QGL::DoubleBuffer | QGL::DepthBuffer));
    //mesh->mat_names;
}

View3d::~View3d()
{
    //makeCurrent();
    delete mesh;
}

void View3d::initializeGL()
{
    //qglClearColor(Qt::black);
    glClearColor(0.75,0.75,1.0,0);
    glShadeModel(GL_FLAT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);

    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glEnable(GL_NORMALIZE);

    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
    glEnable(GL_LIGHTING);

    const GLfloat ambient[] = { 0.f, 0.f, 0.f, 0.f };
    const GLfloat diffuse[] = { 1.f, 1.f, 1.f, 1.f };
    const GLfloat specular[] = { 1.f, 1.f, 1.f, 1.f };
    const GLfloat position[] = { 1.f / std::sqrt(3.f), 1.f / std::sqrt(3.f), 1.f / std::sqrt(3.f), 0.f };
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
    glLightfv(GL_LIGHT0, GL_POSITION, position);
    glEnable(GL_LIGHT0);

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glScalef(1.f,-1.f,1.f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void View3d::resizeGL(int w, int h)
{
    /*glViewport(0,0,w,h);*/
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    GLfloat x = w / h;
    glFrustum(-x, x, -1.0, 1.0, 4.0, 15.0);
    glMatrixMode(GL_MODELVIEW);
}

void View3d::draw(){

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -10.0);
    glRotatef(rotationX, 1.0, 0.0, 0.0);
    glRotatef(rotationY, 0.0, 1.0, 0.0);
    glRotatef(rotationZ, 0.0, 0.0, 1.0);
    for (int i = 0; i < 6; ++i) {
        glLoadName(i);
        glBegin(GL_QUADS);
        qglColor(faceColors[i]);
        for (int j = 0; j < 4; ++j) {
           // glVertex3f(coords[i] [j] [0], coords[i] [j] [1],
            //coords[i] [j] [2]);
        }
        glEnd();
    }
    mesh->draw();
}

void View3d::paintGL()
{
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1.0, 1.0, 1.0, 0.0f);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(70.0, double(width()) / height(), 0.75 * radius, 2.75 * radius);
        glMatrixMode(GL_MODELVIEW);

        glPushMatrix();

        glTranslatef(center[0],center[1],center[2]);
        glMultMatrixd((const GLdouble*)mesh_mat.data());
        glTranslatef(-center[0],-center[1],-center[2]);

        mesh->draw();
        glColor4f(1.0f,1.0f,1.0f,0.5f);
        glPopMatrix();
}
/*Загрузка меша из файла и  отображение его на виджет*/

void View3d::load( QString &filename)
{
    makeCurrent();
    qDebug() << "view3d_load_2";
    mesh->load(filename);
    mesh->optimize();
    this->filename = filename;
    const std::pair<Vec3, Vec3> &bbox = mesh->getBoundingBox();
    center = 0.5f * (bbox.first + bbox.second);
    radius = 0.5f * (bbox.second - bbox.first).length();

    const Vec3 view_dir(0,0,-1);
    const Vec3 up(0,1,0);
    const Vec3 eye = center - 1.75f * radius * view_dir;

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(eye[0], eye[1], eye[2], center[0], center[1], center[2], up[0], up[1], up[2]);

    mesh_mat = QMatrix4x4();
    update();
    face_cnt = mesh->face_cnt;
    vtx_cnt = mesh->vtx_cnt;
    for (string a : mesh->mat_names){
         mats.push_back(a);
    }
}

/*Скрывает материал, при нажатии на чекбокс - могут возникать проблемы при отрисовке*/
void View3d::hideMaterial(string matname){

    makeCurrent();
    pair<string,Material> mat;
     unordered_map<string, Material>::iterator it;

     it = mesh->mtllib.find(matname);

     Material a = mesh->mtllib[matname];

     qDebug() << a.shininess;   

    glColor4f(1.0, 1.0, 1.0, 0.5);

}
/*Подсветка материала - на данный момент не работает */
void View3d::highlight(const string matname){
     makeCurrent();
     unordered_map<string, Material>::iterator it;
     it = mesh->mtllib.find(matname);

     Material a = mesh->mtllib[matname];

     qDebug() << "ax"<<a.ambient.x();
qDebug() <<      a.ambient.y();
     qDebug() << a.ambient.z();

     a.ambient.x()=0.5000;
     a.ambient.y()=0.5000;
     a.ambient.z()=0.5000;
    update();
     glColor3f(a.ambient.x(),a.ambient.y(),a.ambient.z());
update();
     //qDebug() << a.shininess;

     //a.specular =  Vec4(0.0f,0.0f,0.0f,0.0f);
     //a.diffuse = Vec4(0.0f,0.0f,0.0f,0.0f);
     //a.shininess = 0.0f;
       qDebug() << a.transparency;
     //qDebug() << a.shininess;
    a.transparency = 0.5f;
       qDebug() << a.transparency;
    qDebug() << a.texture_name;
     qDebug() << a.normal_texture_name;
     update();
}

void View3d::showMaterial(string matname){

    MTLlist.find(matname);
    unordered_map<string, Material>::iterator it;

    it = MTLlist.find(matname);   
}


/*События движения мыши*/
void View3d::mouseMoveEvent(QMouseEvent *e)
{



    if (e->buttons() & (Qt::LeftButton | Qt::RightButton)) {
          GLfloat rx = (GLfloat) (e->x() - last_cursor_pos.x()) / width();
          GLfloat ry = (GLfloat) (e->y() - last_cursor_pos.y()) / height();


          makeCurrent();
          glMatrixMode(GL_MODELVIEW);
          if (e->buttons() & Qt::LeftButton) {
              // Left button down - rotate around X and Y axes
              glRotatef(-180*ry, 1, 0, 0);
              glRotatef(180*rx, 0, 1, 0);
          } else if (e->buttons() & Qt::RightButton) {
              // Right button down - rotate around X and Z axes
              glRotatef(-180*ry, 1, 0, 0);
              glRotatef(-180*rx, 0, 0, 1);
          }
          updateGL();
          last_cursor_pos = e->pos();
      } else {

      }
}
/*События колесика мыши*/
void View3d::mouseWheelEvent(QWheelEvent  *e){
    if(e->delta() > 0)
       {
        qDebug() << "zoom in";

    zoom_in_step(e->delta());
       }
     else {
         //qDebug() << "zoom out";
       zoom_out_step(e->delta());
    }
    update();
}


void View3d::mousePressEvent(QMouseEvent *e)
{
    last_cursor_pos = e->pos();
}

/*Возвращает модель в исходное состояние*/

void View3d::optimize(){

    glMatrixMode(GL_MODELVIEW);

    const int windowWidth = this->width() ;
    const int windowHeight = this->height() ;

    makeCurrent();

    const std::pair<Vec3, Vec3> &bbox = mesh->getBoundingBox();
    center = 0.5f * (bbox.first + bbox.second);
    radius = 0.5f * (bbox.second - bbox.first).length();

    const Vec3 view_dir(0,0,-1);
    const Vec3 up(0,1,0);
    const Vec3 eye = center - 1.75f * radius * view_dir;

    zNear = 1.0f; zFar = zNear + radius;
    GLdouble left = center.x() - radius;
    GLdouble right = center.x() + radius;
    GLdouble bottom = center.y() - radius;
    GLdouble top = center.y() + radius;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(left, right, bottom, top, zNear, zFar);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    GLdouble aspect = (GLdouble) windowWidth / windowHeight;
    if ( aspect < 1.0 ) {

        // window taller than wide
        bottom /= aspect;
        top /= aspect;
    } else {
        left *= aspect;
        right *= aspect;
    }


    glMatrixMode(GL_MODELVIEW);

   // gluLookAt(eye[0], eye[1], eye[2], center[0], center[1], center[2], up[0], up[1], up[2]);
    gluLookAt (eye.x(), eye.y(), eye.z(), center.x(), center.y(), center.z(),up.x(),up.y(),up.z());

     update();

}


void View3d::zoom_in_step(float step){
    makeCurrent();

     glScalef (step, step, step);
     update();
}

void View3d::zoom_out_step(float step){
    makeCurrent();

     glScalef (step, step, step);
     update();
}
/*Отображение по размеру виджета просмотра - работает проблематично, пока что не добавлял*/
void View3d::normalize(){

    glMatrixMode(GL_MODELVIEW);

    const int windowWidth = this->width() ;
    const int windowHeight = this->height() ;

    makeCurrent();

    mesh->optimize();

    const std::pair<Vec3, Vec3> &bbox = mesh->getBoundingBox();
    center = 0.5f * (bbox.first + bbox.second);
    radius = 0.5f * (bbox.second - bbox.first).length();

    const Vec3 view_dir(0,0,-1);
    const Vec3 up(0,1,0);
    const Vec3 eye = center - 1.75f * radius * view_dir;

    glMatrixMode(GL_MODELVIEW);

   glLoadIdentity();
   // gluLookAt(eye[0], eye[1], eye[2], center[0], center[1], center[2], up[0], up[1], up[2]);
    gluLookAt (0., 0., radius, center.x(), center.y(), center.z(),up.x(),up.y(),up.z());

     update();
}

void View3d::move_up(float step){
    makeCurrent();
    glTranslatef(0.0f, step,0.0f );
     update();
}

void View3d::move_left(float step){
    makeCurrent();
    glTranslatef(-step, 0.0f, 0.0f);
     update();
}

void View3d::move_right(float step){
     makeCurrent();
     glTranslatef(step, 0.0f, 0.0f);
      update();
}

void View3d::move_down(float step){
     makeCurrent();
     glTranslatef(0.0f, -step,0.0f );
      update();
}

void View3d::rotate_x(float pos){
    //turn to radians

    //GLfloat ry = (GLfloat) (e->y() - last_cursor_pos.y()) / height();
    makeCurrent();

     float angle  = pos * 180 / M_PI;
     //if ((angle >= 2*M_PI) || (angle <=0)) angle = 0;
     glRotatef(angle / 6,1.0f,0.0f,0.0f);
     update();

     this->x_angle = angle;
}

void View3d::rotate_y(float pos){
    makeCurrent();
    float angle  = pos * 180 / M_PI;
    //if (angle >= 2*M_PI) angle = 0;
    glRotatef(angle / 6,0.0f,1.0f,0.0f);
    update();

    this->y_angle = angle;
}

void View3d::rotate_z(float pos){
    makeCurrent();
    float angle  = pos * (180/M_PI);
   // if (angle >= 2*M_PI) angle = 0;
    glRotatef(angle / 6,0.0f,0.0f,1.0f);
    update();

    this->z_angle = angle;
}

/*Очищает viewport*/
void View3d::flush(){


    glClear(GL_COLOR_BUFFER_BIT);

    mesh->clear();
}

