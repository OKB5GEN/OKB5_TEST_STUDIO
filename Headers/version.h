#ifndef APP_VERSION_H
#define APP_VERSION_H

#include <QString>

class Version
{
public:
    Version(const QString& str = QString());

    QString toString() const;
    void fromString(const QString& str);

    uint32_t versionCode() const;

    uint32_t major() const;
    uint32_t minor() const;
    uint32_t patch() const;

private:
    uint32_t mMajor;
    uint32_t mMinor;
    uint32_t mPatch;
};
#endif // APP_VERSION_H
