#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "view3d.h"
#include "utils.h"
#include <QGLWidget>
#include <QFileDialog>
#include <QShortcut>
#include <iostream>
#include "mesh.h"
#include <QProcess>


#define _USE_MATH_DEFINES


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->openModelBtn,SIGNAL(clicked()),this,SLOT(on_actionOpen_triggered()));

    ui->closeModelBtn->setDisabled(true);

    ui->menuToolBar->addWidget(ui->openModelBtn);
    ui->menuToolBar->addWidget(ui->fullScreenModelBtn);

    ui->menuToolBar->addWidget(ui->closeModelBtn);

    //path
    ui->statusBar->addWidget(ui->pathLabel);
    ui->statusBar->addWidget(ui->pathString);
    //points
    ui->statusBar->addWidget(ui->pointslabel);
    ui->statusBar->addWidget(ui->pointsString);
    //faces
    ui->statusBar->addWidget(ui->facesLabel);
    ui->statusBar->addWidget(ui->facesString);
    ui->closeLbl->show();
    ui->statusBar->addWidget(ui->closeLbl);

     ui->statusBar->addWidget(ui->matLab);
      ui->statusBar->addWidget(ui->matstring);

    disableButtons();
    ui->right_widget->show();
    ui->left_widget->show();
    ui->showLeftWiget->hide();
      ui->showRightWidget->hide();

      ui->widget_3d->mesh->mtllib.clear();


      ui->upButton->setAutoRepeat(true);
      ui->downButton->setAutoRepeat(true);
      ui->leftButton->setAutoRepeat(true);
      ui->rightButton->setAutoRepeat(true);

      ui->x_rot_down->setAutoRepeat(true);
      ui->x_rot_up->setAutoRepeat(true);
      ui->y_rot_up->setAutoRepeat(true);
      ui->y_rot_down->setAutoRepeat(true);
      ui->z_rot_up->setAutoRepeat(true);
      ui->z_rot_down->setAutoRepeat(true);

      ui->stepZoomOut->setAutoRepeat(true);
      ui->stepZoomIn->setAutoRepeat(true);

      /*ui->upButton->setAutoRepeat(true);
      ui->downButton->setAutoRepeat(true);*/
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionOpen_triggered(){

    ui->matlistWidget->clear();
     //ui->matlistWidget->clear();
    fileName =
                       QFileDialog::getOpenFileName(this,tr("Open Document"),
                           QDir::currentPath(),
                                                   tr("obj files (*.obj);;3ds files (*.3ds);;all files(*)" ));
    ui->widget_3d->load(fileName);
    qDebug() << "view3d_load";

   MTLlist = ui->widget_3d->mesh->loadMTL(fileName);

       vector<std::string> mat_names;
       mat_names.reserve(MTLlist.size());
       vector<Material> material_val;
       material_val.reserve(MTLlist.size());

       for(pair<std::string, Material> a : MTLlist) {
            mat_names.push_back(a.first);
       }

       for(std::string a  :ui->widget_3d->mats) {

           QString str = QString::fromUtf8(a.c_str());

           item = new QListWidgetItem(ui->matlistWidget);
           item->setCheckState(Qt::Checked);
           item->setText(str);
       }


     connect(ui->matlistWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(highlight(QListWidgetItem*)));



        ui->facesString->setText(QString::number(ui->widget_3d->face_cnt,10));

        ui->pointsString->setText(QString::number(ui->widget_3d->vtx_cnt,10));

        ui->pathString->setText(fileName);

        ui->matstring->setText(QString::number(ui->widget_3d->mesh->mat_cnt,10));

        enableButtons();
}

void MainWindow::highlight(QListWidgetItem *item){

   qDebug() << item->text();
   const std::string a = item->text().toUtf8().constData();
   ui->widget_3d->highlight(a);
}

void MainWindow::disableButtons(){

    ui->upButton->setEnabled(false);
    ui->downButton->setEnabled(false);
    ui->rightButton->setEnabled(false);
    ui->leftButton->setEnabled(false);

    ui->z_rot_down->setEnabled(false);
    ui->z_rot_up->setEnabled(false);
    ui->x_rot_down->setEnabled(false);
    ui->x_rot_up->setEnabled(false);
    ui->y_rot_down->setEnabled(false);
    ui->y_rot_up->setEnabled(false);

    ui->stepZoomIn->setEnabled(false);
    ui->stepZoomOut->setEnabled(false);
    //ui->normButton->setEnabled(false);
    ui->optimizeButton->setEnabled(false);

}

void MainWindow::enableButtons(){

    ui->closeModelBtn->setEnabled(true);


    ui->upButton->setEnabled(true);
       ui->downButton->setEnabled(true);
       ui->rightButton->setEnabled(true);
       ui->leftButton->setEnabled(true);

       ui->z_rot_down->setEnabled(true);
       ui->z_rot_up->setEnabled(true);
       ui->x_rot_down->setEnabled(true);
       ui->x_rot_up->setEnabled(true);
       ui->y_rot_down->setEnabled(true);
       ui->y_rot_up->setEnabled(true);
       ui->stepZoomIn->setEnabled(true);
       ui->stepZoomOut->setEnabled(true);
       //ui->normButton->setEnabled(true);
       ui->optimizeButton->setEnabled(true);

}

void MainWindow::on_stepZoomIn_clicked()
{
    float step = 1.1f;
    ui->widget_3d->zoom_in_step(step);
}

