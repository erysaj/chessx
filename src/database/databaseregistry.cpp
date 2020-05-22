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
    if (m_entries.contains(path))
    {
        return &m_entries[path];
    }
    return nullptr;
}

void DatabaseRegistry::insert(DatabaseListEntry entry)
{
    auto path = entry.m_path;
    Q_ASSERT(!m_entries.contains(path));
    m_entries[path] = entry;
    emit didInsert(path);
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

    QList<DatabaseListEntry> entries;
    for (const auto& path: files)
    {
        DatabaseListEntry entry;
        entry.m_path = path;
        entry.m_name = QFileInfo(path).fileName();
        // set isFavorite explicitly as we may fail to parse attrs
        entry.setIsFavorite(true);

        entries.append(entry);
    }
    // update attrs
    for (int i = 0, sz = std::min(entries.size(), attrs.size()); i < sz; ++i)
    {
        entries[i].decodeAttributes(attrs[i]);
    }
    // update last game index
    for (int i = 0, sz = std::min(entries.size(), games.size()); i < sz; ++i)
    {
        entries[i].m_lastGameIndex = games[i].toInt();
    }
    // load
    for (const auto& entry: entries)
    {
        insert(entry);
    }
// TODO: port
//    row = m_paths.size() - 1;
//    QModelIndex m = createIndex(row, DBLV_FAVORITE, (void*)nullptr);
//    emit OnSelectIndex(m);
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
