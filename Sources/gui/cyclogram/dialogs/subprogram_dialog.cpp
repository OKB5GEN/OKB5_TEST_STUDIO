#include <QtWidgets>

#include "Headers/gui/cyclogram/dialogs/subprogram_dialog.h"
#include "Headers/gui/cyclogram/cyclogram_widget.h"
#include "Headers/logic/commands/cmd_sub_program.h"
#include "Headers/logic/cyclogram.h"
#include "Headers/logic/variable_controller.h"
#include "Headers/logger/Logger.h"
#include "Headers/file_reader.h"
#include "Headers/file_writer.h"
#include "Headers/gui/tools/monitor_auto.h"
#include "Headers/gui/cyclogram/variables_window.h"

SubProgramDialog::SubProgramDialog(CmdSubProgram* command, QWidget * parent):
    QDialog(parent),
    mCommand(command),
    mVariablesWindow(Q_NULLPTR)
{
  //  setWindowTitle(tr("Edit sub program"));

  //  adjustSize();

    QString windowTitle = parent->windowTitle();
    if (!windowTitle.isEmpty())
    {
        windowTitle += QString(" -> ");
    }

    windowTitle += mCommand->text();
    setWindowTitle(windowTitle);

    mWidget = new CyclogramWidget(this);
    mWidget->load(mCommand->cyclogram());

    QVBoxLayout* layout = new QVBoxLayout(this);
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    layout->addLayout(buttonLayout);

    QPushButton* saveBtn = new QPushButton(QIcon(":/images/save"), tr("Save"), this);
    QPushButton* variablesBtn = new QPushButton(QIcon(":/images/variable"), tr("Variables"), this);
    QPushButton* chartBtn = new QPushButton(QIcon(":/images/monitor_auto"), tr("Chart"), this);
    QPushButton* deleteBtn = new QPushButton(QIcon(":/images/delete_all"), tr("Delete"), this);

    connect(saveBtn, SIGNAL(clicked(bool)), this, SLOT(onSaveClick()));
    connect(variablesBtn, SIGNAL(clicked(bool)), this, SLOT(onVariablesClick()));
    connect(chartBtn, SIGNAL(clicked(bool)), this, SLOT(onChartClick()));
    connect(deleteBtn, SIGNAL(clicked(bool)), mWidget, SLOT(deleteSelectedItem()));

    buttonLayout->addWidget(saveBtn);
    buttonLayout->addWidget(variablesBtn);
    buttonLayout->addWidget(chartBtn);
    buttonLayout->addWidget(deleteBtn);

    layout->addWidget(mWidget);
    setLayout(layout);
    setAttribute(Qt::WA_DeleteOnClose);
    resize(mWidget->size()); //TODO too big size case

    //TODO connect cyclogram & cyclogram widget signals
    // (file changed, can be saved, cyclogram command selected and can be deleted, variables window active)
    // QFileSystemWatcher - for outside file/directory changing detection
}

SubProgramDialog::~SubProgramDialog()
{

}

void SubProgramDialog::onSaveClick()
{
    QString fileName = Cyclogram::defaultStorePath() + mCommand->filePath();
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text))
    {
        QMessageBox::warning(this, tr("OKB5 Test Studio"),
                                   tr("Cannot write file %1:\n%2.").
                                   arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return;
    }

    FileWriter writer(mCommand->cyclogram());
    if (writer.writeFile(&file))
    {
        LOG_INFO(QString("File '%1' saved").arg(fileName));
        // save last save dir
        //QSettings settings;
        //QString savePath = QFileInfo(fileName).absoluteDir().path();
        //settings.setValue(SETTING_LAST_SAVE_FILE_DIR, savePath);

        //setCurrentFile(fileName);
        //statusBar()->showMessage(tr("File saved"), 2000);
        //emit documentSaved(true);
    }
}

void SubProgramDialog::onVariablesClick()
{
    if (!mVariablesWindow)
    {
        mVariablesWindow = new VariablesWindow(this);
        mVariablesWindow->setCyclogram(mCommand->cyclogram());
        mVariablesWindow->setWindowTitle(windowTitle());
    }

    mVariablesWindow->show();
}

