#include "userdialog.h"
#include "ui_userdialog.h"
#include "databasecontrol.h"
#include "userinformation.h"
#include "reviewdialog.h"
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QListWidgetItem>
#include <QTextBrowser>
#include <QMenu>


UserDialog::UserDialog(QString name, QWidget *parent, QTcpSocket* socket) :
    QMainWindow(parent),
    ui(new Ui::UserDialog),
    tcpSocket(socket),
    userName(name)
{
    ui->setupUi(this);
    ui->label_userNameContent->setText(userName);


    connect(ui->treeView_bookStock, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(slotTreeMenu(const QPoint&)));
}


void UserDialog::slotTreeMenu(const QPoint &pos)
{
    QMenu menu;

    QModelIndex curIndex = ui->treeView_bookStock->indexAt(pos);      //当前点击的元素的index
    QModelIndex index = curIndex.sibling(curIndex.row(),0); //该行的第1列元素的index
    if (index.isValid())
    {
        menu.addAction(QStringLiteral("借阅"), this, SLOT(slotBorrowBook()));
        menu.addSeparator();    //添加一个分隔线
        menu.addAction(QStringLiteral("查看评论"), this, SLOT(slotReadReview()));
    }
    menu.exec(QCursor::pos());  //显示菜单
}

void UserDialog::slotReadReview()
{
//    ReviewDialog* reviewDialog = new ReviewDialog(this);
//    reviewDialog->setAttribute(Qt::WA_DeleteOnClose);

//    QModelIndex idx = ui->treeView_bookStock->currentIndex();
//    QString isbn = model->record(idx.row()).value("book_isbn").toString();


//    QMap<QString, QMap<QString, QString>>* map = databaseControl->getUserReviewMap();

//    for(QMap<QString, QString>::const_iterator it = (*map)[isbn].cbegin();
//        it != (*map)[isbn].cend(); ++it)
//    {
//        QListWidgetItem *itme = new QListWidgetItem(reviewDialog->listWidget());
//        QSize size = itme->sizeHint();
//        itme->setSizeHint(QSize(size.width(), 100));

//        QWidget* widget = new QWidget(reviewDialog);
//        QVBoxLayout* layout = new QVBoxLayout;
//        QTextBrowser* textBrower = new QTextBrowser(widget);
//        QLabel* author = new QLabel(it.key(), widget);
//        textBrower->setText(it.value());
//        layout->addWidget(author);
//        layout->addWidget(textBrower);
//        widget->setLayout(layout);

//        reviewDialog->listWidget()->setItemWidget(itme, widget);
//    }


//    //为每一个评论建立QListWidgetItem并一个Widget为其ItemWidget


//    reviewDialog->show();
//    delete map;
}

void UserDialog::slotBorrowBook()
{

}

UserDialog::~UserDialog()
{
    delete ui;
}

QPushButton* UserDialog::returnQuit()
{
    return ui->pushButton_quit;
}

void UserDialog::on_pushButton_clicked()
{

}
