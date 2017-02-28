#ifndef CMD_SUB_PROGRAM_H
#define CMD_SUB_PROGRAM_H

#include "Headers/logic/commands/cmd_action.h"

class Cyclogram;

class CmdSubProgram: public CmdAction
{
    Q_OBJECT

public:
    CmdSubProgram(QObject* parent);

    void run() override;

public:
    void setFilePath(const QString& filePath);
    void setName(const QString& name);
    const QString& filePath() const;
    const QString& name() const;

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

private:
    void updateText();

    void writeCustomAttributes(QXmlStreamWriter* writer) override;
    void readCustomAttributes(QXmlStreamReader* reader) override;

    QString mFilePath;
    Cyclogram* mCyclogram;
};

#endif // CMD_SUB_PROGRAM_H
