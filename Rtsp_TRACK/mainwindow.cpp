//Yasar Utku Alcalar
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ffmpegdecoder_track.h"
#include "login_track.h"
#include <unistd.h>
#include <QPainter>
#include <QInputDialog>
#include <opencv2/opencv.hpp>
#include <QWidget>
#include <iostream>
#include <QElapsedTimer>
#include <QDebug>
#include <QMouseEvent>
#include <qevent.h>


using namespace std;
using namespace cv;
int counter = 0;
int counter_2 = 0;
bool is_trackingbox_selected = false;
bool draw_rect = false;
cv::Mat src;

static QElapsedTimer t;


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    Login l;

    if(l.exec() != QDialog::Accepted)
    {
        exit(0);
    }

    decoder = new FFmpegDecoder(l.getRtspURL().toStdString());
    decoder->connect();
    mode = l.getMode().toInt();

    if(decoder->isConnected()) {
        new std::thread(&FFmpegDecoder::decode, decoder);
    }
}


MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::paintEvent(QPaintEvent *)
{
    t.start();
    button_color(); // to show which tracking mode is on

    if(decoder->isConnected())
    {
        decoder->mtx.lock();
        if(!decoder->decodedImgBuf.empty())
        {
            src = decoder->decodedImgBuf.front().clone();
            decoder->decodedImgBuf.pop_front();
        }
        decoder->mtx.unlock();


        if(!src.empty())
        {
            if (mode !=1 && mode !=2 && mode !=3 && mode !=4 && mode !=5 && mode !=6 && mode !=7){
                qDebug() << "Invalid Number!\nPlease enter a valid number from the list";
                exit(0);
            }

            width_of_screen = ui->widget->width();
            height_of_screen = ui->widget->height();

            if (counter_2 < 1)
            {
                if (is_trackingbox_selected == false)
                {
                    if ( draw_rect == true )
                    {
                        drawRect();
                    }
                    sendFrame(src);
                    usleep(100000);
                    return;
                }

                else
                {
                    trackingBox = cv::Rect2d(point_start.x()*(1920/width_of_screen), point_start.y()*(1080/height_of_screen),
                                             abs(point_end.x()-point_start.x())*(1920/width_of_screen), abs(point_end.y()-point_start.y())*(1080/height_of_screen));
                }
                counter_2 = counter_2 + 1;
            }

            if (counter < 1)
            {
                if (tracker != NULL)
                {
                    tracker -> clear();
                    tracker = NULL;
                }

                if (mode == 1)
                {
                    tracker = cv::TrackerBoosting::create();
                }

                else if (mode == 2)
                {
                    tracker = cv::TrackerMIL::create();
                }

                else if (mode == 3)
                {
                    tracker = cv::TrackerKCF::create();
                }

                else if (mode == 4)
                {
                    tracker = cv::TrackerTLD::create();
                }

                else if (mode == 5)
                {
                    tracker = cv::TrackerMedianFlow::create();
                }

                else if (mode == 6)
                {
                    tracker = cv::TrackerMOSSE::create();
                }

                else if (mode == 7)
                {
                    tracker = cv::TrackerCSRT::create();
                }

                tracker->init(src, trackingBox);
                counter = counter + 1;
            }


            if ( draw_rect == true )
            {
                drawRect();
            }


            if (tracker->update(src, trackingBox))
            {
                cv::rectangle(src, trackingBox, cv::Scalar(0, 255, 0), 3);
                usleep(25000);
            }

            sendFrame(src);
        }

        else {
            update();
            usleep(10000);
        }
        src.release();
    } else {
        update();
        usleep(10000);
    }
}

void MainWindow::drawRect()
{
    trackingBox = cv::Rect2d(rect_1.x()*(1920/width_of_screen), rect_1.y()*(1080/height_of_screen),
                             abs(rect_2.x()-rect_1.x())*(1920/width_of_screen), abs(rect_2.y()-rect_1.y())*(1080/height_of_screen));
    cv::rectangle(src, trackingBox, cv::Scalar(255, 0, 255), 3);
}


