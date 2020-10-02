#include "bookmodel.h"
#include <QDebug>

BookModel::BookModel(QObject* parent):
    QAbstractTableModel (parent),
    headerName{"ISBN", "NAME", "STATUS", "LOCATION"}
{
}

QVariant BookModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }
    if(orientation == Qt::Horizontal)
    {
        return headerName[section];
    }
    return section + 1;
}

int BookModel::rowCount(const QModelIndex& parent) const
{
    return bookData.count();
}

int BookModel::columnCount(const QModelIndex& parent) const
{
    return 4;
}

QVariant BookModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
    {
        return QVariant();
    }
    if(role == Qt::TextAlignmentRole)
    {
        return int(Qt::AlignCenter);
    }
    else if(role == Qt::DisplayRole || role == Qt::EditRole)
    {
        QString str = bookData[index.row()][index.column()];
        return str;
    }
    return QVariant();
}

bool BookModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(index.isValid() && ((role == Qt::EditRole) && (readOnly == false)))
    {
        qDebug() << value;
        latestData.clear();
        latestData.append(bookData[index.row()][0]);
        latestData.append(QString::number(index.column(), 10));
        latestData.append(value.toString());
        bookData[index.row()][index.column()] = value.toString();

        emit dataChanged(index, index);
        return true;
    }
    return false;
}

Qt::ItemFlags BookModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    flags |= Qt::ItemIsEditable;
    return flags;
}

void BookModel::setBookData(const QVector<QStringList>& data)
{
    beginResetModel();
    bookData = data;
    endResetModel();
}

bool BookModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if(row < 0 || count < 1 || row + count > rowCount(parent))
    {
        return false;
    }

    beginRemoveRows(parent, row, row + count - 1);
    for(int i=row+count-1;i>=row;i--)
    {
        //移除该行数据
        bookData.removeAt(i);
    }
    endRemoveRows();
    return true;
}

bool BookModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if(row < 0 || count < 1 || row > rowCount())
    {
        return false;
    }

    beginInsertRows(parent, row, row + count - 1);
    for(int i = row;i < row + count; i++)
    {
        qDebug() << "test";
        //插入数据
        bookData.insert(i - 1, QStringList());
    }
    endInsertRows();
    return true;
}

void BookModel::insertBook(const QStringList& bookInfor)
{
    beginResetModel();
    bookData.append(bookInfor);
    endResetModel();
}
