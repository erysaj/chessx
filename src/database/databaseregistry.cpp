#include "databaseregistry.h"
#include "databaseinfo.h"

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
    return m_entries.value(path, nullptr);
}

void DatabaseRegistry::setState(const QString& identifier, DatabaseListEntryState value)
{
    auto index = m_paths.indexOf(identifier);
    if (index < 0)
        return;

    auto item = m_entries[identifier];
    if (item->m_state == value)
        return;
    item->m_state = value;
    emit itemChanged(index, DatabaseListEntry::AttrMask_State);
}

void DatabaseRegistry::setStars(const QString& identifier, int value)
{
    auto index = m_paths.indexOf(identifier);
    if (index < 0)
        return;

    auto item = m_entries[identifier];
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
    auto index = m_paths.indexOf(identifier);
    if (index < 0)
        return;

    auto item = m_entries[identifier];
    if (item->m_utf8 == value)
        return;
    item->m_utf8 = value;
    emit itemChanged(index, DatabaseListEntry::AttrMask_Utf8);
}

void DatabaseRegistry::setLastGame(const QString& identifier, int value)
{
    auto index = m_paths.indexOf(identifier);
    if (index < 0)
        return;

    auto item = m_entries[identifier];
    if (item->m_lastGameIndex == value)
        return;
    item->m_lastGameIndex = value;
    emit itemChanged(index, DatabaseListEntry::AttrMask_LastGame);
}

void DatabaseRegistry::insert(DatabaseListEntry* entry)
{
    auto path = entry->m_path;
    Q_ASSERT(!m_entries.contains(path));
    m_entries[path] = entry;
    entry->setParent(this);
    emit didInsert(path);
}

void DatabaseRegistry::onDatabaseOpen(const QString& identifier, bool utf8)
{
    auto index = m_paths.indexOf(identifier);
    if (index < 0)
    {
        // insert new entry
        auto item = new DatabaseListEntry();
        item->m_path = identifier;
        item->m_utf8 = utf8;
        item->m_state = EDBL_OPEN;
        item->m_name = QFileInfo(identifier).fileName();
        insert(item);
    }
    else
    {
        // update existing entry
        setState(identifier, EDBL_OPEN);
        setUtf8(identifier, utf8);
    }
}

void DatabaseRegistry::makeFavorite(const QString& identifier)
{
    auto index = m_paths.indexOf(identifier);
    if (index < 0)
    {
        auto item = new DatabaseListEntry();
        item->m_path = identifier;
        item->m_name = QFileInfo(identifier).fileName();
        item->setIsFavorite(true);
        insert(item);
    }
    else
    {
        auto item = m_entries[identifier];
        if (!item->isFavorite())
        {
            setStars(identifier, 1);
            setLastGame(identifier, 0);
        }
    }
}

void DatabaseRegistry::saveFavorites(IConfigSection& cfg) const
{
    QStringList files;
    QStringList attrs;
    QStringList games;

    for (const auto& path: m_paths)
    {
        const auto& entry = *findByPath(path);
        if (!entry.isFavorite())
            continue;
        files.append(entry.m_path);
        attrs.append(entry.encodeAttributes());
        games.append(QString::number(entry.m_lastGameIndex));
    }
    cfg.setValue("Files", files);
    cfg.setValue("Attributes", attrs);
    cfg.setValue("LastGameIndex", games);
}

void DatabaseRegistry::loadFavorites(const IConfigSection& cfg)
{
    auto files = cfg.value("Files").toStringList();
    auto attrs = cfg.value("Attributes").toStringList();
    auto games = cfg.value("LastGameIndex").toList();

    QList<DatabaseListEntry*> entries;
    for (const auto& path: files)
    {
        auto entry = new DatabaseListEntry();
        entry->m_path = path;
        entry->m_name = QFileInfo(path).fileName();
        // set isFavorite explicitly as we may fail to parse attrs
        entry->setIsFavorite(true);

        entries.append(entry);
    }
    // update attrs
    for (int i = 0, sz = std::min(entries.size(), attrs.size()); i < sz; ++i)
    {
        entries[i]->decodeAttributes(attrs[i]);
    }
    // update last game index
    for (int i = 0, sz = std::min(entries.size(), games.size()); i < sz; ++i)
    {
        entries[i]->m_lastGameIndex = games[i].toInt();
    }
    // load
    for (auto entry: entries)
    {
        insert(entry);
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