void MainWindow::on_stepZoomOut_clicked()
{
    float step = 0.9f;
    ui->widget_3d->zoom_out_step(step);

}

void MainWindow::on_normButton_clicked()
{
    ui->widget_3d->normalize();
}

void MainWindow::on_optimizeButton_clicked()
{
    ui->widget_3d->optimize();
}

void MainWindow::on_upButton_clicked()
{
   float step = 1.5f;
    ui->widget_3d->move_up(step);
}

void MainWindow::on_downButton_clicked()
{
    float step = 1.5f;
     ui->widget_3d->move_down(step);
}

void MainWindow::on_leftButton_clicked()
{
    float step = 1.5f;
     ui->widget_3d->move_left(step);
}

void MainWindow::on_rightButton_clicked()
{
    float step = 1.5f;
     ui->widget_3d->move_right(step);
}

void MainWindow::on_closeModelBtn_clicked()
{
    ui->closeLbl->setText("Закрываю...");
    ui->widget_3d->flush();
    ui->facesString->clear();
    ui->pointsString->clear();
    ui->pathString->clear();
    ui->matlistWidget->clear();
    ui->closeModelBtn->setDisabled(true);
    ui->closeLbl->setText("");
     ui->widget_3d->mesh->mtllib.clear();
     ui->matlistWidget->clear();

    disableButtons();
}

void MainWindow::on_x_rot_down_clicked()
{
    float x_angle =  ui->widget_3d->last_cursor_pos.x() / width();

   if (x_angle <= 0)
       x_angle = 0;
x_angle--;
   qDebug() << x_angle;
   ui->widget_3d->rotate_x(x_angle);
}

void MainWindow::on_x_rot_up_clicked()
{
    float x_angle = ui->widget_3d->last_cursor_pos.x() / width();

   if (x_angle >= 2*M_PI)
       x_angle = 0;
    x_angle++;
   qDebug() << x_angle;

   ui->widget_3d->rotate_x(x_angle);
}

void MainWindow::on_y_rot_down_clicked()
{
    float y_angle = ui->widget_3d->last_cursor_pos.y() / height();
   if (y_angle<= 0)
       y_angle = 0;
   y_angle--;

    ui->widget_3d->rotate_y(y_angle);
}

void MainWindow::on_y_rot_up_clicked()
{
     float y_angle = ui->widget_3d->last_cursor_pos.y() / height();
    if (y_angle >= 2*M_PI)
        y_angle = 0;
    y_angle++;
     ui->widget_3d->rotate_y(y_angle);
}

void MainWindow::on_z_rot_down_clicked()
{
    float z_angle = ui->widget_3d->last_cursor_pos.y() / width();
    if (z_angle <= 0)
        z_angle= 0;

   z_angle--;
    ui->widget_3d->rotate_z(z_angle);
}

void MainWindow::on_z_rot_up_clicked()
{
    float z_angle =  ui->widget_3d->last_cursor_pos.x() / height();
    if (z_angle  >= 2*M_PI)
        z_angle = 0;

   z_angle++;
    ui->widget_3d->rotate_z(z_angle);
}


void MainWindow::on_actionClose_triggered()
{
     ui->closeLbl->setText("Закрываю...");

    ui->widget_3d->flush();
    ui->facesString->clear();
    ui->pointsString->clear();
    ui->pathString->clear();
    ui->matlistWidget->clear();

    ui->closeLbl->setText("");
    disableButtons();
}

void MainWindow::on_actionExit_triggered()
{
    QApplication::exit(0);
}

void MainWindow::on_actionZoom_In_triggered()
{
    float step = 1.1f;
    ui->widget_3d->zoom_in_step(step);
}

void MainWindow::on_actionZoom_out_triggered()
{
    float step = 0.9f;
    ui->widget_3d->zoom_out_step(step);
}

void MainWindow::on_fullScreenModelBtn_clicked()
{
    this->showFullScreen();


    ui->widget_3d->showFullScreen();
}

void MainWindow::scrollmatListLeft(){
    ui->left_widget->hide();
}

void MainWindow::scrollmatListRight(){
     ui->left_widget->show();
}

void MainWindow::on_showRightWidget_clicked()
{
    ui->right_widget->show();
      ui->showRightWidget->hide();
}

void MainWindow::on_showLeftWiget_clicked()
{
    ui->left_widget->show();
    ui->showLeftWiget->hide();

}

void MainWindow::on_hideRightWidget_clicked()
{
     ui->right_widget->hide();
     ui->showRightWidget->show();
}

void MainWindow::on_hideLeftWidget_clicked()
{
      ui->left_widget->hide();
       ui->showLeftWiget->show();
}

void MainWindow::on_x_rot_down_pressed()
{

    x_angle = ui->widget_3d->last_cursor_pos.x() / width();

       if (x_angle <= 0)
           x_angle = 0;
    x_angle--;
       qDebug() << x_angle;
}

void MainWindow::on_x_rot_down_released()
{
    // ui->x_rot_down->released();
      ui->widget_3d->rotate_x(x_angle);
}

void MainWindow::on_upButton_pressed()
{
    /*float step = 1.0f;
     ui->widget_3d->move_up(step);*/
     qDebug() << "press";
}


void MainWindow::on_upButton_released()
{
    /*float step = 1.0f;
     ui->widget_3d->move_up(step);*/


     qDebug() << "release";
}
