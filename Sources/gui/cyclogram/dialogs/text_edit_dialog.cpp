#include "Headers/gui/cyclogram/dialogs/text_edit_dialog.h"

#include "Headers/gui/tools/console_text_widget.h"
#include "Headers/logic/variable_controller.h"
#include "Headers/logic/command.h"

#include <QtWidgets>

TextEditDialog::TextEditDialog(Mode mode, QWidget * parent):
    QDialog(parent),
    mMode(mode)
{
    QGridLayout * layout = new QGridLayout(this);

    int row = 0;
    mLineEdit = new QLineEdit(this);
    mLineEdit->setText("Default value");
    layout->addWidget(mLineEdit, row, 0);

    ++row;
    if (mMode == SHAPE_EDIT)
    {
        setWindowTitle(tr("Default Shape Edit Dialog"));
        ConsoleTextWidget* consoleTextWidget = new ConsoleTextWidget(this);
        layout->addWidget(consoleTextWidget, row, 0);
        ++row;
    }
    else if (mMode == VARIABLE_EDIT)
    {
        setWindowTitle(tr("Add new variable"));
    }

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel , Qt::Horizontal, this);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(onAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    layout->addWidget(buttonBox, row, 0);
    ++row;

    setLayout(layout);
}

TextEditDialog::~TextEditDialog()
{

}

void TextEditDialog::setCommand(Command* command)
{
    mCommand = command;
}

void TextEditDialog::setText(const QString& text)
{
    mLineEdit->setText(text);
}

QString TextEditDialog::text() const
{
    return mLineEdit->text();
}

void TextEditDialog::onAccept()
{
    if (mMode == VARIABLE_EDIT)
    {
        Q_ASSERT(mCommand);

        QString varName = mLineEdit->text();
        if (varName.isEmpty())
        {
            QMessageBox::warning(this, tr("Error"), tr("Variable name must not be empty!"));
            return;
        }

        if (mCommand->variableController()->isVariableExist(varName))
        {
            QMessageBox::warning(this, tr("Error"), tr("Variable with such name already exist. Try another name"));
            return;
        }
    }

    accept();
}

QString TextEditDialog::addVarText()
{
    return tr("Add new");
}
