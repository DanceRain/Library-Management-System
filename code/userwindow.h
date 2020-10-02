#ifndef SECDIALOG_H
#define SECDIALOG_H

#include <QMainWindow>
#include <QModelIndex>

class QTcpSocket;
class QPushButton;
class BookModel;

namespace Ui {
class UserWindow;
}

class QSortFilterProxyModel;

class UserWindow : public QMainWindow
{
    Q_OBJECT

public:
    enum Operation{GET, READUSERREVIEW, READBOOKREVIW, WRITEREVIEW, BORROWBOOK, RETURNBOOK, ADDFRIEND, SENDMESSAGE, UPDATEBOOKMAP};
    explicit UserWindow(QWidget *parent = nullptr, const QString& name = nullptr);
    QPushButton* returnQuit();
    ~UserWindow();

private slots:
    void closeConnection();
    void connectToServer();
    void connectionClosedByServer();

    void userInforRequest(); //用户信息初始请求

    void readUserReview();//打开用户的书籍评论
    void readBookReview();//打开书籍评论

    void borrowBook();
    void returnBook();

    void slotTreeMenu(const QPoint &pos);//书库右键菜单槽

    void getResponse();//对套接字接收到的信息过滤

    void filterStatus(const QString& arg);
    void filterChanged(const QString &text);

    void updateBookMap();
private:
    void setModelAndReview(QDataStream& in, int times);
    void setBookDialog(QDataStream& in);
    void setBookStatus(QDataStream& in);
    void setBorrowedBook(QDataStream& in);
    void connectForBookMap();


    Ui::UserWindow *ui;
    QTcpSocket* tcpSocket;
    QString userName;
    quint16 nextBlockSize;
    Operation operationFlag;
    BookModel* bookModel;
    QMap<QString, QString>* userReview;
    QStringList* borrowBookList;
    QSortFilterProxyModel* modelProxyStatus;
    QSortFilterProxyModel* modelProxyIsbn;
    QModelIndex sourceIndex;
};

#endif // SECDIALOG_H
