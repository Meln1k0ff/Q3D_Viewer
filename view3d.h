#ifndef VIEW3D_H
#define VIEW3D_H

#include "GL/glew.h"
#include <QGLWidget>
#include <QMatrix4x4>

#include "math.h"
#include "mesh.h"

class View3d : public QGLWidget
{
    friend class Mesh;
    Q_OBJECT
public:
    View3d(QWidget *parent = 0);
    ~View3d();
    Mesh *mesh;

    float x_angle;
    float y_angle;
    float z_angle;

    unordered_map<string, Material> MTLlist;
    void zoom_in_step(float step);
    void zoom_out_step(float step);
    void normalize();
    void move_up(float step);
    void move_left(float step);
    void move_right(float step);
    void move_down(float step);


    void optimize();


    void hideMaterial(string matname);
    void showMaterial(string matname);

    void highlight(const string matname);   

    std::vector<string> mats;

    uint32_t vtx_cnt;
    uint32_t face_cnt;
    QString filename;
      QPointF last_cursor_pos;
     void flush();

signals:

public slots:

     void load(QString &filename);
     void rotate_x(float pos);
     void rotate_y(float pos);
     void rotate_z(float pos);


protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();
    void draw();


   void mouseMoveEvent(QMouseEvent *e);
   void mousePressEvent(QMouseEvent *e);
   void mouseWheelEvent(QWheelEvent *e);


private:
    Vec3 center;
    float radius;
    float zNear;
    float zFar;
    float scaleRatio;
    QMatrix4x4 mesh_mat;

    GLfloat rotationX;
    GLfloat rotationY;
    GLfloat rotationZ;
    QColor faceColors[6];
};

#endif // VIEW3D_H
