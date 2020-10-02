#include "userwindow.h"
#include "ui_userwindow.h"
#include "reviewdialog.h"
#include <QListWidgetItem>
#include <QTextBrowser>
#include <QMenu>
#include <QTcpSocket>
#include <QMessageBox>
#include "bookmodel.h"
#include <QStatusBar>
#include "writereview.h"
#include <QSortFilterProxyModel>

UserWindow::UserWindow(QWidget *parent, const QString& name) :
    QMainWindow(parent),
    ui(new Ui::UserWindow),
    tcpSocket(new QTcpSocket(this)),
    userName(name),
    operationFlag(GET),
    bookModel(new BookModel(this)),
    userReview(nullptr),
    borrowBookList(nullptr),
    modelProxyStatus(new QSortFilterProxyModel(this)),
    modelProxyIsbn(new QSortFilterProxyModel(this))
{
    ui->setupUi(this);
    statusBar();
    ui->label_userNameContent->setText(userName);

    bookModel->setReadOnly(); //将model设置为只读

    connect(tcpSocket, &QTcpSocket::connected, this, &UserWindow::userInforRequest);   //用户初始信息请求
    connect(tcpSocket, &QTcpSocket::connected, this, &UserWindow::readBookReview);   //用户查看书评
    connect(tcpSocket, &QTcpSocket::connected, this, &UserWindow::updateBookMap);
    connect(tcpSocket, &QTcpSocket::connected, this, &UserWindow::borrowBook);
    connect(tcpSocket, &QTcpSocket::connected, this, &UserWindow::returnBook);

    connect(ui->pushButton_readMyReview, &QPushButton::clicked, this, &UserWindow::connectForBookMap); //查看自身评论

    connect(tcpSocket, &QTcpSocket::readyRead, this, &UserWindow::getResponse);

    connect(ui->comboBox_bookStatus, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(filterStatus(const QString&)));
    connect(ui->lineEdit_fliter, SIGNAL(textChanged(QString)), this, SLOT(filterChanged(QString)));
    ui->treeView_bookStock->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(ui->treeView_bookStock, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(slotTreeMenu(const QPoint&)));
    connectToServer();
}

void UserWindow::connectToServer()
{
    tcpSocket->connectToHost("175.24.120.185", 8888);
    nextBlockSize = 0;
}

void UserWindow::userInforRequest()
{
    if(operationFlag != GET)
    {
        return;
    }
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);

    out.setVersion(QDataStream::Qt_5_13);
    out << quint16(0) << quint8(7) << ui->label_userNameContent->text() << userName;
    out.device()->seek(0);
    out << quint16(static_cast<quint16>(block.size()) - sizeof(quint16));
    tcpSocket->write(block);
}

//对套接字接收到的信息过滤
void UserWindow::getResponse()
{
    QDataStream in(tcpSocket);
    in.setVersion(QDataStream::Qt_5_13);
    int loopTimes = 0;

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
        switch(operationFlag)
        {
            case GET:
            {
                setModelAndReview(in, loopTimes);
            }break;
            case READBOOKREVIW:
            {
                setBookDialog(in);
            }break;
            case UPDATEBOOKMAP:
            {
                setModelAndReview(in, 1);
            }break;
            case RETURNBOOK:
            {
                setBorrowedBook(in);
            }break;
            case BORROWBOOK:
            {
                setBookStatus(in);
            }break;
        }
        ++loopTimes;
        nextBlockSize = 0;
    }
}

//初始用户信息包含用户的书籍评价
void UserWindow::setModelAndReview(QDataStream& in, int times)
{
    if(times == 0)
    {
        QVector<QStringList> data;
        in >> data;

        modelProxyStatus->setSourceModel(modelProxyIsbn);
        modelProxyStatus->setFilterKeyColumn(2);

        modelProxyIsbn->setSourceModel(bookModel);
        modelProxyIsbn->setFilterKeyColumn(0);

        bookModel->setBookData(data);
        ui->treeView_bookStock->setModel(modelProxyStatus);
        ui->treeView_bookStock->header()->setDefaultAlignment(Qt::AlignCenter);//设置列标题居中
    }
    else if(times == 1)
    {
        qDebug() << "this is setModelReview";
        QMap<QString, QString> review;
        in >> review;
        userReview = new QMap<QString, QString>(review);
        ui->label_bookReviewContent->setText(QString::number(userReview->size(), 10));
        if(operationFlag == UPDATEBOOKMAP)
        {
            readUserReview();
        }
    }
    else
    {
        QStringList borrowedBook;
        in >> borrowedBook;
        borrowBookList =  new QStringList(borrowedBook);
    }

}

