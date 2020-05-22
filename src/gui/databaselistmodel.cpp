/****************************************************************************
*   Copyright (C) 2012 by Jens Nissen jens-chessx@gmx.net                   *
****************************************************************************/

#include "databaselistmodel.h"
#include <QDateTime>
#include <QFileInfo>
#include <QFont>
#include <QPixmap>

#include "databaseinfo.h"

#if defined(_MSC_VER) && defined(_DEBUG)
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif // _MSC_VER

static QString formatFileSize(qint64 size)
{
    static QStringList suffixes;
    if (suffixes.empty())
    {
        suffixes << "" << "k" << "M" << "G" << "T" << "P";
    }
    auto s = suffixes.cbegin();
    for (; s + 1 != suffixes.cend() && size >= 1024; ++s)
    {
        size /= 1024;
    }
    return QString("%1%2").arg(size).arg(*s);
}

DatabaseListModel::DatabaseListModel(DatabaseRegistry* registry, QObject* parent)
    : QAbstractItemModel(parent)
    , m_registry(registry)
    , m_currentRow(-1)
{
    m_columnNames << tr("Favorite") << tr("Name") << tr("Size") << tr("Open") << tr("Path") << tr("Format") << tr("Date") << tr("Read");
    connect(m_registry, SIGNAL(didInsert(QString)), this, SLOT(slotDbInserted(QString)));
    connect(m_registry, SIGNAL(itemChanged(int,quint32)), this, SLOT(slotItemChanged(int,quint32)));
}

QModelIndex DatabaseListModel::index(int row, int column, const QModelIndex &parent) const
{
    if(parent.isValid())
    {
        return QModelIndex();
    }
    return createIndex(row, column, (void*)nullptr);
}

QModelIndex DatabaseListModel::parent(const QModelIndex &) const
{
    return QModelIndex();
}

int DatabaseListModel::rowCount(const QModelIndex &parent) const
{
    if(parent.isValid())
    {
        return 0;
    }
    return m_registry->m_paths.count();
}

int DatabaseListModel::columnCount(const QModelIndex &) const
{
    return m_columnNames.count();
}

bool DatabaseListModel::hasChildren(const QModelIndex &parent) const
{
    return !parent.isValid();
}

QVariant DatabaseListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_registry->m_paths.size())
    {
        return QVariant();
    }

    const auto& path = m_registry->m_paths.at(index.row());
    const auto& db = *(m_registry->findByPath(path));
    if(role == Qt::DecorationRole)
    {
        switch(index.column())
        {
        case DBLV_FAVORITE:
        {
            int stars = db.m_stars;
            switch (stars)
            {
            case 0: return QPixmap(":/images/folder_grey.png");
            case 1: return QPixmap(":/images/folder_favorite1.png");
            case 2: return QPixmap(":/images/folder_favorite2.png");
            case 3: return QPixmap(":/images/folder_favorite3.png");
            case 4: return QPixmap(":/images/startup.png");
            case 5: return QPixmap(":/images/active.png");
            }
        }
        case DBLV_OPEN:
        {
            bool bIsOpen = db.m_state == EDBL_OPEN;
            bool bIsCurrent = m_currentRow == index.row();
            if(bIsOpen)
            {
                return QPixmap(bIsCurrent ? ":/images/folder_new.png" : ":/images/fileopen.png");
            }
            else
            {
                return QPixmap(":/images/folder_closed.png");
            }
        }
        default:
            return QVariant();
        }
    }
    else if(role == Qt::DisplayRole)
    {
        switch(index.column())
        {
        case DBLV_FAVORITE:
            return QVariant();
        case DBLV_NAME:
        {
            QString s = db.m_name;
            if (s.endsWith(".pgn")) s.remove(".pgn");
            return s;
        }
        case DBLV_SIZE:
        {
            return formatFileSize(DatabaseInfo::GetDatabaseSize(db.m_path));
        }
        case DBLV_DATE:
        {
            QFileInfo f(db.m_path);
            return f.lastModified().date().toString(Qt::ISODate);
        }
        case DBLV_DATE_READ:
        {
            QFileInfo f(db.m_path);
            return f.lastRead().date().toString(Qt::ISODate);
        }
        case DBLV_OPEN:
            return QVariant();
        case DBLV_PATH:
            return db.m_path;
        case DBLV_UTF8:
            return db.classType();
        default:
            break;
        }
    }
    else if(role == Qt::FontRole)
    {
        if(m_currentRow == index.row())
        {
            if((index.column() == DBLV_NAME) || (index.column() == DBLV_PATH))
            {
                QFont boldFont;
                boldFont.setBold(true);
                return boldFont;
            }
        }
    }
    else if(role == Qt::ToolTipRole)
    {
        switch(index.column())
        {
        case DBLV_FAVORITE:
        {
            return QString(db.isFavorite() ? tr("Favorite") : "");
        }
        case DBLV_PATH:
        {
            QString s = db.m_name;
            return s;
        }
        case DBLV_OPEN:
        {
            bool bIsOpen = db.m_state == EDBL_OPEN;
            return QString(bIsOpen ? tr("Open") : tr("Closed"));
        }
        case DBLV_UTF8:
        {
            return db.classType();
        }
        case DBLV_NAME:
        {
            QString s = db.m_name;
            s[0] = s[0].toUpper();
            return s;
        }
        case DBLV_DATE:
        {
            QFileInfo f(db.m_path);
            return f.lastModified();
        }
        case DBLV_DATE_READ:
        {
            QFileInfo f(db.m_path);
            return f.lastRead();
        }
        case DBLV_SIZE:
        {
            return formatFileSize(DatabaseInfo::GetDatabaseSize(db.m_path));
        }
        default:
            break;
        }
    }
    else if(role == Qt::UserRole)
    {
        switch(index.column())
        {
        case DBLV_FAVORITE:
        {
            int stars = db.m_stars;
            return QString::number(stars);
        }
        case DBLV_PATH:
        {
            QString s = db.m_name;
            return s;
        }
        case DBLV_OPEN:
        {
            bool bIsOpen = db.m_state == EDBL_OPEN;
            return QString(bIsOpen ? "Open" : "Closed");
        }
        case DBLV_UTF8:
        {
            return db.classType();
        }
        case DBLV_NAME:
        {
            QString s = db.m_name;
            s[0] = s[0].toUpper();
            return s;
        }
        case DBLV_DATE:
        {
            QFileInfo f(db.m_path);
            return f.lastModified();
        }
        case DBLV_DATE_READ:
        {
            QFileInfo f(db.m_path);
            return f.lastRead();
        }
        case DBLV_SIZE:
        {
            return DatabaseInfo::GetDatabaseSize(db.m_path);
        }
        default:
            break;
        }
    }

    return QVariant();
}

