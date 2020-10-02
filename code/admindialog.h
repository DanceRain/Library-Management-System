#ifndef ADMINDIALOG_H
#define ADMINDIALOG_H

#include <QDialog>

class QTcpSocket;
class BookModel;
class QSortFilterProxyModel;

namespace Ui {
class AdminDialog;
}


class AdminDialog : public QDialog
{
    Q_OBJECT

public:
    enum Operation{GET, ADD, DELETE, MODIFY};
    explicit AdminDialog(QWidget *parent = nullptr, const QString& userName = "");
    QPushButton* returnQuit();
    ~AdminDialog();

private slots:
    void connectToServer();
    void getRequestResult();
    void closeConnection();
    void connectionClosedByServer();

    void filterChanged(const QString &text);

    void deleteBook();
    void addBook();
    void modifyBook();

    void sendLibraryRequest();


private:
    Ui::AdminDialog *ui;
    QTcpSocket* tcpSocket;
    quint16 nextBlockSize;
    Operation operateFlag;
    BookModel* bookModel;
    QSortFilterProxyModel *modelProxy;
    QString userName;
};

#endif // ADMINDIALOG_H
