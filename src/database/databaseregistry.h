#ifndef DATABASEREGISTRY_H
#define DATABASEREGISTRY_H

#include <QString>
#include <QList>
#include <QHash>
#include <QObject>
#include <QPointer>

#include "config.h"
#include "historylist.h"

class QUndoGroup;
class DatabaseInfo;

class DatabaseListEntry: public QObject
{
    Q_OBJECT

public:
    enum State
    {
        EDBL_OPEN,     ///< Database is open
        EDBL_CLOSE     ///< Database is closed
    };


    enum AttrMask: quint32
    {
        AttrMask_State      = 1 << 0,
        AttrMask_Utf8       = 1 << 1,
        AttrMask_Favorite   = 1 << 2,
        AttrMask_Stars      = 1 << 3,
        AttrMask_LastGame   = 1 << 4,
    };

    DatabaseListEntry(const QString& path);

    QString identifier() const { return m_path; }
    QString path() const { return m_path; }
    QString name() const { return m_name; }
    State state() const { return m_state; }

    bool isOpen() const { return m_state == EDBL_OPEN; }

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

private:
    QString m_path;
    QString m_name;

public:
    State m_state;
    int     m_stars;
    bool    m_utf8;
    int     m_lastGameIndex;
    DatabaseInfo* m_info;
};

class DatabaseRegistry: public QObject
{
    Q_OBJECT

public:
    DatabaseRegistry();
    ~DatabaseRegistry();

    int itemsCount() const { return m_identifiers.size(); }
    DatabaseListEntry* itemAt(int index) const;
    int indexOf(const QString& identifier) const { return m_identifiers.indexOf(identifier); }

    QList<DatabaseInfo*> databases() const { return m_databases; }
    DatabaseInfo* findDisplayName(QString path) const;
    void remove(DatabaseInfo* dbi);
    DatabaseInfo* clipDbInfo() const { return m_databases[0]; }

    DatabaseListEntry* findByPath(QString path) const;
    void setStartupDatabase(const QString& identifier);

    void setState   (const QString& identifier, DatabaseListEntry::State value);
    void setStars   (const QString& identifier, int value);
    void setUtf8    (const QString& identifier, bool value);
    void setLastGame(const QString& identifier, int value);

    void insert(DatabaseListEntry* item);

    void saveFavorites(IConfig& cfg) const;
    void loadFavorites(const IConfig& cfg);

    void makeFavorite(const QString& identifier);

    void openFile(const QString& path, bool utf8);

signals:
    void itemsInsertBegan(int first, int last);
    void itemsInsertEnded(int first, int last);
    void itemChanged(int index, quint32 updates);
    void dbOpen(DatabaseInfo* dbi);
    void dbOpenSuccess(DatabaseInfo* dbi);
    void dbOpenFailure(DatabaseInfo* dbi);

public: // TODO: make private
    QList<DatabaseInfo*> m_databases;
    QUndoGroup* m_undoGroup;
    HistoryList m_recentFiles;
    QPointer<DatabaseInfo> m_currentDatabase;

private:
    QList<QString> m_identifiers;
    QHash<QString, DatabaseListEntry*> m_items;

private slots:
    void slotDataBaseLoaded(DatabaseInfo* db);
};

#endif
