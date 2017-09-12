#include "Headers/gui/cyclogram/dialogs/cmd_subprogram_edit_dialog.h"
#include "Headers/logic/cyclogram_manager.h"
#include "Headers/logic/commands/cmd_sub_program.h"
#include "Headers/logic/cyclogram.h"
#include "Headers/logic/variable_controller.h"
#include "Headers/logger/Logger.h"
#include "Headers/file_reader.h"
#include "Headers/gui/tools/console_text_widget.h"
#include "Headers/app_settings.h"

#include <QtWidgets>

CmdSubProgramEditDialog::CmdSubProgramEditDialog(QWidget * parent):
    RestorableDialog(parent),
    mCommand(Q_NULLPTR)
{
    setupUI();
    setWindowTitle(tr("Edit sub program"));
    setMinimumSize(QSize(1000, 500));
}

CmdSubProgramEditDialog::~CmdSubProgramEditDialog()
{

}

void CmdSubProgramEditDialog::setupUI()
{
    QGridLayout * layout = new QGridLayout(this);

    mSubprogramNameStr = new QLineEdit(tr("Sub"), this);
    mSubprogramNameStr->setMaximumWidth(100);

    // Cyclogram file
    QGroupBox* filePathBox = new QGroupBox(tr("Settings"), this);
    QGridLayout* fileNameLayout = new QGridLayout(filePathBox);
    mFileNameStr = new QLineEdit(filePathBox);
    mFileNameStr->setBackgroundRole(QPalette::Dark);
    QPushButton* browseButton = new QPushButton(filePathBox);
    connect(browseButton, SIGNAL(clicked(bool)), this, SLOT(openFile()));

    connect(mFileNameStr, SIGNAL(editingFinished()), this, SLOT(updateUI()));

    browseButton->setText(tr("Browse"));

    fileNameLayout->addWidget(new QLabel(tr("Name:"), this), 0, 0);
    fileNameLayout->addWidget(mSubprogramNameStr, 0, 1);
    fileNameLayout->addWidget(new QLabel(tr("File:"), this), 0, 2);
    fileNameLayout->addWidget(mFileNameStr, 0, 3);
    fileNameLayout->addWidget(browseButton, 0, 4);

    // extended settings
    mShowExtendedSettings = new QPushButton(tr("Show extended settings"), this);
    mShowExtendedSettings->setCheckable(true);
    connect(mShowExtendedSettings, SIGNAL(toggled(bool)), this, SLOT(onShowExtendedSettings(bool)));
    fileNameLayout->addWidget(mShowExtendedSettings, 0, 5);

    mCyclogramDescription = new QTextEdit(this);
    mCyclogramDescription->setPlaceholderText(tr("Type subprogram description here"));

    mDescriptionHeader = new QLabel(tr("Description:"), this);

    fileNameLayout->addWidget(mDescriptionHeader, 1, 0, 1, 1);
    fileNameLayout->addWidget(mCyclogramDescription, 1, 1, 1, 5);

    onShowExtendedSettings(false);

    filePathBox->setLayout(fileNameLayout);

    layout->addWidget(filePathBox, 0, 0, 1, 2);

    // input and output params
    QGroupBox* inputGroupBox = new QGroupBox(tr("Input. Set subprogram variables initial values (either from calling cyclogram variable or direct value)"), this);
    QGroupBox* outputGroupBox = new QGroupBox(tr("Output. Set calling cyclogram variables current values (either from subprogram variable value or direct value)"), this);
    QVBoxLayout* inputLayout = new QVBoxLayout();
    QVBoxLayout* outputLayout = new QVBoxLayout();

    mInParams = new QTableWidget(this);
    QStringList inHeaders;
    inHeaders.append(tr("Subprogram variable"));
    inHeaders.append(tr(""));
    inHeaders.append(tr("Calling c. variable"));
    inHeaders.append(tr(""));
    inHeaders.append(tr("Direct value"));
    mInParams->setColumnCount(inHeaders.size());
    mInParams->setHorizontalHeaderLabels(inHeaders);
    inputLayout->addWidget(mInParams);

    mOutParams = new QTableWidget(this);
    QStringList outHeaders;
    outHeaders.append(tr("Calling c. variable"));
    outHeaders.append(tr(""));
    outHeaders.append(tr("Subprogram variable"));
    outHeaders.append(tr(""));
    outHeaders.append(tr("Direct value"));
    mOutParams->setColumnCount(outHeaders.size());
    mOutParams->setHorizontalHeaderLabels(outHeaders);
    outputLayout->addWidget(mOutParams);

    inputGroupBox->setLayout(inputLayout);
    outputGroupBox->setLayout(outputLayout);

    layout->addWidget(inputGroupBox, 1, 0);
    layout->addWidget(outputGroupBox, 1, 1);

    // Console text widget >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    mConsoleTextWidget = new ConsoleTextWidget(this);
    layout->addWidget(mConsoleTextWidget, 2, 0, 1, 2);

    // Dialog button box >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel , Qt::Horizontal, this);
    layout->addWidget(buttonBox, 3, 1);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(onAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    setLayout(layout);
}

void CmdSubProgramEditDialog::setCommand(CmdSubProgram* command, QSharedPointer<Cyclogram> callingCyclogram)
{
    mCommand = command;
    mCallingCyclogram = callingCyclogram;

    if (!mCommand)
    {
        LOG_ERROR(QString("Incorrect subprogram command specified"));
        return;
    }

    mSubprogramNameStr->setText(mCommand->name());
    mFileNameStr->setText(mCommand->filePath());

    auto cmdCyclogram = mCommand->cyclogram();
    mCyclogramDescription->setPlainText(cmdCyclogram->setting(Cyclogram::SETTING_DESCRIPTION).toString());

    updateUI();

    mConsoleTextWidget->setCommand(mCommand);

    readSettings();
}

void CmdSubProgramEditDialog::onAccept()
{
    if (!mCommand)
    {
        LOG_ERROR(QString("No subprogram command specified"));
        return;
    }

    if (mSubprogramNameStr->text().isEmpty())
    {
        QMessageBox::warning(this, tr("Incorrect input"), tr("Subprogram name must not be empty!"));
        return;
    }

    mCommand->setFilePath(mFileNameStr->text());
    mCommand->setName(mSubprogramNameStr->text());

    auto cmdCyclogram = mCommand->cyclogram();
    QString curCyclogramDecr = cmdCyclogram->setting(Cyclogram::SETTING_DESCRIPTION).toString();
    QString newCyclogramDescr = mCyclogramDescription->toPlainText();
    if (curCyclogramDecr != newCyclogramDescr)
    {
        cmdCyclogram->setSetting(Cyclogram::SETTING_DESCRIPTION, newCyclogramDescr);
    }

    QMap<QString, QVariant> input;
    QMap<QString, QVariant> output;

    int inCount = mInParams->rowCount();
    int outCount = mOutParams->rowCount();

    for (int i = 0; i < inCount; ++i)
    {
        QLabel* label = qobject_cast<QLabel*>(mInParams->cellWidget(i, 0));
        QString name;
        if (label)
        {
            name = label->text();
        }

        QCheckBox* varSelectBtn = qobject_cast<QCheckBox*>(mInParams->cellWidget(i, 1));
        if (varSelectBtn->isChecked())
        {
            QComboBox* comboBox = qobject_cast<QComboBox*>(mInParams->cellWidget(i, 2));
            if (comboBox)
            {
                input[name] = comboBox->currentText();
            }
        }
        else
        {
            QLineEdit* valueText = qobject_cast<QLineEdit*>(mInParams->cellWidget(i, 4));
            if (valueText)
            {
                input[name] = valueText->text().toDouble();
            }
        }
    }

    for (int i = 0; i < outCount; ++i)
    {
        QLabel* label = qobject_cast<QLabel*>(mOutParams->cellWidget(i, 0));
        QString name;
        if (label)
        {
            name = label->text();
        }

        QCheckBox* varSelectBtn = qobject_cast<QCheckBox*>(mOutParams->cellWidget(i, 1));
        if (varSelectBtn->isChecked())
        {
            QComboBox* comboBox = qobject_cast<QComboBox*>(mOutParams->cellWidget(i, 2));
            if (comboBox)
            {
                output[name] = comboBox->currentText();
            }
        }
        else
        {
            QLineEdit* valueText = qobject_cast<QLineEdit*>(mOutParams->cellWidget(i, 4));
            if (valueText)
            {
                output[name] = valueText->text().toDouble();
            }
        }
    }

    mCommand->setParams(input, output);
    mConsoleTextWidget->saveCommand();

    accept();
}

void CmdSubProgramEditDialog::openFile()
{
    QString path = Cyclogram::defaultStorePath();

    if (!mFileNameStr->text().isEmpty())
    {
        QString currentFileName = path + mFileNameStr->text();
        if (QFileInfo(currentFileName).exists())
        {
            path = QFileInfo(currentFileName).absoluteDir().path();
        }
        else
        {
            LOG_WARNING(QString("Corrupted file reference '%1' detected. Set path to current application directory path").arg(currentFileName));
        }
    }

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open cyclogram file"), path, tr("OKB5 Cyclogram Files (*%1)").arg(AppSettings::extension()));
    if (!fileName.isEmpty())
    {
        // load cyclogram
        QStringList tokens = fileName.split(Cyclogram::defaultStorePath());

        if (tokens.size() != 2)
        {
            LOG_ERROR(QString("Invalid directory. All cyclograms must be stored in %1 or its subfolders").arg(Cyclogram::defaultStorePath()));
            return;
        }

        mFileNameStr->setText(tokens.at(1));
        updateUI();
    }
}

void CmdSubProgramEditDialog::updateUI()
{
    if (mCallingCyclogram.isNull())
    {
        LOG_ERROR(QString("No calling cyclogram set to subprogram '%1'").arg(mSubprogramNameStr->text()));
        return;
    }

    QSharedPointer<Cyclogram> cyclogram = mCommand->cyclogram();
    auto callingCyclogram = mCallingCyclogram.lock();

    mSubprogramNameStr->setText(mCommand->text());

    mInParams->clearContents();
    mOutParams->clearContents();

    const QMap<QString, VariableController::VariableData>& subprogramVariables = cyclogram->variableController()->variablesData();
    const QMap<QString, VariableController::VariableData>& callingCyclogramVariables = callingCyclogram->variableController()->variablesData();

    const QMap<QString, QVariant>& inputParams = mCommand->inputParams();
    const QMap<QString, QVariant>& outputParams = mCommand->outputParams();

    int inCount = subprogramVariables.size();
    int outCount = callingCyclogramVariables.size();
    mInParams->setRowCount(inCount);
    mOutParams->setRowCount(outCount);

    int i = 0;
    for (auto it = subprogramVariables.begin(); it != subprogramVariables.end(); ++it)
    {
        // param name
        QString name = it.key();
        QLabel* text = new QLabel(mInParams);
        text->setTextInteractionFlags(Qt::NoTextInteraction);
        text->setText(name);
        mInParams->setCellWidget(i, 0, text);

        // "use variable input" button
        QCheckBox* varSelectBtn = new QCheckBox(mInParams);
        mInParams->setCellWidget(i, 1, varSelectBtn);

        // variable selector
        QComboBox* comboBox = new QComboBox(mInParams);
        comboBox->installEventFilter(this);
        comboBox->addItems(callingCyclogramVariables.keys());
        mInParams->setCellWidget(i, 2, comboBox);

        // "use direct value input" button
        QCheckBox* valueSelectBtn = new QCheckBox(mInParams);
        mInParams->setCellWidget(i, 3, valueSelectBtn);

        // variable selector
        QLineEdit* valueEdit = new QLineEdit(mInParams);
        valueEdit->setValidator(new QDoubleValidator(mInParams));
        valueEdit->setText(QString::number(it.value().initialValue));
        mInParams->setCellWidget(i, 4, valueEdit);

        // by default command input params are directly set values equal to "initial" value of corresponding variable in cyclogram file
        bool isVariable = false;

        if (mCommand->loaded()) // already set command (load variables mapping from command) TODO
        {
            auto it = inputParams.find(name);
            if (it != inputParams.end())
            {
                if (it.value().type() == QVariant::String)
                {
                    int index = comboBox->findText(it.value().toString());
                    if (index != -1)
                    {
                        comboBox->setCurrentIndex(index);
                        isVariable = true;
                    }
                }
                else if (it.value().type() == QVariant::Double)
                {
                    valueEdit->setText(it.value().toString());
                    isVariable = false;
                }
            }
        }

        valueEdit->setEnabled(!isVariable);
        valueSelectBtn->setChecked(!isVariable);
        comboBox->setEnabled(isVariable);
        varSelectBtn->setChecked(isVariable);

        connect(valueSelectBtn, SIGNAL(stateChanged(int)), this, SLOT(onInputCheckBoxStateChanged(int)));
        connect(varSelectBtn, SIGNAL(stateChanged(int)), this, SLOT(onInputCheckBoxStateChanged(int)));
        ++i;
    }

    i = 0;
    for (auto it = callingCyclogramVariables.begin(); it != callingCyclogramVariables.end(); ++it)
    {
        QString name = it.key();
        QLabel* text = new QLabel(mOutParams);
        text->setTextInteractionFlags(Qt::NoTextInteraction);
        text->setText(name);
        mOutParams->setCellWidget(i, 0, text);

        // "use variable input" button
        QCheckBox* varSelectBtn = new QCheckBox(mOutParams);
        mOutParams->setCellWidget(i, 1, varSelectBtn);

        // variable selector
        QComboBox* comboBox = new QComboBox(mOutParams);
        comboBox->addItem("");;
        comboBox->installEventFilter(this);
        for (auto iter = subprogramVariables.begin(); iter != subprogramVariables.end(); ++iter)
        {
            QString subName = iter.key();
            comboBox->addItem(subName);
        }

        //comboBox->addItems(callingCyclogramVariables.keys());
        mOutParams->setCellWidget(i, 2, comboBox);

        // "use direct value input" button
        QCheckBox* valueSelectBtn = new QCheckBox(mOutParams);
        mOutParams->setCellWidget(i, 3, valueSelectBtn);

        // variable selector
        QLineEdit* valueEdit = new QLineEdit(mOutParams);
        valueEdit->setValidator(new QDoubleValidator(mOutParams));
        valueEdit->setText("0");
        mOutParams->setCellWidget(i, 4, valueEdit);

        bool isVariable = true; // by default command output params are variables itself

        if (mCommand->loaded()) // already set command TODO
        {
            auto it = outputParams.find(name);
            if (it != outputParams.end())
            {
                if (it.value().type() == QVariant::String)
                {
                    int index = comboBox->findText(it.value().toString());
                    if (index != -1)
                    {
                        comboBox->setCurrentIndex(index);
                        isVariable = true;
                    }
                }
                else if (it.value().type() == QVariant::Double)
                {
                    valueEdit->setText(it.value().toString());
                    isVariable = false;
                }
            }
        }

        valueEdit->setEnabled(!isVariable);
        valueSelectBtn->setChecked(!isVariable);
        comboBox->setEnabled(isVariable);
        varSelectBtn->setChecked(isVariable);

        connect(valueSelectBtn, SIGNAL(stateChanged(int)), this, SLOT(onOutputCheckBoxStateChanged(int)));
        connect(varSelectBtn, SIGNAL(stateChanged(int)), this, SLOT(onOutputCheckBoxStateChanged(int)));
        ++i;
    }

    mInParams->resizeColumnsToContents();
    mOutParams->resizeColumnsToContents();

    if (mFileNameStr->text().isEmpty())
    {
        CyclogramManager::removeCyclogram(cyclogram);
    }
}

void CmdSubProgramEditDialog::updateTable(QTableWidget* widget, QCheckBox* changedBox, int state)
{
    for (int row = 0; row < widget->rowCount(); row++)
    {
        QCheckBox* varSelectBox = qobject_cast<QCheckBox*>(widget->cellWidget(row, 1));
        QCheckBox* valueSelectBox = qobject_cast<QCheckBox*>(widget->cellWidget(row, 3));

        if (varSelectBox == changedBox || valueSelectBox == changedBox)
        {
            QComboBox* varBox = qobject_cast<QComboBox*>(widget->cellWidget(row, 2));
            QLineEdit* valueEdit = qobject_cast<QLineEdit*>(widget->cellWidget(row, 4));

            varSelectBox->blockSignals(true);
            valueSelectBox->blockSignals(true);

            bool varBoxSelected = (varSelectBox == changedBox) && (state == Qt::Checked);
            bool valueEditSelected = (valueSelectBox == changedBox) && (state == Qt::Checked);
            bool varBoxUnselected = (varSelectBox == changedBox) && (state == Qt::Unchecked);
            bool valueEditUnselected = (valueSelectBox == changedBox) && (state == Qt::Unchecked);

            if (varSelectBox == changedBox)
            {
                valueSelectBox->setCheckState((state == Qt::Checked) ? Qt::Unchecked : Qt::Checked);
            }
            else if (valueSelectBox == changedBox)
            {
                varSelectBox->setCheckState((state == Qt::Checked) ? Qt::Unchecked : Qt::Checked);
            }

            varBox->setEnabled(varBoxSelected || valueEditUnselected);
            valueEdit->setEnabled(valueEditSelected || varBoxUnselected);

            varSelectBox->blockSignals(false);
            valueSelectBox->blockSignals(false);
            break;
        }
    }
}

void CmdSubProgramEditDialog::onInputCheckBoxStateChanged(int state)
{
    QCheckBox* changedBox = qobject_cast<QCheckBox*>(QObject::sender());
    if (!changedBox)
    {
        LOG_ERROR(QString("Widget not found"));
        return;
    }

    updateTable(mInParams, changedBox, state);
}

void CmdSubProgramEditDialog::onOutputCheckBoxStateChanged(int state)
{
    QCheckBox* changedBox = qobject_cast<QCheckBox*>(QObject::sender());
    if (!changedBox)
    {
        LOG_ERROR(QString("Widget not found"));
        return;
    }

    updateTable(mOutParams, changedBox, state);
}

bool CmdSubProgramEditDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Wheel && qobject_cast<QComboBox*>(obj))
    {
        return true; // do not process wheel events if combo box is not "expanded/opened"
    }
    else
    {
        return QObject::eventFilter(obj, event); // standard event processing
    }
}

void CmdSubProgramEditDialog::onShowExtendedSettings(bool checked)
{
    mShowExtendedSettings->setText(checked ? tr("Hide extended settings") : tr("Show extended settings"));
    mCyclogramDescription->setVisible(checked);
    mDescriptionHeader->setVisible(checked);
}
