#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "databasecontrol.cpp"
#include "signupdialog.h"
#include "admindialog.h"
#include <QMessageBox>
#include "UserWindow.h"



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    userWindow(nullptr),
    signUpDialog(nullptr),
    adminDialog(nullptr),
    userFlag(true)
{
    ui->setupUi(this);
    ui->label_tip->hide();


    connect(ui->pushButton_signIn, SIGNAL(clicked()), this, SLOT(connectToServer()));
    connect(ui->pushButton_adminSignIn, SIGNAL(clicked()), this, SLOT(connectToServer()));
    connect(&tcpSocket, SIGNAL(connected()), this, SLOT(sendUserLoginRequest()));
    connect(&tcpSocket, SIGNAL(connected()), this, SLOT(sendAdminLoginRequest()));
    connect(&tcpSocket, SIGNAL(readyRead()), this, SLOT(getLoginMessage()));
    connect(&tcpSocket, SIGNAL(disconnected()), this, SLOT(connectionClosedByServer()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

//连接Tcp
void MainWindow::connectToServer()
{
    tcpSocket.connectToHost("175.24.120.185", 8888);
    nextBlockSize = 0;

    if(static_cast<QPushButton*>(sender()) == ui->pushButton_adminSignIn)
    {
        userFlag = false;
    }
    else
    {
        userFlag = true;
    }
}

//发送普通用户登录请求
void MainWindow::sendUserLoginRequest()
{
    if(userFlag != true)
    {
        return;
    }
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);

    out.setVersion(QDataStream::Qt_5_13);
    out << quint16(0) << quint8(1) << ui->lineEdit_userName->text() << ui->lineEdit_passWord->text() << QString("user");
    out.device()->seek(0);
    out << quint16(static_cast<quint16>(block.size()) - sizeof(quint16));

    tcpSocket.write(block);
}

//发送管理员登录请求
void MainWindow::sendAdminLoginRequest()
{

    if(userFlag != false)
    {
        return;
    }

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);

    out.setVersion(QDataStream::Qt_5_13);
    out << quint16(0) << quint8(1) << ui->lineEdit_userName->text() << ui->lineEdit_passWord->text() << QString("admin");
    out.device()->seek(0);
    out << quint16(static_cast<quint16>(block.size()) - sizeof(quint16));

    tcpSocket.write(block);
        qDebug() << userFlag;
}

//请求信息确认并登录
void MainWindow::getLoginMessage()
{
    QDataStream in(&tcpSocket);
    in.setVersion(QDataStream::Qt_5_13);
    forever{
        if(nextBlockSize == 0)
        {
            if(tcpSocket.bytesAvailable() < sizeof(quint16))
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
        if(isOk)
        {
            if(userFlag == true)
            {
                hide();
                userWindow = new UserWindow(nullptr, ui->lineEdit_userName->text());
                userWindow->setAttribute(Qt::WA_DeleteOnClose);
                userWindow->show();
                connect(userWindow->returnQuit(), SIGNAL(clicked()), this, SLOT(returnSignInDialog()));
            }
            else
            {
                hide();
                adminDialog = new AdminDialog(nullptr, ui->lineEdit_userName->text());
                adminDialog->setAttribute(Qt::WA_DeleteOnClose);
                adminDialog->show();
                connect(adminDialog->returnQuit(), SIGNAL(clicked()), this, SLOT(returnSignInDialog()));
            }
        }
        else
        {
            ui->label_tip->show();
        }

        nextBlockSize = 0;
    }

}

void MainWindow::closeConnection()
{
    tcpSocket.close();
}

void MainWindow::connectionClosedByServer()
{
    if(nextBlockSize != 0xffff)
    {
         QMessageBox::information(this, "Error", "Connection Close by server");
    }
    closeConnection();
}

void MainWindow::returnSignInDialog()
{
    if(signUpDialog != nullptr)
    {
        signUpDialog->close();
        signUpDialog = nullptr;
    }
    if(userWindow != nullptr)
    {
        userWindow->close();
        userWindow = nullptr;
    }
    if(adminDialog != nullptr)
    {
        adminDialog->close();
        adminDialog = nullptr;
    }

    show();
}

//进入注册窗口
void MainWindow::on_pushButton_signUp_clicked()
{
    hide();
    signUpDialog = new SignUpDialog(this);
    signUpDialog->setAttribute(Qt::WA_DeleteOnClose);  //使用户个人界面close时删除而不是隐藏
    signUpDialog->show();
    connect(signUpDialog->backButton(), SIGNAL(clicked()), this, SLOT(returnSignInDialog()));
}

