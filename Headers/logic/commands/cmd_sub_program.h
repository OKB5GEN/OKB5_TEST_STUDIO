#ifndef CMD_SUB_PROGRAM_H
#define CMD_SUB_PROGRAM_H

#include <QVariant>
#include <QSharedPointer>

#include "Headers/logic/command.h"

class QFileSystemWatcher;

class Cyclogram;

class CmdSubProgram: public Command
{
    Q_OBJECT

public:
    CmdSubProgram(QObject* parent);
    ~CmdSubProgram();

    void run() override;
    void restart();

    void generateFileName();
    void beforeSave();

public:
    void setFilePath(const QString& filePath, bool reload = true);
    void setName(const QString& name);
    const QString& filePath() const;
    const QString& name() const;

    void setLoaded(bool loaded);

    void setParams(const QMap<QString, QVariant>& in, const QMap<QString, QVariant>& out);

    QSharedPointer<Cyclogram> cyclogram();
    bool loaded() const;

    const QMap<QString, QVariant>& inputParams() const;
    const QMap<QString, QVariant>& outputParams() const;

    void stop() override;
#ifdef ENABLE_CYCLOGRAM_PAUSE
    void pause() override;
    void resume() override;
#endif

protected:
    bool loadFromImpl(Command* other) override;

    // methods for processing variable changes in cyclogram, where command is
    void onNameChanged(const QString& newName, const QString& oldName) override;
    void onVariableRemoved(const QString& name) override;

private slots:
    void execute();
    void onCyclogramFinished(const QString& error);
    void onCyclogramModified();
    void reloadCyclogram();

    // methods for processing variable changes in cyclogram, that command contains
    void onInnerVariableNameChanged(const QString& newName, const QString& oldName);
    void onInnerVariableRemoved(const QString& name);
    void onInnerVariableAdded(const QString& name, qreal value);

signals:
    void commandStarted(Command* cmd);
    void commandFinished(Command* cmd);
    void cyclogramChanged();

private:
    bool load();
    void saveFileIfNotExist();
    void setCyclogram(QSharedPointer<Cyclogram> cyclogram);

    void updateText() override;

    void writeCustomAttributes(QXmlStreamWriter* writer) override;
    void readCustomAttributes(QXmlStreamReader* reader, const Version& fileVersion) override;

    QString mFilePath;
    QWeakPointer<Cyclogram> mCyclogram;
    bool mLoaded;

    QMap<QString, QVariant> mInputParams;
    QMap<QString, QVariant> mOutputParams;

    QFileSystemWatcher* mFileWatcher;
    QString mFileHash;
    bool mNeedHashUpdate;
};

#endif // CMD_SUB_PROGRAM_H
