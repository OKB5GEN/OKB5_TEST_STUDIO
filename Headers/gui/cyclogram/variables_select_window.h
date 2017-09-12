#ifndef VARIABLES_SELECT_WINDOW_H
#define VARIABLES_SELECT_WINDOW_H

#include "Headers/gui/tools/restorable_dialog.h"
#include <QStringList>

class QListWidget;
class QCheckBox;

class Cyclogram;

class VariablesSelectWindow: public RestorableDialog
{
    Q_OBJECT

public:
    VariablesSelectWindow(QWidget * parent);
    ~VariablesSelectWindow();

    void setCyclogram(QSharedPointer<Cyclogram> cyclogram, const QStringList& excludeVariables);
    QStringList selectedVariables() const;

private slots:
    void onSelectAllCheckBoxStateChanged(int state);
    void onSelectVarCheckBoxStateChanged(int state);

private:
    QListWidget* mVariablesList;
    QCheckBox* mSelectAllBox;

    QStringList mSelectedVariables;
};

#endif // VARIABLES_SELECT_WINDOW_H
