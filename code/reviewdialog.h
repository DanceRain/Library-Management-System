#ifndef REVIEWDIALOG_H
#define REVIEWDIALOG_H

#include <QDialog>
class QTreeWidget;
class QListWidget;
namespace Ui {
class ReviewDialog;
}

class ReviewDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReviewDialog(QWidget *parent = nullptr);
    QListWidget* listWidget();
    void setTitle(const QString& bookName);
    ~ReviewDialog();

private:
    Ui::ReviewDialog *ui;
};

#endif // REVIEWDIALOG_H
