#include "tablemodel.h"

TableModel::TableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int TableModel::rowCount(const QModelIndex &parent) const
{
    return m_rows.size();
}

int TableModel::columnCount(const QModelIndex &parent) const
{
    return m_headers.size();
}

QVariant TableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_rows.size() || index.column() >= m_headers.size())
        return QVariant();
    if (role == Qt::DisplayRole)
        return m_rows.at(index.row()).at(index.column());
    return QVariant();
}

QVariant TableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) return QVariant();
    if (orientation == Qt::Horizontal && section < m_headers.size())
        return m_headers.at(section);
    if (orientation == Qt::Vertical)
        return section + 1;
    return QVariant();
}

Qt::ItemFlags TableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) return Qt::NoItemFlags;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

bool TableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || role != Qt::EditRole) 
        return false;
    if (index.row() >= m_rows.size() || index.column() >= m_headers.size()) 
        return false;
        
    m_rows[index.row()][index.column()] = value;
    emit dataChanged(index, index, {role});
    return true;
}

bool TableModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (row < 0 || row >= m_rows.size() || count <= 0) return false;
    int last = row + count - 1;
    if (last >= m_rows.size()) last = m_rows.size() - 1;
    beginRemoveRows(QModelIndex(), row, last);
    for (int i = last; i >= row; --i) {
        m_rows.removeAt(i);
        m_rowIds.removeAt(i);
    }
    endRemoveRows();
    return true;
}

void TableModel::setData(const QList<QList<QVariant>> &rows, const QStringList &headers, const QList<int> &rowIds)
{
    beginResetModel();
    m_rows = rows;
    m_headers = headers;
    if (rowIds.isEmpty()) {
        m_rowIds.clear();
        for (int i = 0; i < rows.size(); ++i) m_rowIds.append(i + 1);
    } else {
        m_rowIds = rowIds;
    }
    endResetModel();
}

void TableModel::clear()
{
    beginResetModel();
    m_rows.clear();
    m_headers.clear();
    m_rowIds.clear();
    endResetModel();
}

int TableModel::rowId(int row) const
{
    if (row < 0 || row >= m_rowIds.size()) return -1;
    return m_rowIds.at(row);
}
