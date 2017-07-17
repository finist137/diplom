#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ui_mainwindow.h"
#include "coordinates.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:

    void load_pictures();

    void load_start_settings();

    void make_graph(QCustomPlot* graph_KND, QVector<double> KND,
                    QVector<double> time, int x1_range, int x2_range,
                    int y1_range, int y2_range, char* axe_X, char* axe_Y);

    void make_godograph(QCustomPlot* graph_KND, int x1_range, int x2_range, int y1_range,
                        int y2_range, const float* DN,
                        QVector<double> godograph_X, QVector<double> godograph_Y);

    void set_settings();

    void on_pushButton_clicked();

    void on_Button_KA_clicked();

    void on_Button_pitch_clicked();

    void on_Button_DN_clicked();

    void on_save_KND_clicked();

    void on_help_clicked();

    void on_exit_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H





