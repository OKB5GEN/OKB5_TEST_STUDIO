#ifndef COMMANDS_EDIT_TOOLBAR_H
#define COMMANDS_EDIT_TOOLBAR_H

#include <QToolBar>
#include <QVariant>

class CommandsEditToolbar: public QToolBar
{
    Q_OBJECT

public:
    CommandsEditToolbar(QWidget* parent);

    QAction* deleteAction() const; // TODO remove

public slots:
    void reset();

signals:
    void currentCommandChanged(int drakonIcon);

private slots:
    void onActionTriggered(bool checked);

private:
    QAction* addCustomAction(const QString& iconPath, const QString& text, int data);

    QVector<QAction*> mActions;

    QAction* mDeleteAct;
    QAction* mSelectAct;
};
#endif // COMMANDS_EDIT_TOOLBAR_H
