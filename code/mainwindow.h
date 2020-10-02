#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>

class UserDialog;
class SignUpDialog;
class AdminDialog;
class DatabaseControl;
class UserWindow;


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void returnSignInDialog();
    void on_pushButton_signUp_clicked();

    void connectToServer();
    void getLoginMessage();
    void closeConnection();
    void connectionClosedByServer();
    void sendAdminLoginRequest();
    void sendUserLoginRequest();
private:
    Ui::MainWindow *ui;
    UserWindow*  userWindow;
    SignUpDialog* signUpDialog;
    AdminDialog* adminDialog;
    QTcpSocket tcpSocket;
    quint16 nextBlockSize;
    bool userFlag;
};

#endif // MAINWINDOW_H
