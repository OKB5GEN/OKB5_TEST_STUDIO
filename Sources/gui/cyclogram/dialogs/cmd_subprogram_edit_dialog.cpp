#include <QtWidgets>

#include "Headers/gui/cyclogram/dialogs/cmd_subprogram_edit_dialog.h"
#include "Headers/logic/commands/cmd_sub_program.h"
#include "Headers/logic/cyclogram.h"
#include "Headers/logic/variable_controller.h"
#include "Headers/logger/Logger.h"
#include "Headers/file_reader.h"

CmdSubProgramEditDialog::CmdSubProgramEditDialog(QWidget * parent):
    QDialog(parent),
    mCommand(Q_NULLPTR),
    mCallingCyclogram(Q_NULLPTR)
{
    setupUI();
    setWindowTitle(tr("Edit sub program"));

    adjustSize();
    //setFixedSize(sizeHint());
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
    inHeaders.append(tr("Вх.Параметр"));
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

void CmdSubProgramEditDialog::setCommand(CmdSubProgram* command)
{
    mCommand = command;

    if (mCommand)
    {
        mSubprogramNameStr->setText(mCommand->name());
        mFileNameStr->setText(mCommand->filePath());
    }
}

//void CmdSubProgramEditDialog::updateComponent(int operand, QComboBox* box, QLineEdit* lineEdit, QRadioButton* boxBtn, QRadioButton* lineEditBtn)
//{

//}

void CmdSubProgramEditDialog::onAccept()
{
    if (mCommand)
    {
        mCommand->setFilePath(mFileNameStr->text());
        mCommand->setName(mSubprogramNameStr->text());
    }

    accept();
}

//void CmdSubProgramEditDialog::onCheckBoxStateChanged(int state)
//{
//    mOperationBox->setEnabled(state == Qt::Checked);
//    mOperand2Box->setEnabled(state == Qt::Checked);
//}

//void CmdSubProgramEditDialog::onOper1VarBtnStateChanged(bool toggled)
//{
//    mOper1Box->setEnabled(toggled);
//    mOper1Num->setEnabled(!toggled);

//    mOper1NumBtn->blockSignals(true);
//    mOper1NumBtn->setChecked(false);
//    mOper1NumBtn->blockSignals(false);
//}

//void CmdSubProgramEditDialog::onOper1NumBtnStateChanged(bool toggled)
//{
//    mOper1Box->setEnabled(!toggled);
//    mOper1Num->setEnabled(toggled);

//    mOper1VarBtn->blockSignals(true);
//    mOper1VarBtn->setChecked(false);
//    mOper1VarBtn->blockSignals(false);
//}

//void CmdSubProgramEditDialog::onOper2VarBtnStateChanged(bool toggled)
//{
//    mOper2Box->setEnabled(toggled);
//    mOper2Num->setEnabled(!toggled);

//    mOper2NumBtn->blockSignals(true);
//    mOper2NumBtn->setChecked(false);
//    mOper2NumBtn->blockSignals(false);
//}

//void CmdSubProgramEditDialog::onOper2NumBtnStateChanged(bool toggled)
//{
//    mOper2Box->setEnabled(!toggled);
//    mOper2Num->setEnabled(toggled);

//    mOper2VarBtn->blockSignals(true);
//    mOper2VarBtn->setChecked(false);
//    mOper2VarBtn->blockSignals(false);
//}

void CmdSubProgramEditDialog::openFile()
{
    QString path;
    QString currentFileName = mFileNameStr->text();
    if (!currentFileName.isEmpty())
    {
        if (QFileInfo(currentFileName).exists())
        {
            path = QFileInfo(currentFileName).absoluteDir().path();
        }
        else
        {
            LOG_WARNING(QString("Corrupted file reference '%1' detected. Set path to current application directory path").arg(currentFileName));
        }
    }

    if (path.isEmpty())
    {
        path = QDir::currentPath();
    }

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open cyclogram file"), path, tr("OKB5 Cyclogram Files (*.cgr)"));
    if (!fileName.isEmpty())
    {
        //TODO пока не очень понятно какие пути юзать абсолютные или относительные. Пока сделаны абсолютные
        //абсолютный понятнее и копипастить его проще, но при переносе на другой комп ссылки на подпрограммы могут поехать (надо как-то решить)
        // относительный безопаснее при переносе между машинами, но нечитабельный (от какой директории идет отсчет? по идее от директории приложения)
        // по идее все циклограммы должны храниться в папке/подпапках приложения (или при выборе файла, он копируется в папку приложения и читается уже оттуда)

        //QString relativePath = QDir::current().relativeFilePath(fileName);
        mFileNameStr->setText(fileName);
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

    QString fileName = mFileNameStr->text();

    QSharedPointer<Cyclogram> cyclogram;
    cyclogram.reset(new Cyclogram(Q_NULLPTR));

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

    mInParams->clearContents(); //TODO possibly remove all children?
    mOutParams->clearContents(); //TODO possibly remove all children?

    const QMap<QString, VariableController::VariableData>& subprogramVariables = cyclogram->variableController()->variablesData();
    const QMap<QString, VariableController::VariableData>& callingCyclogramVariables = mCallingCyclogram->variableController()->variablesData();

    int inCount = subprogramVariables.size();
    int outCount = callingCyclogramVariables.size();
    mInParams->setRowCount(inCount);
    mOutParams->setRowCount(outCount);

    //mInParams->setVisible(inCount > 0); // TODO no variables in subprogram?
    //mOutParams->setVisible(outCount > 0); // TODO no variables in calling cyclogram?

    int i = 0;
    for (auto it = subprogramVariables.begin(); it != subprogramVariables.end(); ++it)
    {
        // param name
        QString name = mSubprogramNameStr->text() + "." + it.key();
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

        bool isVariable = false; // by default command input params are directly set values

        if (mCommand->loaded()) // already set command (load variables mapping from command) TODO
        {
//            auto it = inputParams.find(name);
//            if (it != inputParams.end())
//            {
//                if (it.value().type() == QMetaType::QString)
//                {
//                    int index = comboBox->findText(it.value().toString());
//                    if (index != -1)
//                    {
//                        comboBox->setCurrentIndex(index);
//                        isVariable = true;
//                    }
//                }
//                else if (it.value().type() == QMetaType::Double)
//                {
//                    valueEdit->setText(it.value().toString());
//                }
//            }
        }

        connect(valueSelectBtn, SIGNAL(stateChanged(int)), this, SLOT(onCheckBoxStateChanged(int)));
        connect(varSelectBtn, SIGNAL(stateChanged(int)), this, SLOT(onCheckBoxStateChanged(int)));

        valueEdit->setEnabled(!isVariable);
        valueSelectBtn->setChecked(!isVariable);
        comboBox->setEnabled(isVariable);
        varSelectBtn->setChecked(isVariable);

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

        QLabel* equalSign = new QLabel(mOutParams);
        equalSign->setTextInteractionFlags(Qt::NoTextInteraction);
        equalSign->setText("=");
        mOutParams->setCellWidget(i, 1, equalSign);

        QComboBox* comboBox = new QComboBox(mOutParams);
        for (auto iter = subprogramVariables.begin(); iter != subprogramVariables.end(); ++iter)
        {
            QString subName = mSubprogramNameStr->text() + "." + iter.key();
            comboBox->addItem(subName);
        }

        comboBox->addItems(callingCyclogramVariables.keys());

        mOutParams->setCellWidget(i, 2, comboBox);

        if (mCommand->loaded()) // already set command TODO
        {
//            auto it = outputParams.find(name);
//            if (it != outputParams.end())
//            {
//                int index = comboBox->findText(it.value().toString()); // output params are always variables
//                if (index != -1) // add default variable name, corresponding to this paramID
//                {
//                    comboBox->setCurrentIndex(index);
//                }
//            }
        }
        else // set output variable value to itself
        {
            int index = comboBox->findText(name); // output params are always variables
            if (index != -1) // add default variable name, corresponding to this paramID
            {
                comboBox->setCurrentIndex(index);
            }
        }

        ++i;
    }

    mInParams->resizeColumnToContents(1);
    mInParams->resizeColumnToContents(3);

    mOutParams->resizeColumnToContents(1);
}

void CmdSubProgramEditDialog::setCallingCyclogram(Cyclogram* cyclogram)
{
    mCallingCyclogram = cyclogram;
}

void CmdSubProgramEditDialog::onCheckBoxStateChanged(int state)
{
    QCheckBox* changedBox = qobject_cast<QCheckBox*>(QObject::sender());
    if (!changedBox)
    {
        LOG_ERROR(QString("Widget not found"));
        return;
    }

    for (int row = 0; row < mInParams->rowCount(); row++)
    {
        QCheckBox* varSelectBox = qobject_cast<QCheckBox*>(mInParams->cellWidget(row, 1));
        QComboBox* varBox = qobject_cast<QComboBox*>(mInParams->cellWidget(row, 2));
        QCheckBox* valueSelectBox = qobject_cast<QCheckBox*>(mInParams->cellWidget(row, 3));
        QLineEdit* valueEdit = qobject_cast<QLineEdit*>(mInParams->cellWidget(row, 4));

        if (varSelectBox == changedBox || valueSelectBox == changedBox)
        {
            varSelectBox->blockSignals(true);
            valueSelectBox->blockSignals(true);

            bool varBoxSelected = (varSelectBox == changedBox) && (state == Qt::Checked);
            bool valueEditSelected = (valueSelectBox == changedBox) && (state == Qt::Checked);
            bool varBoxUnselected = (valueSelectBox == changedBox) && (state == Qt::Unchecked);
            bool valueEditUnselected = (varSelectBox == changedBox) && (state == Qt::Unchecked);

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