//打开每本书的书籍评价
void UserWindow::setBookDialog(QDataStream& in)
{
    QMap<QString, QString> review;
    in >> review;

    ReviewDialog* reviewDialog = new ReviewDialog(this);
    reviewDialog->setAttribute(Qt::WA_DeleteOnClose);
    reviewDialog->setTitle(bookModel->data(sourceIndex.sibling(sourceIndex.row(), 1), 0).toString());

    for(QMap<QString, QString>::const_iterator it = review.cbegin(); it != review.cend(); ++it)
    {
        QListWidgetItem *itme = new QListWidgetItem(reviewDialog->listWidget());
        QSize size = itme->sizeHint();
        itme->setSizeHint(QSize(size.width(), 100));
        QWidget* widget = new QWidget(reviewDialog);
        QVBoxLayout* layout = new QVBoxLayout;
        QTextBrowser* textBrower = new QTextBrowser(widget);
        QLabel* book = new QLabel(it.key(), widget);
        textBrower->setText(it.value());
        layout->addWidget(book);
        layout->addWidget(textBrower);
        widget->setLayout(layout);

        reviewDialog->listWidget()->setItemWidget(itme, widget);
    }
    //为每一个评论建立QListWidgetItem并一个Widget为其ItemWidget

    reviewDialog->show();
}

//设置借书状态
void UserWindow::setBookStatus(QDataStream& in)
{
    bool status;
    in >> status;
    if(status)
    {
        borrowBookList->append(bookModel->data(sourceIndex.sibling(sourceIndex.row(), 0), 0).toString());
        bookModel->setWriteOnly();
        bookModel->setData(sourceIndex.sibling(sourceIndex.row(), 2), QString::fromLocal8Bit("借出"), Qt::EditRole);
        bookModel->setReadOnly();
    }
}

//设置还书状态
void UserWindow::setBorrowedBook(QDataStream& in)
{
    bool isOk;
    in >> isOk;
    qDebug() << isOk;
    if(isOk)
    {
        borrowBookList->removeOne(bookModel->data(sourceIndex, 0).toString());
        bookModel->setWriteOnly();
        bookModel->setData(sourceIndex.sibling(sourceIndex.row(), 2), QString::fromLocal8Bit("未借出"), Qt::EditRole);
        bookModel->setReadOnly();
    }
}

//打开Dialog查看用户的书籍评价
void UserWindow::readUserReview()
{
    ReviewDialog* reviewDialog = new ReviewDialog(this);
    reviewDialog->setAttribute(Qt::WA_DeleteOnClose);
    reviewDialog->setTitle(userName + QString::fromLocal8Bit("的书评"));

    for(QMap<QString, QString>::const_iterator it = userReview->cbegin(); it != userReview->cend(); ++it)
    {
        QListWidgetItem *itme = new QListWidgetItem(reviewDialog->listWidget());
        QSize size = itme->sizeHint();
        itme->setSizeHint(QSize(size.width(), 100));
        QWidget* widget = new QWidget(reviewDialog);
        QVBoxLayout* layout = new QVBoxLayout;
        QTextBrowser* textBrower = new QTextBrowser(widget);
        QLabel* book = new QLabel(it.key(), widget);
        textBrower->setText(it.value());
        layout->addWidget(book);
        layout->addWidget(textBrower);
        widget->setLayout(layout);

        reviewDialog->listWidget()->setItemWidget(itme, widget);
    }
    //为每一个评论建立QListWidgetItem并一个Widget为其ItemWidget

    reviewDialog->show();
}

void UserWindow::borrowBook()
{
    if(operationFlag != BORROWBOOK)
    {
        return;
    }
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);

    out.setVersion(QDataStream::Qt_5_13);
    qDebug() << (bookModel->data(sourceIndex, 0)).toString() << "test";
    out << quint16(0) << quint8(9) << (bookModel->data(sourceIndex, 0)).toString() << userName;
    out.device()->seek(0);
    out << quint16(static_cast<quint16>(block.size()) - sizeof(quint16));

    tcpSocket->write(block);
}

void UserWindow::returnBook()
{
    if(operationFlag != RETURNBOOK)
    {
        return;
    }
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);

    out.setVersion(QDataStream::Qt_5_13);

    out << quint16(0) << quint8(13) << (bookModel->data(sourceIndex, 0)).toString() << userName;
    out.device()->seek(0);
    out << quint16(static_cast<quint16>(block.size()) - sizeof(quint16));

    tcpSocket->write(block);
}

void UserWindow::readBookReview()
{
    if(operationFlag != READBOOKREVIW)
    {
        return;
    }
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);

    out.setVersion(QDataStream::Qt_5_13);

    out << quint16(0) << quint8(8) << (bookModel->data(sourceIndex, 0)).toString();
    out.device()->seek(0);
    out << quint16(static_cast<quint16>(block.size()) - sizeof(quint16));

    tcpSocket->write(block);
}

