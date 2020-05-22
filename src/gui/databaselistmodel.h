/****************************************************************************
*   Copyright (C) 2012 by Jens Nissen jens-chessx@gmx.net                   *
****************************************************************************/

#ifndef DATABASELISTMODEL_H
#define DATABASELISTMODEL_H

#include <QAbstractItemModel>
#include <QStringList>
#include <QList>
#include <QVector>

#include "databaseregistry.h"

enum DblvColumns
{
    DBLV_FAVORITE,
    DBLV_NAME,
    DBLV_SIZE,
    DBLV_OPEN,
    DBLV_PATH,
    DBLV_UTF8,
    DBLV_DATE,
    DBLV_DATE_READ
};

class DatabaseListModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit DatabaseListModel(DatabaseRegistry* registry, QObject *parent = nullptr);

    int getLastIndex(const QString& s) const;
    int stars(const QString& s) const;

signals:
    void OnSelectIndex(const QModelIndex&);
    void NoFileFavorite();

public slots:
    void addFileOpen(const QString& s, bool utf8);
    void addFavoriteFile(const QString& s, bool bFavorite, int index);
    void setStars(const QString& s, int stars);
    void setFileUtf8(const QString&, bool);
    void setFileClose(const QString& s, int lastIndex);
    void setFileCurrent(const QString& s);
    void update(const QString& s);

public:
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &child) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual bool hasChildren(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;

public:
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const;
    void limitStars(int limit);
protected:
    void checkFileFavorite();

    DatabaseRegistry* m_registry;
    int m_currentRow;
    QStringList m_columnNames;

private slots:
    void slotDbInserted(QString path);
    void update(int row);
};

#endif // DATABASELISTMODEL_H
