#include "Headers/version.h"
#include "Headers/logger/Logger.h"

namespace
{
    static const QString SEPARATOR = ".";
}

Version::Version(const QString& str):
    mMajor(0),
    mMinor(1),
    mPatch(0)
{
    fromString(str);
}

QString Version::toString() const
{
    return QString("%1%2%3%4%5").arg(mMajor).arg(SEPARATOR).arg(mMinor).arg(SEPARATOR).arg(mPatch);
}

uint32_t Version::versionCode() const
{
    uint32_t versionCode = mPatch;

    versionCode += mMinor * 1000;
    versionCode += mMajor * 1000000;

    return versionCode;
}

uint32_t Version::major() const
{
    return mMajor;
}

uint32_t Version::minor() const
{
    return mMinor;
}

uint32_t Version::patch() const
{
    return mPatch;
}

void Version::fromString(const QString& str)
{
    if (str.isEmpty())
    {
        return;
    }

    QStringList tokens = str.split(SEPARATOR);
    if (tokens.size() != 3)
    {
        LOG_ERROR(QString("Ivalid version format '%1'").arg(str));
        return;
    }

    mMajor = tokens.at(0).toUInt();
    mMinor = tokens.at(1).toUInt();
    mPatch = tokens.at(2).toUInt();
}