//右键菜单
void UserWindow::slotTreeMenu(const QPoint &pos)
{
    QMenu menu;

    QModelIndex curIndex = ui->treeView_bookStock->indexAt(pos);      //当前点击的元素的index
    QModelIndex index = curIndex.sibling(curIndex.row(), 0); //该行的第1列元素的index
    sourceIndex = modelProxyIsbn->mapToSource(modelProxyStatus->mapToSource(index));

    if (index.isValid())
    {
        menu.addAction(QStringLiteral("还书"), this, [this]()
        {
            qDebug() << sourceIndex.row();
            if(bookModel->data(sourceIndex.sibling(sourceIndex.row(), 2), 0).toString() == QString::fromLocal8Bit("未借出"))
            {
                statusBar()->showMessage(QString::fromLocal8Bit("书籍未借出"), 3000);
                return;
            }
            this->operationFlag = RETURNBOOK;
            this->connectToServer();
        });
        menu.addSeparator();   //添加分割线

        menu.addAction(QStringLiteral("借阅"), this, [this]()
        {
            if(bookModel->data(sourceIndex.sibling(sourceIndex.row(), 2), 0).toString() == QString::fromLocal8Bit("借出"))
            {
                statusBar()->showMessage(QString::fromLocal8Bit("书籍已借出"), 3000);
                return;
            }
            this->operationFlag = BORROWBOOK;
            this->connectToServer();
        });
        menu.addSeparator();   //添加分割线

        menu.addAction(QStringLiteral("查看评论"), this, [this]()
        {
            this->operationFlag = READBOOKREVIW;
            this->connectToServer();
        });
        menu.addSeparator();

        menu.addAction(QStringLiteral("写评论"), this, [this]()
        {
            WriteReview* writeReviewDialog = new WriteReview(this, userName, bookModel->data(sourceIndex.sibling(sourceIndex.row(), 0), 0).toString(), bookModel->data(sourceIndex.sibling(sourceIndex.row(), 1), 0).toString());
            writeReviewDialog->setAttribute(Qt::WA_DeleteOnClose);
            writeReviewDialog->show();
        });
    }
    menu.exec(QCursor::pos());  //显示菜单
}

void UserWindow::closeConnection()
{
    tcpSocket->close();
    qDebug() << QString::fromLocal8Bit("已关闭Close");
}

//跟新本地数据
void UserWindow::connectForBookMap()
{
    operationFlag = UPDATEBOOKMAP;
    connectToServer();
}

void UserWindow::updateBookMap()
{
    if(operationFlag != UPDATEBOOKMAP)
    {
        return;
    }
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);

    out.setVersion(QDataStream::Qt_5_13);
    out << quint16(0) << quint8(11) << userName;
    out.device()->seek(0);
    out << quint16(static_cast<quint16>(block.size()) - sizeof(quint16));
    tcpSocket->write(block);
}

//该函数提示用户TCP连接已经关闭但数据接收可能不完整。
void UserWindow::connectionClosedByServer()
{
    if(nextBlockSize != 0xffff)
    {
         QMessageBox::information(this, "Error", "Connection Close by server");
    }
    qDebug() << QString::fromLocal8Bit("已关闭Server");
    closeConnection();
}

QPushButton* UserWindow::returnQuit()
{
    return ui->pushButton_quit;
}

void UserWindow::filterChanged(const QString &text)
{
    QRegExp::PatternSyntax syntax = QRegExp::PatternSyntax(QRegExp::RegExp);
    QRegExp regExp(text, Qt::CaseInsensitive, syntax);
    modelProxyIsbn->setFilterRegExp(regExp);
}

void UserWindow::filterStatus(const QString& arg)
{
    QRegExp::PatternSyntax syntax = QRegExp::PatternSyntax(QRegExp::RegExp);
    QRegExp regExp;
    emit ui->lineEdit_fliter->textChanged("");
    if(arg == QString::fromLocal8Bit("未借出"))
    {
        regExp.setPattern(QString::fromLocal8Bit("^\u672A"));
    }
    else if(arg == QString::fromLocal8Bit("借出"))
    {
        regExp.setPattern(QString::fromLocal8Bit("^\u501F"));
    }
    else if(arg == QString::fromLocal8Bit("查看所有"))
    {
        regExp.setPattern(QString::fromLocal8Bit("借出"));
    }
    else
    {
        QString pattern = "";
        qDebug() << *(borrowBookList);
        for(QStringList::const_iterator it = borrowBookList->cbegin(); it != borrowBookList->cend(); ++it)
        {
            if(it == borrowBookList->cend() -1)
            {
                pattern += QString::fromLocal8Bit("^") + *it;
            }
            else
            {
                pattern += QString::fromLocal8Bit("^") + *it + QString::fromLocal8Bit("|");
            }
        }
        if(pattern == "")
        {
            regExp.setPattern("借出");
        }
        else
        {
            regExp.setPattern(pattern);
        }
        regExp.setPatternSyntax(syntax);
        modelProxyIsbn->setFilterRegExp(regExp);
        return;
    }
    regExp.setPatternSyntax(syntax);
    modelProxyStatus->setFilterRegExp(regExp);
}

UserWindow::~UserWindow()
{
    delete bookModel;
    delete userReview;
    delete ui;
}

