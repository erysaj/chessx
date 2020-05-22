#ifndef DATABASEREGISTRY_H
#define DATABASEREGISTRY_H

#include <QString>
#include <QList>
#include <QHash>
#include <QObject>

#include "config.h"

class DatabaseInfo;

enum DatabaseListEntryState
{
    EDBL_OPEN,     ///< Database is open
    EDBL_CLOSE     ///< Database is closed
};

class DatabaseListEntry: public QObject
{
    Q_OBJECT

public:
    enum AttrMask: quint32
    {
        AttrMask_State      = 1 << 0,
        AttrMask_Utf8       = 1 << 1,
        AttrMask_Favorite   = 1 << 2,
        AttrMask_Stars      = 1 << 3,
        AttrMask_LastGame   = 1 << 4,
    };

    DatabaseListEntry()
    {
        m_utf8          = false;
        m_state         = EDBL_CLOSE;
        m_stars         = 0;
        m_lastGameIndex = 0;
    }

    QString m_name;
    QString m_path;

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

    QString encodeAttributes() const;
    void decodeAttributes(const QString& data);
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

    void setState   (const QString& identifier, DatabaseListEntryState value);
    void setStars   (const QString& identifier, int value);
    void setUtf8    (const QString& identifier, bool value);
    void setLastGame(const QString& identifier, int value);

    void insert(DatabaseListEntry* entry);

    void saveFavorites(IConfigSection& cfg) const;
    void loadFavorites(const IConfigSection& cfg);

    void onDatabaseOpen(const QString& identifier, bool utf8);
    void makeFavorite(const QString& identifier);

signals:
    void didInsert(QString path);
    void itemChanged(int index, quint32 updates);

public: // TODO: make private
    QList<DatabaseInfo*> m_databases;
    QList<QString> m_paths;

private:
    QHash<QString, DatabaseListEntry*> m_entries;
};

#endif
