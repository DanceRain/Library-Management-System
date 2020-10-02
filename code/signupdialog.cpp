#include "signupdialog.h"
#include "ui_signupdialog.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QTcpSocket>

SignUpDialog::SignUpDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SignUpDialog),
    tcpSocket(new QTcpSocket(this)),
    userFlag(true)
{
    ui->setupUi(this);
    ui->passwordDifferent->hide();
    ui->userNameRepeat->hide();
    ui->label_tipExist->hide();

    connect(ui->pushButton_complete, SIGNAL(clicked()), this, SLOT(connectToServer()));
    connect(ui->pushButton_adminComplete, SIGNAL(clicked()), this, SLOT(connectToServer()));
    connect(tcpSocket, SIGNAL(connected()), this, SLOT(sendRegisterRequest()));
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(getSignUpMessage()));
    connect(tcpSocket, SIGNAL(disconnected()), this, SLOT(connectionClosedByServer()));
}

//连接网络
void SignUpDialog::connectToServer()
{
    tcpSocket->connectToHost("175.24.120.185", 8888);
    nextBlockSize = 0;
    if(static_cast<QPushButton*>(sender()) == ui->pushButton_adminComplete)
    {
        userFlag = false;
    }
    else
    {
        userFlag = true;
    }
}

//发送注册请求
void SignUpDialog::sendRegisterRequest()
{
    if(userFlag == true)
    {
        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);

        out.setVersion(QDataStream::Qt_5_13);
        out << quint16(0) << quint8(2) << ui->lineEdit_userName->text() << ui->lineEdit_password->text() << QString("user");
        out.device()->seek(0);
        out << quint16(static_cast<quint16>(block.size()) - sizeof(quint16));

        tcpSocket->write(block);
    }
    else
    {
        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);

        out.setVersion(QDataStream::Qt_5_13);
        out << quint16(0) << quint8(2) << ui->lineEdit_userName->text() << ui->lineEdit_password->text() << QString("admin");
        out.device()->seek(0);
        out << quint16(static_cast<quint16>(block.size()) - sizeof(quint16));

        tcpSocket->write(block);
    }
}

//获取注册信息并登录
void SignUpDialog::getSignUpMessage()
{
    QDataStream in(tcpSocket);
    in.setVersion(QDataStream::Qt_5_13);
    forever{
        if(nextBlockSize == 0)
        {
            if(tcpSocket->bytesAvailable() < sizeof(quint16))
            {
                break;
            }
            in >> nextBlockSize;

        }

        if(nextBlockSize == 0xffff)
        {
            closeConnection();
            break;
        }

        bool isOk;
        in >> isOk;
        if(!isOk)
        {
            ui->label_tipExist->show();
        }
        else
        {
            emit ui->pushButton_back->clicked();
        }
        nextBlockSize = 0;
    }
}

void SignUpDialog::closeConnection()
{
    tcpSocket->close();
}

void SignUpDialog::connectionClosedByServer()
{
    if(nextBlockSize != 0xffff)
    {
         QMessageBox::information(this, "Error", "Connection Close by server");
    }
    closeConnection();
}

void SignUpDialog::on_lineEdit_confrimPassword_textChanged(const QString &arg1)
{
    if(arg1 != ui->lineEdit_password->text())
    {
         ui->passwordDifferent->show();
    }
    else
    {
        ui->passwordDifferent->hide();
    }
}

void SignUpDialog::on_lineEdit_password_textChanged(const QString &arg1)
{
    if(arg1 != ui->lineEdit_confrimPassword->text())
    {
         ui->passwordDifferent->show();
    }
    else
    {
        ui->passwordDifferent->hide();
    }
}

QPushButton* SignUpDialog::backButton()
{
    return ui->pushButton_back;
}

SignUpDialog::~SignUpDialog()
{
    delete ui;
}
