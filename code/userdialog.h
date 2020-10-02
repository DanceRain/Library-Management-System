#ifndef SECDIALOG_H
#define SECDIALOG_H

#include <QDialog>
#include <QMainWindow>

class DatabaseControl;
class UserInformation;
class QSqlTableModel;
class QTcpSocket;

namespace Ui {
class UserDialog;
}

class UserDialog : public QMainWindow
{
    Q_OBJECT

public:
    explicit UserDialog(QString name, QWidget *parent = nullptr, QTcpSocket* socket = nullptr);
    QPushButton* returnQuit();
    ~UserDialog();

private slots:
    void on_pushButton_clicked();
    void slotTreeMenu(const QPoint &pos);
    void slotReadReview();
    void slotBorrowBook();

private:
    Ui::UserDialog *ui;
    QTcpSocket* tcpSocket;
    QString userName;
};

#endif // SECDIALOG_H
