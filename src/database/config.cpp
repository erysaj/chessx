#include "config.h"

void SettingsConfig::setValue(const QString &key, const QVariant &value)
{
    m_settings.setValue(key, value);
}

QVariant SettingsConfig::value(const QString &key, const QVariant &defaultValue) const
{
    return m_settings.value(key, defaultValue);
}

void SettingsConfig::remove(const QString &key)
{
    m_settings.remove(key);
}

bool SettingsConfig::contains(const QString &key) const
{
    return m_settings.contains(key);
}


void InMemoryConfig::setValue(const QString &key, const QVariant &value)
{
    m_data[key] = value;
}

QVariant InMemoryConfig::value(const QString &key, const QVariant &defaultValue) const
{
    return m_data.value(key, defaultValue);
}

void InMemoryConfig::remove(const QString &key)
{
    m_data.remove(key);
}

bool InMemoryConfig::contains(const QString &key) const
{
    return m_data.contains(key);
}
