//Yasar Utku Alcalar
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ffmpegdecoder_track.h"
#include <opencv2/tracking.hpp>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
using namespace std;
using namespace cv;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void paintEvent(QPaintEvent *);
    QPoint point, point_start, point_end, point_move, temp, temp_2;
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void button_color();
    void drawRect();

private slots:
    void on_BOOSTING_clicked();

    void on_MIL_clicked();

    void on_KCF_clicked();

    void on_TLD_clicked();

    void on_MEDIANFLOW_clicked();

    void on_MOSSE_clicked();

    void on_CSRT_clicked();

    void on_Exit_Tracking_clicked();

private:
    void sendFrame(Mat &src);
    Ui::MainWindow *ui;
    FFmpegDecoder *decoder;

    cv::Ptr<cv::Tracker> tracker;
    cv::Rect2d trackingBox;
    int mode;
    float width_of_screen, height_of_screen;

};
#endif // MAINWINDOW_H
