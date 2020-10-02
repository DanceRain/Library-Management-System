#include "writereview.h"
#include "ui_writereview.h"
#include <QTcpSocket>
#include <QMessageBox>

WriteReview::WriteReview(QWidget *parent, const QString& id, const QString& isbn, const QString& bName) :
    QDialog(parent),
    ui(new Ui::WriteReview),
    userId(id),
    bookIsbn(isbn),
    bookName(bName),
    tcpSocket(new QTcpSocket())
{
    ui->setupUi(this);
    ui->label_title->setText(bookName);

    connect(ui->pushButton_submit, &QPushButton::clicked, this, &WriteReview::connectToServer);
    connect(tcpSocket, &QTcpSocket::connected, this, &WriteReview::sendReview);
    connect(tcpSocket, &QTcpSocket::readyRead, this, &WriteReview::getResponse);
}

void WriteReview::getResponse()
{
    QDataStream in(tcpSocket);
    in.setVersion(QDataStream::Qt_5_13);

    bool check;
    forever{
        if(nextBlockSize == 0)
        {
            in >> nextBlockSize;
            if(tcpSocket->bytesAvailable() < sizeof(quint16))
            {
                break;
            }
        }

        if(nextBlockSize == 0xffff)
        {
            closeConnection();

            break;
        }
        in >> check;
        nextBlockSize = 0;
    }
    if(check == true)
    {
        QMessageBox::information(this, QString::fromLocal8Bit("评论成功"), QString::fromLocal8Bit("你的评论已经成功提交！"));
    }
}

QPushButton* WriteReview::submitButton()
{
    return ui->pushButton_submit;
}

void WriteReview::sendReview()
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_13);

    QString reviewContent = ui->textEdit_review->toPlainText();
    qDebug() << reviewContent.size();

    qDebug() << userId << bookIsbn << reviewContent;
    out << quint16(0) << quint8(10) << userId << bookIsbn << reviewContent;
    out.device()->seek(0);
    out << quint16(static_cast<quint16>(block.size()) - sizeof(quint16));
    tcpSocket->write(block);
}

void WriteReview::closeConnection()
{
    tcpSocket->close();
    qDebug() << QString::fromLocal8Bit("已关闭Close");
}

void WriteReview::connectToServer()
{
    tcpSocket->connectToHost("175.24.120.185", 8888);
    nextBlockSize = 0;
}

void WriteReview::connectionClosedByServer()
{
    if(nextBlockSize != 0xffff)
    {
         QMessageBox::information(this, "Error", "Connection Close by server");
    }
    closeConnection();
}

WriteReview::~WriteReview()
{
    delete ui;
}
