#include "admindialog.h"
#include "ui_admindialog.h"
#include <QMessageBox>
#include <QTcpSocket>
#include "bookmodel.h"
#include <QSortFilterProxyModel>
#include <QItemSelection>

AdminDialog::AdminDialog(QWidget *parent, const QString& str) :
    QDialog(parent),
    ui(new Ui::AdminDialog),
    tcpSocket(new QTcpSocket(this)),
    operateFlag(GET),
    bookModel(nullptr),
    modelProxy(new QSortFilterProxyModel(this)),
    userName(str)
{

    ui->setupUi(this);
    ui->label_adminName->setText(userName);

    ui->tableView_library->setSelectionBehavior(QAbstractItemView::SelectRows);

    connect(ui->pushButton_importLibrary, SIGNAL(clicked()), this, SLOT(connectToServer()));;
    connect(ui->pushButton_addBook, SIGNAL(clicked()), this, SLOT(connectToServer()));
    connect(ui->pushButton_deleteRow, SIGNAL(clicked()), this, SLOT(connectToServer()));
    connect(ui->pushButton_modify, &QPushButton::clicked, this, &AdminDialog::connectToServer);


    connect(tcpSocket, &QTcpSocket::disconnected, this, &AdminDialog::connectionClosedByServer);
    connect(tcpSocket, &QTcpSocket::readyRead, this, &AdminDialog::getRequestResult);

    connect(tcpSocket, &QTcpSocket::connected, this, &AdminDialog::sendLibraryRequest);
    connect(tcpSocket, SIGNAL(connected()), this, SLOT(deleteBook()));
    connect(tcpSocket, SIGNAL(connected()), this, SLOT(addBook()));
    connect(tcpSocket, SIGNAL(connected()), this, SLOT(modifyBook()));

    connect(ui->lineEdit_isbn, SIGNAL(textChanged(QString)), this, SLOT(filterChanged(QString)));
}

AdminDialog::~AdminDialog()
{
    delete bookModel;
    delete ui;
}

QPushButton* AdminDialog::returnQuit()
{
    return ui->pushButton_quit;
}

//连接服务器并确认操作
void AdminDialog::connectToServer()
{
    tcpSocket->connectToHost("175.24.120.185", 8888);
    nextBlockSize = 0;

    if(static_cast<QPushButton*>(sender()) == ui->pushButton_importLibrary)
    {
        operateFlag = GET;
    }
    else if(static_cast<QPushButton*>(sender()) == ui->pushButton_addBook)
    {
        operateFlag = ADD;
    }
    else if(static_cast<QPushButton*>(sender()) == ui->pushButton_deleteRow)
    {
        if(bookModel == nullptr)
        {
            nextBlockSize = 0xffff;
            connectionClosedByServer();
            return;
        }
        operateFlag = DELETE;
    }
    else
    {
        operateFlag = MODIFY;
    }
}

//获取返回信息。后续步骤改进方案：socket返回帧添加帧标识
void AdminDialog::getRequestResult()
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
            qDebug() << "test";
            closeConnection();
            break;
        }
        switch(operateFlag)
        {
            case ADD:
            {
                bool judge;
                in >> judge;
            }break;

            case DELETE:
            {
                bool judge;
                in >> judge;
            }break;

            case MODIFY:
            {
                bool judge;
                in >> judge;
            }break;

            case GET:
            {
                QVector<QStringList> data;
                in >> data;

                bookModel = new BookModel(this);
                modelProxy->setSourceModel(bookModel);
                modelProxy->setFilterKeyColumn(0);
                bookModel->setBookData(data);
                ui->tableView_library->setModel(modelProxy);
                ui->tableView_library->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
            }break;
        }
        nextBlockSize = 0;
    }
}

//请求图书信息
void AdminDialog::sendLibraryRequest()
{
    if(operateFlag != GET)
    {
        return;
    }
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);

    out.setVersion(QDataStream::Qt_5_13);
    out << quint16(0) << quint8(6);
    out.device()->seek(0);
    out << quint16(static_cast<quint16>(block.size()) - sizeof(quint16));

    tcpSocket->write(block);
}

//增加图书
void AdminDialog::addBook()
{
    if(operateFlag != ADD)
    {
        return;
    }
    if(ui->lineEdit_isbn->text() == "" || ui->lineEdit_name->text() == ""
            || ui->lineEdit_status->text() == "" || ui->lineEdit_location->text() == "")
    {
        nextBlockSize = 0xffff;
        connectionClosedByServer();
        return;
    }

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);

    QStringList bookInfor;
    bookInfor.append(ui->lineEdit_isbn->text());
    bookInfor.append(ui->lineEdit_name->text());
    bookInfor.append(ui->lineEdit_status->text());
    bookInfor.append(ui->lineEdit_location->text());

    out.setVersion(QDataStream::Qt_5_13);
    out << quint16(0) << quint8(3) << bookInfor;
    out.device()->seek(0);
    out << quint16(static_cast<quint16>(block.size()) - sizeof(quint16));
    tcpSocket->write(block);

    bookModel->insertBook(bookInfor);
}

//删除图书
void AdminDialog::deleteBook()
{
    if(operateFlag != DELETE)
    {
        return;
    }
    QItemSelectionModel* selections = ui->tableView_library->selectionModel();
    QModelIndexList selected = selections->selectedRows();
    QStringList isbnList;
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);

    QMap<int, int> rowMap;
    for(QModelIndex index : selected)
    {
        isbnList.append((bookModel->getBookData())[index.row()][0]);
        rowMap.insert(index.row(), 0);
    }

    out.setVersion(QDataStream::Qt_5_13);
    out << quint16(0) << quint8(4) << isbnList;
    out.device()->seek(0);
    out << quint16(static_cast<quint16>(block.size()) - sizeof(quint16));
    tcpSocket->write(block);

    //由于在选中的位置是不分先后次序的，所以其selected也是一个无需的List
    //而当删除前行时，后行的位置会发生变化。所以必须先从行号最大的开始删除
    QMapIterator<int, int> rowMapIterator(rowMap);
    rowMapIterator.toBack();
    while(rowMapIterator.hasPrevious())
    {
        rowMapIterator.previous();
        bookModel->removeRows(rowMapIterator.key(), 1);
    }

}

//提交修改
void AdminDialog::modifyBook()
{
    if(operateFlag != MODIFY)
    {
        return;
    }
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);

    out.setVersion(QDataStream::Qt_5_13);
    out << quint16(0) << quint8(5) << bookModel->getLatestModification();
    out.device()->seek(0);
    out << quint16(static_cast<quint16>(block.size()) - sizeof(quint16));

    tcpSocket->write(block);
}

//查询具体书籍信息
void AdminDialog::filterChanged(const QString &text)
{
    QRegExp::PatternSyntax syntax = QRegExp::PatternSyntax(QRegExp::RegExp);
    QRegExp regExp(text, Qt::CaseInsensitive, syntax);
    modelProxy->setFilterRegExp(regExp);
}

void AdminDialog::closeConnection()
{
    tcpSocket->close();
    qDebug() << QString::fromLocal8Bit("已关闭Close");
}

//该函数提示用户TCP连接已经关闭但数据接收可能不完整。
void AdminDialog::connectionClosedByServer()
{
    if(nextBlockSize != 0xffff)
    {
         QMessageBox::information(this, "Error", "Connection Close by server");
    }
    qDebug() << QString::fromLocal8Bit("已关闭Server");
    closeConnection();
}

