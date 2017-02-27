#ifndef CMD_SUB_PROGRAM_H
#define CMD_SUB_PROGRAM_H

#include "Headers/logic/commands/cmd_action.h"

class CmdSubProgram: public CmdAction
{
    Q_OBJECT

public:
    CmdSubProgram(QObject* parent);

    void run() override;

public:
    void setFilePath(const QString& filePath);
    const QString& filePath() const;

private slots:
    void onNameChanged(const QString& newName, const QString& oldName) override;
    void onVariableRemoved(const QString& name) override;
    void execute();

private:
    void updateText();

    void writeCustomAttributes(QXmlStreamWriter* writer) override;
    void readCustomAttributes(QXmlStreamReader* reader) override;

    QString mFilePath;
};

#endif // CMD_SUB_PROGRAM_H
