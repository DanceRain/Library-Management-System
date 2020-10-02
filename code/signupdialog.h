#ifndef SIGNUPDIALOG_H
#define SIGNUPDIALOG_H

#include <QDialog>


namespace Ui {
class SignUpDialog;
}
class QTcpSocket;

class SignUpDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SignUpDialog(QWidget *parent = nullptr);
    QPushButton* backButton();

    ~SignUpDialog();

private slots:
    void on_lineEdit_confrimPassword_textChanged(const QString &arg1);
    void on_lineEdit_password_textChanged(const QString &arg1);

    void closeConnection();
    void connectionClosedByServer();
    void connectToServer();
    void getSignUpMessage();
    void sendRegisterRequest();

private:
    Ui::SignUpDialog *ui;
    QTcpSocket* tcpSocket;
    quint16 nextBlockSize;
    bool userFlag;
};

#endif // SIGNUPDIALOG_H
