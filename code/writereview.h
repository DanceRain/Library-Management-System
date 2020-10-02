#ifndef WRITEREVIEW_H
#define WRITEREVIEW_H

#include <QDialog>

namespace Ui {
class WriteReview;
}
class QTcpSocket;

class WriteReview : public QDialog
{
    Q_OBJECT

public:
    explicit WriteReview(QWidget *parent = nullptr, const QString& id = nullptr, const QString& isbn = nullptr, const QString& bName = nullptr);
    QPushButton* submitButton();
    ~WriteReview();
private slots:
    void closeConnection();
    void connectToServer();
    void connectionClosedByServer();

    void sendReview();
    void getResponse();

private:
    Ui::WriteReview *ui;
    QString userId;
    QString bookIsbn;
    QString bookName;
    QTcpSocket* tcpSocket;
    quint16 nextBlockSize;
};

#endif // WRITEREVIEW_H
