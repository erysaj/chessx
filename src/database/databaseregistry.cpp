#include "databaseregistry.h"

#include <QUndoGroup>

#include "databaseinfo.h"
#include "clipboarddatabase.h"

DatabaseListEntry::DatabaseListEntry(const QString& path)
    : m_path(path)
    , m_name(QFileInfo(path).fileName())
    , m_state(EDBL_CLOSE)
    , m_stars(0)
    , m_utf8(false)
    , m_lastGameIndex(0)
    , m_info(nullptr)
{
}

DatabaseRegistry::DatabaseRegistry()
    : m_undoGroup(new QUndoGroup(this))
    , m_currentDatabase(nullptr)
{
    auto clipDb = new ClipboardDatabase;
    auto clipInfo = new DatabaseInfo(m_undoGroup, clipDb);
    auto clipItem = new DatabaseListEntry(clipDb->name());
    clipItem->m_state = DatabaseListEntry::EDBL_OPEN;
    clipItem->m_info = clipInfo;
    insert(clipItem);
    m_databases.append(clipInfo);
}

DatabaseRegistry::~DatabaseRegistry()
{
    qDeleteAll(m_databases.begin(), m_databases.end());
}

DatabaseListEntry* DatabaseRegistry::itemAt(int index) const
{
    const auto& identifier = m_identifiers.at(index);
    return m_items.value(identifier, nullptr);
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
    return m_items.value(path, nullptr);
}

void DatabaseRegistry::setStartupDatabase(const QString& identifier)
{
    for (auto item: m_items)
    {
        // startup database is marked 5 stars
        // there should be only 1 such database
        if (item->identifier() == identifier)
        {
            setStars(item->identifier(), 5);
        }
        else if (item->m_stars > 4)
        {
            setStars(item->identifier(), 4);
        }
    }
}

void DatabaseRegistry::setState(const QString& identifier, DatabaseListEntry::State value)
{
    auto index = m_identifiers.indexOf(identifier);
    if (index < 0)
        return;

    auto item = m_items[identifier];
    if (item->m_state == value)
        return;
    item->m_state = value;
    emit itemChanged(index, DatabaseListEntry::AttrMask_State);
}

void DatabaseRegistry::setStars(const QString& identifier, int value)
{
    auto index = m_identifiers.indexOf(identifier);
    if (index < 0)
        return;

    auto item = m_items[identifier];
    if (item->m_stars == value)
        return;

    auto oldFav = item->isFavorite();
    item->m_stars = value;
    auto newFav = item->isFavorite();

    quint32 updates = DatabaseListEntry::AttrMask_Stars;
    if (newFav != oldFav)
    {
        updates |= DatabaseListEntry::AttrMask_Favorite;
    }
    emit itemChanged(index, updates);
}

void DatabaseRegistry::setUtf8(const QString& identifier, bool value)
{
    auto index = m_identifiers.indexOf(identifier);
    if (index < 0)
        return;

    auto item = m_items[identifier];
    if (item->m_utf8 == value)
        return;
    item->m_utf8 = value;
    emit itemChanged(index, DatabaseListEntry::AttrMask_Utf8);
}

void DatabaseRegistry::setLastGame(const QString& identifier, int value)
{
    auto index = m_identifiers.indexOf(identifier);
    if (index < 0)
        return;

    auto item = m_items[identifier];
    if (item->m_lastGameIndex == value)
        return;
    item->m_lastGameIndex = value;
    emit itemChanged(index, DatabaseListEntry::AttrMask_LastGame);
}

void DatabaseRegistry::insert(DatabaseListEntry* item)
{
    auto identifier = item->identifier();
    Q_ASSERT(!m_items.contains(identifier));
    int dst = m_identifiers.size();

    emit itemsInsertBegan(dst, dst);
    m_items[identifier] = item;
    item->setParent(this);
    m_identifiers.append(identifier);
    emit itemsInsertEnded(dst, dst);
}

