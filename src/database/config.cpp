#include "config.h"

void SettingsConfigSection::setValue(const QString &key, const QVariant &value)
{
    m_settings.setValue(key, value);
}

QVariant SettingsConfigSection::value(const QString &key, const QVariant &defaultValue) const
{
    return m_settings.value(key, defaultValue);
}

void SettingsConfigSection::remove(const QString &key)
{
    m_settings.remove(key);
}

bool SettingsConfigSection::contains(const QString &key) const
{
    return m_settings.contains(key);
}


void InMemoryConfigSection::setValue(const QString &key, const QVariant &value)
{
    m_data[key] = value;
}

QVariant InMemoryConfigSection::value(const QString &key, const QVariant &defaultValue) const
{
    return m_data.value(key, defaultValue);
}

void InMemoryConfigSection::remove(const QString &key)
{
    m_data.remove(key);
}

bool InMemoryConfigSection::contains(const QString &key) const
{
    return m_data.contains(key);
}
