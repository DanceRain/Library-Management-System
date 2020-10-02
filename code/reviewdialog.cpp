#include "reviewdialog.h"
#include "ui_reviewdialog.h"
#include <QListWidget>

ReviewDialog::ReviewDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ReviewDialog)
{
    ui->setupUi(this);
}

QListWidget* ReviewDialog::listWidget()
{
    return ui->listWidget;
}

void ReviewDialog::setTitle(const QString& bookName)
{
    ui->label_title->setText(bookName);
}

ReviewDialog::~ReviewDialog()
{
    delete ui;
}