void DatabaseRegistry::makeFavorite(const QString& identifier)
{
    auto index = m_identifiers.indexOf(identifier);
    if (index < 0)
    {
        auto item = new DatabaseListEntry(identifier);
        item->setIsFavorite(true);
        insert(item);
    }
    else
    {
        auto item = m_items[identifier];
        if (!item->isFavorite())
        {
            setStars(identifier, 1);
            setLastGame(identifier, 0);
        }
    }
}

void DatabaseRegistry::openFile(const QString& path, bool utf8)
{
    // Create database, connect progress bar and open file
    DatabaseInfo* db = new DatabaseInfo(m_undoGroup, path);
    connect(db, SIGNAL(LoadFinished(DatabaseInfo*)), this, SLOT(slotDataBaseLoaded(DatabaseInfo*)), Qt::QueuedConnection);
    emit dbOpen(db);

    if(!db->open(utf8))
    {
        slotDataBaseLoaded(db);
    }
    else
    {
        m_databases.append(db);
    }
}

void DatabaseRegistry::saveFavorites(IConfig& cfg) const
{
    QStringList files;
    QStringList attrs;
    QStringList games;

    for (const auto& identifier: m_identifiers)
    {
        auto item = m_items[identifier];
        if (!item->isFavorite())
            continue;
        files.append(item->path());
        attrs.append(item->encodeAttributes());
        games.append(QString::number(item->m_lastGameIndex));
    }
    cfg.setValue("Favorites/Files", files);
    cfg.setValue("Favorites/Attributes", attrs);
    cfg.setValue("Favorites/LastGameIndex", games);
}

void DatabaseRegistry::loadFavorites(const IConfig& cfg)
{
    auto files = cfg.value("Favorites/Files").toStringList();
    auto attrs = cfg.value("Favorites/Attributes").toStringList();
    auto games = cfg.value("Favorites/LastGameIndex").toList();

    QList<DatabaseListEntry*> items;
    for (const auto& path: files)
    {
        auto item = new DatabaseListEntry(path);
        // set isFavorite explicitly as we may fail to parse attrs
        item->setIsFavorite(true);

        items.append(item);
    }
    // update attrs
    for (int i = 0, sz = std::min(items.size(), attrs.size()); i < sz; ++i)
    {
        items[i]->decodeAttributes(attrs[i]);
    }
    // update last game index
    for (int i = 0, sz = std::min(items.size(), games.size()); i < sz; ++i)
    {
        items[i]->m_lastGameIndex = games[i].toInt();
    }
    // load
    for (auto item: items)
    {
        insert(item);
    }
}

void DatabaseRegistry::slotDataBaseLoaded(DatabaseInfo* db)
{
    if (db->IsLoaded())
    {
        QString path = db->displayName();
        auto item = m_items[path];
        if (!item)
        {
            item = new DatabaseListEntry(path);
            item->m_utf8 = db->IsUtf8();
            item->m_state = DatabaseListEntry::EDBL_OPEN;
            item->m_info = db;
            insert(item);
        }
        else
        {
            item->m_info = db;
            setState(path, DatabaseListEntry::EDBL_OPEN);
            setUtf8(path, db->IsUtf8());
        }
        m_recentFiles.append(path);
        emit dbOpenSuccess(db);
    }
    else
    {
        emit dbOpenFailure(db);
        m_databases.removeOne(db);
        delete db;
    }
}

void DatabaseListEntry::setIsFavorite(bool isFavorite)
{
    m_stars = isFavorite ? 1 : 0;
}

bool DatabaseListEntry::isFavorite() const
{
    return (m_stars > 0);
}

QString DatabaseListEntry::encodeAttributes() const
{
    QString fmt("%1+stars%2");
    return fmt.arg(m_utf8? "utf8": "ansi").arg(m_stars);
}

void DatabaseListEntry::decodeAttributes(const QString& data)
{
    m_utf8 = data.contains("utf8");
    QRegExp regExp("stars([0-9])");
    if (regExp.indexIn(data) >= 0)
    {
        QString d = regExp.cap(1);
        m_stars = d.toInt();
    }
}