void SubProgramDialog::onChartClick()
{
    MonitorAuto* dialog = new MonitorAuto(this);

    //mActiveMonitors.insert(dialog); //TODO разобраться с иерархией окон, ее обновлением при изменении файлов и т.д.

    //TODO remove copypaste from cyclogram widget
    QString windowTitle = this->windowTitle();
    if (!windowTitle.isEmpty())
    {
        windowTitle += QString(" -> ");
    }

    windowTitle += mCommand->text();
    dialog->setWindowTitle(windowTitle);

    //connect(dialog, SIGNAL(finished(int)), this, SLOT(onAutoMonitorClosed()));

    dialog->setCyclogram(mCommand->cyclogram());
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

/*
void CmdSubProgramEditDialog::setupUI()
{
    QGridLayout * layout = new QGridLayout(this);

    mSubprogramNameStr = new QLineEdit(tr("Sub"), this);
    mSubprogramNameStr->setMaximumWidth(100);

    // Cyclogram file
    QGroupBox* filePathBox = new QGroupBox(tr("Settings"), this);
    QHBoxLayout* fileNameLayout = new QHBoxLayout(filePathBox);
    mFileNameStr = new QLineEdit(filePathBox);
    mFileNameStr->setBackgroundRole(QPalette::Dark);
    QPushButton* browseButton = new QPushButton(filePathBox);
    connect(browseButton, SIGNAL(clicked(bool)), this, SLOT(openFile()));

    browseButton->setText(tr("Browse"));
    fileNameLayout->addWidget(new QLabel(tr("Name:"), this));
    fileNameLayout->addWidget(mSubprogramNameStr);
    fileNameLayout->addWidget(new QLabel(tr("File:"), this));
    fileNameLayout->addWidget(mFileNameStr);
    fileNameLayout->addWidget(browseButton);
    filePathBox->setLayout(fileNameLayout);

    layout->addWidget(filePathBox, 0, 0, 1, 2);

    // input and output params
    QGroupBox* inputGroupBox = new QGroupBox(tr("Input"), this);
    QGroupBox* outputGroupBox = new QGroupBox(tr("Output"), this);
    QVBoxLayout* inputLayout = new QVBoxLayout();
    QVBoxLayout* outputLayout = new QVBoxLayout();

    mInParams = new QTableWidget(this);
    QStringList inHeaders;
    inHeaders.append(tr("Вх.Переменная"));
    inHeaders.append(tr(""));
    inHeaders.append(tr("Переменная"));
    inHeaders.append(tr(""));
    inHeaders.append(tr("Значение"));
    mInParams->setColumnCount(inHeaders.size());
    mInParams->setHorizontalHeaderLabels(inHeaders);
    inputLayout->addWidget(mInParams);

    mOutParams = new QTableWidget(this);
    QStringList outHeaders;
    outHeaders.append(tr("Вых.Переменная"));
    outHeaders.append(tr(""));
    outHeaders.append(tr("Переменная"));
    outHeaders.append(tr(""));
    outHeaders.append(tr("Значение"));
    mOutParams->setColumnCount(outHeaders.size());
    mOutParams->setHorizontalHeaderLabels(outHeaders);
    outputLayout->addWidget(mOutParams);

    inputGroupBox->setLayout(inputLayout);
    outputGroupBox->setLayout(outputLayout);

    layout->addWidget(inputGroupBox, 1, 0);
    layout->addWidget(outputGroupBox, 1, 1);

    // Dialog button box >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel , Qt::Horizontal, this);
    layout->addWidget(buttonBox, 2, 1);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(onAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    setLayout(layout);
}

void CmdSubProgramEditDialog::setCommand(CmdSubProgram* command, Cyclogram* cyclogram)
{
    mCommand = command;
    mCallingCyclogram = cyclogram;

    if (mCommand)
    {
        mSubprogramNameStr->setText(mCommand->name());
        mFileNameStr->setText(mCommand->filePath());
        updateUI();
    }
}

void CmdSubProgramEditDialog::onAccept()
{
    if (mCommand)
    {
        mCommand->setFilePath(mFileNameStr->text());
        mCommand->setName(mSubprogramNameStr->text());

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
    }

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

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open cyclogram file"), path, tr("OKB5 Cyclogram Files (*.cgr)"));
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
    if (!mCallingCyclogram)
    {
        LOG_ERROR(QString("No calling cyclogram set to subprogram '%1'").arg(mSubprogramNameStr->text()));
        return;
    }

    QSharedPointer<Cyclogram> cyclogram;
    cyclogram.reset(new Cyclogram(Q_NULLPTR));

    if (!mFileNameStr->text().isEmpty()) // some cyclogram file name exist, load cyclogram from file
    {
        QString fileName = Cyclogram::defaultStorePath() + mFileNameStr->text();

        QFile file(fileName);
        FileReader reader(cyclogram.data());

        if (!file.open(QFile::ReadOnly | QFile::Text))
        {
            LOG_ERROR(QString("Cannot read file %1: %2").arg(QDir::toNativeSeparators(fileName), file.errorString()));
            return;
        }

        if (!reader.read(&file))
        {
            LOG_ERROR(QString("Parse error in file %1: %2").arg(QDir::toNativeSeparators(fileName), reader.errorString()));
            return;
        }
    }
    else // no cyclogram file set, possibly just batch calling cyclogram variables changing
    {
        cyclogram->createDefault();
    }

    mInParams->clearContents();
    mOutParams->clearContents();

    const QMap<QString, VariableController::VariableData>& subprogramVariables = cyclogram->variableController()->variablesData();
    const QMap<QString, VariableController::VariableData>& callingCyclogramVariables = mCallingCyclogram->variableController()->variablesData();

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
        QString name = mCommand->subprogramPrefix() + it.key();
        QLabel* text = new QLabel(mInParams);
        text->setTextInteractionFlags(Qt::NoTextInteraction);
        text->setText(name);
        mInParams->setCellWidget(i, 0, text);

        // "use variable input" button
        QCheckBox* varSelectBtn = new QCheckBox(mInParams);
        mInParams->setCellWidget(i, 1, varSelectBtn);

        // variable selector
        QComboBox* comboBox = new QComboBox(mInParams);
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
        for (auto iter = subprogramVariables.begin(); iter != subprogramVariables.end(); ++iter)
        {
            QString subName = mCommand->subprogramPrefix() + iter.key();
            comboBox->addItem(subName);
        }

        comboBox->addItems(callingCyclogramVariables.keys());
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
            else
            {
                int index = comboBox->findText(name);
                if (index != -1)
                {
                    comboBox->setCurrentIndex(index);
                }
            }
        }
        else // set output variable value to itself by default if command not loaded
        {
            int index = comboBox->findText(name);
            if (index != -1)
            {
                comboBox->setCurrentIndex(index);
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

    mInParams->resizeColumnToContents(1);
    mInParams->resizeColumnToContents(3);

    mOutParams->resizeColumnToContents(1);
    mOutParams->resizeColumnToContents(3);
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
*/