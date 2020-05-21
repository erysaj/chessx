#ifndef DATABASEREGISTRY_H
#define DATABASEREGISTRY_H

#include <QString>
#include <QList>
#include <QHash>
#include <QObject>

class DatabaseInfo;

enum DatabaseListEntryState
{
    EDBL_OPEN,     ///< Database is open
    EDBL_CLOSE     ///< Database is closed
};

class DatabaseListEntry
{
public:
    DatabaseListEntry()
    {
        m_isCurrent     = false;
        m_utf8          = false;
        m_state         = EDBL_CLOSE;
        m_stars         = 0;
        m_lastGameIndex = 0;
    }

    QString m_name;
    QString m_path;

    bool    m_isCurrent;
    bool    m_utf8;
    int     m_lastGameIndex;
    int     m_stars;
    DatabaseListEntryState m_state;

    bool isFavorite() const;
    void setIsFavorite(bool isFavorite);

    QString classType() const
    {
        if (m_name.endsWith(".bin"))
        {
            return "Polyglot";
        }
        if (m_name.endsWith(".ctg"))
        {
            return "Chessbase Book";
        }
        if (m_name.endsWith(".abk"))
        {
            return "Arena Book";
        }
        return m_utf8 ? "UTF8" : "ANSI";
    }
};

class DatabaseRegistry: public QObject
{
    Q_OBJECT

public:
    ~DatabaseRegistry();

    QList<DatabaseInfo*> databases() const { return m_databases; }
    DatabaseInfo* findDisplayName(QString path) const;
    void remove(DatabaseInfo* dbi);

    DatabaseListEntry* findByPath(QString path) const;

    void insert(DatabaseListEntry entry);

signals:
    void didInsert(QString path);

public: // TODO: make private
    QList<DatabaseInfo*> m_databases;

private:
    mutable QHash<QString, DatabaseListEntry> m_entries;
};

#endif
