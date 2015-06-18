#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDockWidget>
#include <QPushButton>
#include "view3d.h"
#include <QToolBar>
#include <QTranslator>
#include <QListWidget>

namespace Ui {

class MainWindow;

}


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    MainWindow(QWidget *parent = 0);
    QDockWidget widget;
    QPushButton button;
    QString fileName;
    unordered_map<std::string, Material> MTLlist;

    ~MainWindow();

    QStringList matlist;
    float transformAngle(int position);
    std::vector<string> matNames;
    void flush();

protected:

      /** creates the language menu dynamically from the content of m_langPath
        */




private slots:

    void highlight(QListWidgetItem *item);

    void on_actionOpen_triggered();

    void on_stepZoomIn_clicked();

    void on_stepZoomOut_clicked();

    void on_normButton_clicked();

    void on_upButton_clicked();

    void on_downButton_clicked();

    void on_leftButton_clicked();

    void on_rightButton_clicked();

    void on_closeModelBtn_clicked();

    void on_x_rot_down_clicked();

    void on_x_rot_up_clicked();

    void on_y_rot_down_clicked();

    void on_y_rot_up_clicked();

    void on_z_rot_down_clicked();

    void on_z_rot_up_clicked();

    void on_optimizeButton_clicked();



    void on_actionClose_triggered();

    void on_actionExit_triggered();

    void on_actionZoom_In_triggered();

    void on_actionZoom_out_triggered();

    void on_fullScreenModelBtn_clicked();


void on_showRightWidget_clicked();


    void on_showLeftWiget_clicked();



    void on_hideRightWidget_clicked();

    void on_hideLeftWidget_clicked();

    void on_upButton_pressed();

    void on_upButton_released();

    void on_x_rot_down_pressed();

    void on_x_rot_down_released();


private:
    Ui::MainWindow *ui;
    QTranslator     m_translator;   /**< contains the translations for this application */
     QTranslator     m_translatorQt; /**< contains the translations for qt */
     QString         m_currLang;     /**< contains the currently loaded language */
     QString         m_langPath;     /**< Path of language files. This is always fixed to /languages. */
     void disableButtons();
     void enableButtons();
     void scrollmatListLeft();
     void scrollmatListRight();
     QListWidget listMat;
     QListWidgetItem *item;

       float x_angle;
};

#endif // MAINWINDOW_H
