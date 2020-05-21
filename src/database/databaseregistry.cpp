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

void DatabaseListEntry::setIsFavorite(bool isFavorite)
{
    m_stars = isFavorite ? 1 : 0;
}

bool DatabaseListEntry::isFavorite() const
{
    return (m_stars > 0);
}
