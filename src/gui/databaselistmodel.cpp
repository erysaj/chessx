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

DatabaseRegistry::~DatabaseRegistry()
{
    qDeleteAll(m_databases.begin(), m_databases.end());
}

DatabaseInfo* DatabaseRegistry::findDisplayName(QString path) const
{
    for (auto dbi: m_databases)
    {
        if (dbi->displayName() == path)
        {
            return dbi;
        }
    }
    return nullptr;
}

void DatabaseRegistry::remove(DatabaseInfo* dbi)
{
    m_databases.removeAt(m_databases.indexOf(dbi));
}

DatabaseListEntry* DatabaseRegistry::findByPath(QString path) const
{
    if (m_entries.contains(path))
    {
        return &m_entries[path];
    }
    return nullptr;
}

void DatabaseRegistry::add(QString path, DatabaseListEntry entry)
{
    Q_ASSERT(!m_entries.contains(path));
    m_entries[path] = entry;
}

DatabaseListModel::DatabaseListModel(DatabaseRegistry* registry, QObject* parent)
    : QAbstractItemModel(parent)
    , m_registry(registry)
{
    m_columnNames << tr("Favorite") << tr("Name") << tr("Size") << tr("Open") << tr("Path") << tr("Format") << tr("Date") << tr("Read");
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
    return m_paths.count();
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
    if (!index.isValid() || index.row() >= m_paths.size())
    {
        return QVariant();
    }

    const auto& path = m_paths.at(index.row());
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
            bool bIsCurrent = db.m_isCurrent;
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
        if(db.m_isCurrent)
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

void DatabaseListModel::addEntry(DatabaseListEntry& d, const QString& s)
{
    beginInsertRows(QModelIndex(), m_paths.count(), m_paths.count());
    d.m_name = QFileInfo(s).fileName();
    m_registry->add(d.m_path, d);
    m_paths.push_back(s);
    endInsertRows();
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
    for (int i = 0, sz = m_paths.size(); i < sz; ++i)
    {
        auto db = m_registry->findByPath(m_paths[i]);
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

void DatabaseListModel::addFileOpen(const QString& s, bool utf8)
{
    auto row = m_paths.indexOf(s);
    if (row < 0)
    {
        // insert new entry
        DatabaseListEntry d;
        d.m_path = s;
        d.m_utf8 = utf8;
        d.m_state = EDBL_OPEN;
        addEntry(d, s);
    }
    else
    {
        // update existing entry
        auto db = m_registry->findByPath(s);
        db->m_utf8 = utf8;
        if (db->m_state != EDBL_OPEN)
        {
            db->m_state = EDBL_OPEN;
            QModelIndex m = createIndex(row, DBLV_OPEN, (void*)nullptr);
            emit QAbstractItemModel::dataChanged(m, m);
            m = createIndex(row, DBLV_UTF8, (void*)nullptr);
            emit QAbstractItemModel::dataChanged(m, m);
        }
    }
}

void DatabaseListModel::addFavoriteFile(const QString& s, bool bFavorite, int index)
{
    auto row = m_paths.indexOf(s);
    if (row < 0)
    {
        // insert new entry
        DatabaseListEntry d;
        d.m_path = s;
        d.setIsFavorite(bFavorite);
        d.m_lastGameIndex = index;
        addEntry(d, s);
        // entry is appended
        row = m_paths.size() - 1;
        QModelIndex m = createIndex(row, DBLV_FAVORITE, (void*)nullptr);
        emit OnSelectIndex(m);
    }
    else
    {
        // update existing entry
        auto db = m_registry->findByPath(s);
        if (db->isFavorite() != bFavorite)
        {
            db->setIsFavorite(bFavorite);
            db->m_lastGameIndex = index;
            QModelIndex m = createIndex(row, DBLV_FAVORITE, (void*)nullptr);
            emit QAbstractItemModel::dataChanged(m, m);
            emit OnSelectIndex(m);
        }
    }
}

void DatabaseListModel::setStars(const QString &s, int stars)
{
    auto row = m_paths.indexOf(s);
    if (row < 0)
        return;

    auto db = m_registry->findByPath(s);
    if (db->m_stars == stars)
        return;

    db->m_stars = stars;
    QModelIndex m = createIndex(row, DBLV_FAVORITE, (void*)nullptr);
    emit QAbstractItemModel::dataChanged(m, m);
}

void DatabaseListModel::setFileClose(const QString& s, int lastIndex)
{
    auto row = m_paths.indexOf(s);
    if (row < 0)
        return;

    auto db = m_registry->findByPath(s);
    if (db->m_state == EDBL_CLOSE)
        return;

    db->m_state = EDBL_CLOSE;
    db->m_lastGameIndex = lastIndex;
    QModelIndex m = createIndex(row, DBLV_OPEN, (void*)nullptr);
    emit QAbstractItemModel::dataChanged(m, m);
}

void DatabaseListModel::setFileUtf8(const QString& s, bool utf8)
{
    auto row = m_paths.indexOf(s);
    if (row < 0)
        return;

    auto db = m_registry->findByPath(s);
    if (db->m_utf8 == utf8)
        return;

    db->m_utf8 = utf8;
    QModelIndex m = createIndex(row, DBLV_UTF8, (void*)nullptr);
    emit QAbstractItemModel::dataChanged(m, m);
}

void DatabaseListModel::setFileCurrent(const QString& s)
{
    for (int row = 0, cnt = m_paths.size(); row < cnt; ++row)
    {
        auto db = m_registry->findByPath(m_paths[row]);
        if (db->m_isCurrent)
        {
            db->m_isCurrent = false;
            QModelIndex m = createIndex(row, DBLV_NAME, (void*) nullptr);
            QModelIndex n = createIndex(row, DBLV_UTF8, (void*) nullptr);
            emit QAbstractItemModel::dataChanged(m, n);
        }
    }

    auto row = m_paths.indexOf(s);
    if (row != -1)
    {
        auto db = m_registry->findByPath(s);
        db->m_isCurrent = true;

        QModelIndex m = createIndex(row, DBLV_NAME, (void*) nullptr);
        QModelIndex n = createIndex(row, DBLV_UTF8, (void*) nullptr);
        emit QAbstractItemModel::dataChanged(m, n);
        emit OnSelectIndex(createIndex(row, DBLV_FAVORITE, (void*) nullptr));
    }
}

void DatabaseListModel::update(const QString& s)
{
    auto row = m_paths.indexOf(s);
    if (row != -1)
    {
        QModelIndex m = createIndex(row, DBLV_NAME, (void*) nullptr);
        QModelIndex n = createIndex(row, DBLV_UTF8, (void*) nullptr);
        emit QAbstractItemModel::dataChanged(m, n);
    }
}

void DatabaseListModel::toStringList(QStringList& list)
{
    for (int row = 0, cnt = m_paths.size(); row < cnt; ++row)
    {
        auto db = m_registry->findByPath(m_paths[row]);
        if (!db->isFavorite())
            continue;
        list.append(db->m_path);
    }
}

void DatabaseListModel::toAttrStringList(QStringList& list) const
{
    for (int row = 0, cnt = m_paths.size(); row < cnt; ++row)
    {
        auto db = m_registry->findByPath(m_paths[row]);
        if (!db->isFavorite())
            continue;
        QString s;
        s = (db->m_utf8) ? "utf8" : "ansi";
        s.append("+stars");
        s.append(QString::number(db->m_stars));
        list.append(s);
    }
}

void DatabaseListModel::toIndexList(QList<QVariant>& list) const
{
    for (int row = 0, cnt = m_paths.size(); row < cnt; ++row)
    {
        auto db = m_registry->findByPath(m_paths[row]);
        if (!db->isFavorite())
            continue;
        list.append(db->m_lastGameIndex);
    }
}

//////////////////////////////////////////////////////
/// \brief DatabaseListEntry::setIsFavorite
/// \param isFavorite

void DatabaseListEntry::setIsFavorite(bool isFavorite)
{
    m_stars = isFavorite ? 1 : 0;
}

bool DatabaseListEntry::isFavorite() const
{
    return (m_stars > 0);
}
