//Yasar Utku Alcalar
#include "login_track.h"
#include "ui_login_track.h"
#include <qstring.h>
#include <stdio.h>
#include <QString>
#include "mainwindow.h"

Login::Login(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Login)
{
    ui->setupUi(this);
}

Login::~Login()
{
    delete ui;
}

void Login::on_btn_play_clicked()
{
    close();
}

QString Login::getRtspURL()
{
    return ui->lineEdit->text();;
}

QString Login::getMode()
{
    return ui->mode->text();;
}

void Login::on_btn_cancel_clicked()
{
    exit(0);
}