QVariant DatabaseListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role != Qt::DisplayRole)
    {
        return QVariant();
    }

    if(orientation == Qt::Horizontal)
    {
        return QString("%1").arg(m_columnNames.at(section));
    }
    else
    {
        return QString("%1").arg(section);
    }
}

Qt::ItemFlags DatabaseListModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);

    if(index.isValid())
    {
        return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags | Qt::ItemIsSelectable;
    }
    else
    {
        return Qt::ItemIsDropEnabled | defaultFlags | Qt::ItemIsSelectable;
    }
}

void DatabaseListModel::slotDbInserted(QString path)
{
    beginInsertRows(QModelIndex(), m_registry->m_paths.count(), m_registry->m_paths.count());
    m_registry->m_paths.push_back(path);
    endInsertRows();

    auto item = m_registry->findByPath(path);
    if (item->isFavorite())
    {
        setFileCurrent(path);
    }
}

void DatabaseListModel::slotItemChanged(int index, quint32 updates)
{
    int minCol = -1;
    int maxCol = -1;

    // track the range of affected columns
    auto invalidate = [&minCol, &maxCol](int column)
    {
        if (minCol == -1 || column < minCol)
        {
            minCol = column;
        }
        if (maxCol == -1 || column > maxCol)
        {
            maxCol = column;
        }
    };

    if ((updates & DatabaseListEntry::AttrMask_State) != 0)
    {
        invalidate(DBLV_OPEN);
    }
    if ((updates & DatabaseListEntry::AttrMask_Utf8) != 0)
    {
        invalidate(DBLV_UTF8);
    }
    if ((updates & DatabaseListEntry::AttrMask_Stars) != 0)
    {
        invalidate(DBLV_FAVORITE);
    }
    if ((updates & DatabaseListEntry::AttrMask_Favorite) != 0)
    {
        auto path = m_registry->m_paths[index];
        auto item = m_registry->findByPath(path);
        if (item->isFavorite())
        {
            setFileCurrent(path);
        }
    }

    if (minCol != -1 && maxCol != -1)
    {
        QModelIndex tl = createIndex(index, minCol, (void*)nullptr);
        QModelIndex br = createIndex(index, maxCol, (void*)nullptr);
        emit QAbstractItemModel::dataChanged(tl, br);
    }
}

int DatabaseListModel::getLastIndex(const QString& s) const
{
    auto db = m_registry->findByPath(s);
    if (db)
    {
        return db->m_lastGameIndex;
    }
    return 0;
}

void DatabaseListModel::limitStars(int limit)
{
    for (int i = 0, sz = m_registry->m_paths.size(); i < sz; ++i)
    {
        auto db = m_registry->findByPath(m_registry->m_paths[i]);
        Q_ASSERT(db != nullptr);

        if (db->m_stars <= limit)
            continue;

        db->m_stars = limit;
        QModelIndex m = createIndex(i, DBLV_FAVORITE, (void*)nullptr);
        emit QAbstractItemModel::dataChanged(m, m);
    }
}

int DatabaseListModel::stars(const QString &s) const
{
    auto db = m_registry->findByPath(s);
    if (db)
    {
        return db->m_stars;
    }
    return 0;
}

void DatabaseListModel::setStars(const QString &s, int stars)
{
    m_registry->setStars(s, stars);
}

void DatabaseListModel::setFileClose(const QString& s, int lastIndex)
{
    m_registry->setLastGame(s, lastIndex);
    m_registry->setState(s, EDBL_CLOSE);
}

void DatabaseListModel::setFileUtf8(const QString& s, bool utf8)
{
    m_registry->setUtf8(s, utf8);
}

void DatabaseListModel::setFileCurrent(const QString& s)
{
    auto prev = m_currentRow;
    auto next = m_registry->m_paths.indexOf(s);
    if (next == prev)
        return;

    update(prev);
    m_currentRow = next;
    update(next);
    if (next != -1)
    {
        emit OnSelectIndex(createIndex(next, DBLV_FAVORITE, (void*) nullptr));
    }
}

void DatabaseListModel::update(const QString& s)
{
    auto row = m_registry->m_paths.indexOf(s);
    update(row);
}

void DatabaseListModel::update(int row)
{
    if (row < 0)
        return;
    auto tl = createIndex(row, DBLV_NAME, (void*)nullptr);
    auto br = createIndex(row, DBLV_UTF8, (void*)nullptr);
    emit QAbstractItemModel::dataChanged(tl, br);
}
