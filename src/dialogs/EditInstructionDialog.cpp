#include "EditInstructionDialog.h"
#include "ui_EditInstructionDialog.h"
#include "core/Cutter.h"

#include <QCheckBox>

EditInstructionDialog::EditInstructionDialog(InstructionEditMode editMode, QWidget *parent)
    : QDialog(parent), ui(new Ui::EditInstructionDialog), editMode(editMode)
{
    ui->setupUi(this);
    ui->lineEdit->setMinimumWidth(400);
    ui->instructionLabel->setWordWrap(true);
    if (editMode == EDIT_TEXT) {
        ui->fillWithNops->setVisible(true);
        ui->fillWithNops->setCheckState(Qt::Checked);
    } else {
        ui->fillWithNops->setVisible(false);
    }
    setWindowFlags(windowFlags() & (~Qt::WindowContextHelpButtonHint));

    connect(ui->lineEdit, &QLineEdit::textEdited, this, &EditInstructionDialog::updatePreview);
}

EditInstructionDialog::~EditInstructionDialog() {}

void EditInstructionDialog::on_buttonBox_accepted()
{
    QString input = ui->lineEdit->text();
    QStringList instructionList = input.split(';', QString::SkipEmptyParts);

    RVA offset = Core()->getOffset();

    for (const QString &instruction : instructionList) {
        QString trimmedInstruction = instruction.trimmed();
        if (editMode == EDIT_BYTES) {
            QByteArray data = CutterCore::hexStringToBytes(trimmedInstruction);
            Core()->editBytes(offset, data.toHex());
            offset += data.size();
        } else if (editMode == EDIT_TEXT) {
            QByteArray data = Core()->assemble(trimmedInstruction);
            if (!data.isEmpty()) {
                Core()->editBytes(offset, data.toHex());
                offset += data.size();
            }
        }
    }

    accept();
}

void EditInstructionDialog::on_buttonBox_rejected()
{
    close();
}

bool EditInstructionDialog::needsNops() const
{
    if (editMode != EDIT_TEXT) {
        return false;
    }

    return ui->fillWithNops->checkState() == Qt::Checked;
}

QString EditInstructionDialog::getInstruction() const
{
    return ui->lineEdit->text();
}

void EditInstructionDialog::setInstruction(const QString &instruction)
{
    ui->lineEdit->setText(instruction);
    ui->lineEdit->selectAll();
    updatePreview(instruction);
}

void EditInstructionDialog::updatePreview(const QString &input)
{
    QString result;

    if (editMode == EDIT_NONE) {
        ui->instructionLabel->setText("");
        return;
    } else if (editMode == EDIT_BYTES) {
        QStringList bytesList = input.split(';', QString::SkipEmptyParts);
        QStringList disassembledInstructions;
        for (const QString &bytes : bytesList) {
            QByteArray data = CutterCore::hexStringToBytes(bytes.trimmed());
            QString disassembled = Core()->disassemble(data).replace('\n', "; ");
            disassembledInstructions.append(disassembled);
        }
        result = disassembledInstructions.join("\n");
    } else if (editMode == EDIT_TEXT) {
        QStringList instructionList = input.split(';', QString::SkipEmptyParts);
        QStringList assembledInstructions;
        for (const QString &instruction : instructionList) {
            QByteArray data = Core()->assemble(instruction.trimmed());
            QString assembled = CutterCore::bytesToHexString(data).trimmed();
            assembledInstructions.append(assembled);
        }
        result = assembledInstructions.join("\n");
    }

    if (result.isEmpty() || result.contains("invalid")) {
        ui->instructionLabel->setText("Unknown Instruction");
    } else {
        ui->instructionLabel->setText(result);
    }
}
