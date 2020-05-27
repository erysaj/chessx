#ifndef CONFIG_H
#define CONFIG_H

#include <QString>
#include <QVariant>
#include <QHash>
#include <QSettings>


/** @ingroup Core
  IConfig interface abstracts an access to a settings file.
*/
struct IConfig
{
    virtual ~IConfig() {}

    virtual void setValue(const QString &key, const QVariant &value) = 0;
    virtual QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const = 0;
    virtual void remove(const QString &key) = 0;
    virtual bool contains(const QString &key) const = 0;
};


class SettingsConfig: public IConfig
{
public:
    SettingsConfig(QSettings& settings)
        : m_settings(settings)
    {}

    void setValue(const QString &key, const QVariant &value) override;
    QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const override;
    void remove(const QString &key) override;
    bool contains(const QString &key) const override;

private:
    QSettings& m_settings;
};


class InMemoryConfig: public IConfig
{
public:
    void setValue(const QString &key, const QVariant &value) override;
    QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const override;
    void remove(const QString &key) override;
    bool contains(const QString &key) const override;

private:
    QHash<QString, QVariant> m_data;
};

#endif
