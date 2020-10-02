#ifndef BOOKMODEL_H
#define BOOKMODEL_H

#include <QAbstractTableModel>

class BookModel : public QAbstractTableModel
{
public:
    BookModel(QObject* parent = 0);
    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QVariant data(const QModelIndex& index, int role) const;
    bool setData(const QModelIndex& index, const QVariant& value, int role);
    void setBookData(const QVector<QStringList>& data);
    Qt::ItemFlags flags(const QModelIndex& index) const;

    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex());
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
    void insertBook(const QStringList& bookInfor);

    QVector<QStringList> getBookData()
    {
        return bookData;
    }
    QStringList getLatestModification()
    {
        return latestData;
    }
    void setReadOnly()
    {
        readOnly = true;
    }
    void setWriteOnly()
    {
        readOnly = false;
    }


private:
    QVector<QStringList> bookData;
    QStringList latestData;
    QStringList headerName;
    bool readOnly = false;
};

#endif // BOOKMODEL_H
