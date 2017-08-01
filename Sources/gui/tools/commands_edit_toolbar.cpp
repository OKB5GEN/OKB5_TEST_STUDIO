#include "Headers/gui/tools/commands_edit_toolbar.h"
#include "Headers/shape_types.h"

CommandsEditToolbar::CommandsEditToolbar(QWidget* parent):
    QToolBar(tr("Edit"), parent)
{
    mSelectAct = addCustomAction(":/resources/images/select", tr("Select"), -1);
    QAction* addModuleCmdAction = addCustomAction(":/resources/images/cmd_module", tr("Add module command"), DRAKON::ACTION_MODULE);
    QAction* addMathCmdAction = addCustomAction(":/resources/images/cmd_math", tr("Add math command"), DRAKON::ACTION_MATH);
    QAction* addQuestionCmdAction = addCustomAction(":/resources/images/cmd_question", tr("Add question command"), DRAKON::CONDITION);
    QAction* addCycleCmdAction = addCustomAction(":/resources/images/cmd_cycle", tr("Add cycle command"), DRAKON::CYCLE);
    QAction* addSelectStateCmdAction = addCustomAction(":/resources/images/cmd_select_state", tr("Add select state command"), DRAKON::SELECT_STATE);
    QAction* addGoToBranchCmdAction = addCustomAction(":/resources/images/cmd_go_to_branch", tr("Add go-to-branch command"), DRAKON::GO_TO_BRANCH);
    QAction* addSubprogramCmdAction = addCustomAction(":/resources/images/cmd_subprogram", tr("Add subprogram command"), DRAKON::SUBPROGRAM);
    //QAction* addParallelProcessCmdAction = addCustomAction(":/resources/images/cmd_parallel_process", tr("Add parallel process command"), DRAKON::PARALLEL_PROCESS);
    QAction* addDelayCmdAction = addCustomAction(":/resources/images/cmd_delay", tr("Add delay command"), DRAKON::DELAY);
    //QAction* addInputCmdAction = addCustomAction(":/resources/images/cmd_input", tr("Add input command"), DRAKON::INPUT);
    QAction* addOutputCmdAction = addCustomAction(":/resources/images/cmd_output", tr("Add output command"), DRAKON::OUTPUT);

    // delete command action
    mDeleteAct = new QAction(QIcon(":/resources/images/delete_all"), tr("Delete"), this);
    mDeleteAct->setEnabled(false);
    mDeleteAct->setStatusTip(tr("Delete selected command"));

    addSeparator();

    addAction(mDeleteAct);

    reset();

    foreach (QAction* action, mActions)
    {
        connect(action, SIGNAL(triggered(bool)), this, SLOT(onActionTriggered(bool)));
    }
}

QAction* CommandsEditToolbar::deleteAction() const
{
    return mDeleteAct;
}

QAction* CommandsEditToolbar::addCustomAction(const QString& iconPath, const QString& text, int data)
{
    QAction* action = new QAction(QIcon(iconPath), text, this);
    action->setCheckable(true);
    action->setData(data);
    action->setStatusTip(text);
    addAction(action);
    mActions.push_back(action);

    return action;
}

void CommandsEditToolbar::reset()
{
    mDeleteAct->setDisabled(true);

    foreach (QAction* action, mActions)
    {
        action->blockSignals(true);
        action->setChecked(false);
        action->blockSignals(false);
    }

    mSelectAct->setChecked(true);
    emit currentCommandChanged(-1);
}

void CommandsEditToolbar::onActionTriggered(bool checked)
{
    QAction* triggeredAction = qobject_cast<QAction*>(QObject::sender());

    if (!triggeredAction)
    {
        return;
    }

    int command = -1;

    foreach (QAction* action, mActions)
    {
        action->blockSignals(true);

        if (action == triggeredAction)
        {
            action->setChecked(true);
            command = action->data().toInt();
        }
        else
        {
            action->setChecked(false);
        }

        action->blockSignals(false);
    }


    emit currentCommandChanged(command);
}
