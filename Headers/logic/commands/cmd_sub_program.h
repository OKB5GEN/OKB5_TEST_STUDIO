#ifndef CMD_SUB_PROGRAM_H
#define CMD_SUB_PROGRAM_H

#include <QVariant>
#include <QSharedPointer>

#include "Headers/logic/commands/cmd_action.h"

class Cyclogram;

class CmdSubProgram: public CmdAction
{
    Q_OBJECT

public:
    CmdSubProgram(QObject* parent);

    void run() override;
    void restart();

public:
    void setFilePath(const QString& filePath);
    void setName(const QString& name);
    const QString& filePath() const;
    const QString& name() const;

    void setLoaded(bool loaded);

    void setParams(const QMap<QString, QVariant>& in, const QMap<QString, QVariant>& out);

    QSharedPointer<Cyclogram> cyclogram();
    bool loaded() const;

    const QMap<QString, QVariant>& inputParams() const;
    const QMap<QString, QVariant>& outputParams() const;

    QString subprogramPrefix() const;

    void stop() override;
#ifdef ENABLE_CYCLOGRAM_PAUSE
    void pause() override;
    void resume() override;
#endif

private slots:
    void onNameChanged(const QString& newName, const QString& oldName) override;
    void onVariableRemoved(const QString& name) override;
    void execute();
    void onCyclogramFinished(const QString& error);

signals:
    void commandStarted(Command* cmd);
    void commandFinished(Command* cmd);

private:
    bool load();

    void updateText() override;

    void writeCustomAttributes(QXmlStreamWriter* writer) override;
    void readCustomAttributes(QXmlStreamReader* reader) override;

    QString mFilePath;
    QWeakPointer<Cyclogram> mCyclogram;
    bool mLoaded;

    QMap<QString, QVariant> mInputParams;
    QMap<QString, QVariant> mOutputParams;
};

#endif // CMD_SUB_PROGRAM_H