void MainWindow::sendFrame(Mat &src)
{
    QPainter painter(this);
    QImage image = QImage(src.data, src.cols, src.rows, QImage::Format_RGB888);
    QPixmap  pix =  QPixmap::fromImage(image);
    painter.drawPixmap(0, 0, ui->widget->width(), ui->widget->height(), pix);
    this->setWindowTitle("Tracking Application");
    if(is_trackingbox_selected == true){
        if (t.elapsed())
            painter.drawText(10, 10, QString("FPS: %1").arg(1000/t.restart())); }
    update();
    usleep(10000);
}


void MainWindow::mousePressEvent(QMouseEvent *event)
{
    point_start = event->pos();
    point = point_start;
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    point = event->pos();
    point_move = point;
    draw_rect = true;

    if(point_start.x() < point_move.x() && point_start.y() < point_move.y())
    {
        rect_1 = point_start;
        rect_2 = point_move;
    }
    if(point_start.x() > point_move.x() && point_start.y() < point_move.y())
    {
        int temporary = point_start.x();
        int temporary_2 = point_move.x();
        QPoint rect_1_temporary (temporary_2, point_start.y());
        QPoint rect_2_temporary (temporary, point_move.y());
        rect_1 = rect_1_temporary;
        rect_2 = rect_2_temporary;
    }
    if(point_start.x() < point_move.x() && point_start.y() > point_move.y())
    {
        int temporary = point_start.y();
        int temporary_2 = point_move.y();
        QPoint rect_1_temporary (point_start.x(), temporary_2);
        QPoint rect_2_temporary (point_move.x(), temporary);
        rect_1 = rect_1_temporary;
        rect_2 = rect_2_temporary;
    }
    if(point_start.x() > point_move.x() && point_start.y() > point_move.y())
    {
        rect_1 = point_move;
        rect_2 = point_start;
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    draw_rect = false;
    point = event->pos();
    point_end = point;

    if(point_start.x() > point_end.x() && point_start.y() > point_end.y())
    {
        temporary_point = point_start;
        temporary_point2 = point_end;
        point_start  = temporary_point2;
        point_end = temporary_point;
    }
    if(point_start.x() > point_end.x() && point_start.y() < point_end.y())
    {
        int temporary = point_start.x();
        int temporary_2 = point_end.x();
        QPoint point_start_temporary (temporary_2, point_start.y());
        QPoint point_end_temporary (temporary, point_end.y());
        point_start = point_start_temporary;
        point_end = point_end_temporary;
    }
    if(point_start.x() < point_end.x() && point_start.y() > point_end.y())
    {
        int temporary = point_start.y();
        int temporary_2 = point_end.y();
        QPoint point_start_temporary (point_start.x(), temporary_2);
        QPoint point_end_temporary (point_end.x(), temporary);
        point_start = point_start_temporary;
        point_end = point_end_temporary;
    }

    counter_2 = 0;
    counter = 0;
    is_trackingbox_selected = true;
}

void MainWindow::on_BOOSTING_clicked()
{
    qDebug() << "Tracker type has set to BOOSTING";
    mode = 1;
    counter = 0;
}

void MainWindow::on_MIL_clicked()
{
    qDebug() << "Tracker type has set to MIL";
    counter = 0;
    mode = 2;
}

void MainWindow::on_KCF_clicked()
{
    qDebug() << "Tracker type has set to KCF";
    mode = 3;
    counter = 0;
}

void MainWindow::on_TLD_clicked()
{
    qDebug() << "Tracker type has set to TLD";
    mode = 4;
    counter = 0;
}

void MainWindow::on_MEDIANFLOW_clicked()
{
    qDebug() << "Tracker type has set to MEDIANFLOW";
    mode = 5;
    counter = 0;
}

void MainWindow::on_MOSSE_clicked()
{
    qDebug() << "Tracker type has set to MOSSE";
    mode = 6;
    counter = 0;
}

void MainWindow::on_CSRT_clicked()
{
    qDebug() << "Tracker type has set to CSRT";
    mode = 7;
    counter = 0;
}

void MainWindow::on_Exit_Tracking_clicked()
{
    counter_2 = 0;
    is_trackingbox_selected = false;
    if (tracker != NULL)
    {
        tracker -> clear();
        tracker = NULL;
    }
}

void MainWindow::button_color()
{
    if (mode == 1)
    {
        ui->BOOSTING->setStyleSheet("QPushButton{ background-color: yellow }");
        ui->MIL->setStyleSheet("QPushButton{ background-color: white }");
        ui->KCF->setStyleSheet("QPushButton{ background-color: white }");
        ui->TLD->setStyleSheet("QPushButton{ background-color: white }");
        ui->MEDIANFLOW->setStyleSheet("QPushButton{ background-color: white }");
        ui->MOSSE->setStyleSheet("QPushButton{ background-color: white }");
        ui->CSRT->setStyleSheet("QPushButton{ background-color: white }");
    }
    if (mode == 2)
    {
        ui->BOOSTING->setStyleSheet("QPushButton{ background-color: white }");
        ui->MIL->setStyleSheet("QPushButton{ background-color: yellow }");
        ui->KCF->setStyleSheet("QPushButton{ background-color: white }");
        ui->TLD->setStyleSheet("QPushButton{ background-color: white }");
        ui->MEDIANFLOW->setStyleSheet("QPushButton{ background-color: white }");
        ui->MOSSE->setStyleSheet("QPushButton{ background-color: white }");
        ui->CSRT->setStyleSheet("QPushButton{ background-color: white }");
    }
    if (mode == 3)
    {
        ui->BOOSTING->setStyleSheet("QPushButton{ background-color: white }");
        ui->MIL->setStyleSheet("QPushButton{ background-color: white }");
        ui->KCF->setStyleSheet("QPushButton{ background-color: yellow }");
        ui->TLD->setStyleSheet("QPushButton{ background-color: white }");
        ui->MEDIANFLOW->setStyleSheet("QPushButton{ background-color: white }");
        ui->MOSSE->setStyleSheet("QPushButton{ background-color: white }");
        ui->CSRT->setStyleSheet("QPushButton{ background-color: white }");
    }
    if (mode == 4)
    {
        ui->BOOSTING->setStyleSheet("QPushButton{ background-color: white }");
        ui->MIL->setStyleSheet("QPushButton{ background-color: white }");
        ui->KCF->setStyleSheet("QPushButton{ background-color: white }");
        ui->TLD->setStyleSheet("QPushButton{ background-color: yellow }");
        ui->MEDIANFLOW->setStyleSheet("QPushButton{ background-color: white }");
        ui->MOSSE->setStyleSheet("QPushButton{ background-color: white }");
        ui->CSRT->setStyleSheet("QPushButton{ background-color: white }");
    }
    if (mode == 5)
    {
        ui->BOOSTING->setStyleSheet("QPushButton{ background-color: white }");
        ui->MIL->setStyleSheet("QPushButton{ background-color: white }");
        ui->KCF->setStyleSheet("QPushButton{ background-color: white }");
        ui->TLD->setStyleSheet("QPushButton{ background-color: white }");
        ui->MEDIANFLOW->setStyleSheet("QPushButton{ background-color: yellow }");
        ui->MOSSE->setStyleSheet("QPushButton{ background-color: white }");
        ui->CSRT->setStyleSheet("QPushButton{ background-color: white }");
    }
    if (mode == 6)
    {
        ui->BOOSTING->setStyleSheet("QPushButton{ background-color: white }");
        ui->MIL->setStyleSheet("QPushButton{ background-color: white }");
        ui->KCF->setStyleSheet("QPushButton{ background-color: white }");
        ui->TLD->setStyleSheet("QPushButton{ background-color: white }");
        ui->MEDIANFLOW->setStyleSheet("QPushButton{ background-color: white }");
        ui->MOSSE->setStyleSheet("QPushButton{ background-color: yellow }");
        ui->CSRT->setStyleSheet("QPushButton{ background-color: white }");
    }
    if (mode == 7)
    {
        ui->BOOSTING->setStyleSheet("QPushButton{ background-color: white }");
        ui->MIL->setStyleSheet("QPushButton{ background-color: white }");
        ui->KCF->setStyleSheet("QPushButton{ background-color: white }");
        ui->TLD->setStyleSheet("QPushButton{ background-color: white }");
        ui->MEDIANFLOW->setStyleSheet("QPushButton{ background-color: white }");
        ui->MOSSE->setStyleSheet("QPushButton{ background-color: white }");
        ui->CSRT->setStyleSheet("QPushButton{ background-color: yellow }");
    }

}
