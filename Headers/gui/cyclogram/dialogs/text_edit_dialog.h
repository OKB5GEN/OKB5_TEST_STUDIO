#ifndef TEXT_EDIT_DIALOG_H
#define TEXT_EDIT_DIALOG_H

#include <QDialog>

class QLineEdit;

class Command;

class TextEditDialog : public QDialog
{
    Q_OBJECT

public:
    enum Mode
    {
        SHAPE_EDIT,
        VARIABLE_EDIT
    };

    TextEditDialog(Mode mode, QWidget * parent);
    ~TextEditDialog();

    static QString addVarText();

    void setCommand(Command* command);
    void setText(const QString& text);
    QString text() const;

private slots:
    void onAccept();

private:
    QLineEdit* mLineEdit;
    Command* mCommand;
    Mode mMode;
};

#endif // TEXT_EDIT_DIALOG_H
