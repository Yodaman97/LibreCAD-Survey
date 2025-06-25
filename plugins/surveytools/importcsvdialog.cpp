#include "importcsvdialog.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFile>
#include <QGridLayout>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QTableView>
#include <QTextStream>

ImportCsvDialog::ImportCsvDialog(const QString &filePath, QWidget *parent)
    : QDialog(parent)
    , m_filePath(filePath)
{
    setWindowTitle(tr("Import CSV"));

    m_delimCombo = new QComboBox(this);
    m_delimCombo->addItem(tr("Comma (,)", ","));
    m_delimCombo->addItem(tr("Semicolon (;)", ";"));
    m_delimCombo->addItem(tr("Tab (\t)", "\t"));
    m_delimCombo->addItem(tr("Space ( )", " ")); 

    m_skipHeaderCheck = new QCheckBox(tr("Skip first row"), this);

    m_previewModel = new QStandardItemModel(this);
    m_previewView = new QTableView(this);
    m_previewView->setModel(m_previewModel);
    m_previewView->horizontalHeader()->setStretchLastSection(true);

    connect(m_delimCombo, &QComboBox::currentTextChanged, this, &ImportCsvDialog::updatePreview);
    connect(m_skipHeaderCheck, &QCheckBox::toggled, this, &ImportCsvDialog::updatePreview);

    auto *btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(btnBox, &QDialogButtonBox::accepted, this, &ImportCsvDialog::accept);
    connect(btnBox, &QDialogButtonBox::rejected, this, &ImportCsvDialog::reject);

    auto *layout = new QVBoxLayout(this);
    auto *optLayout = new QHBoxLayout;
    optLayout->addWidget(m_delimCombo);
    optLayout->addWidget(m_skipHeaderCheck);
    layout->addLayout(optLayout);
    layout->addWidget(m_previewView);
    layout->addWidget(btnBox);

    updatePreview();
}

QChar ImportCsvDialog::currentDelimiter() const
{
    return m_delimCombo->currentData().toChar();
}

QStringList ImportCsvDialog::splitLine(const QString &line, QChar delim) const
{
    if (delim == '\t')
        return line.split('\t');
    if (delim == ' ')
        return line.split(' ', Qt::SkipEmptyParts);
    return line.split(delim);
}

void ImportCsvDialog::updatePreview()
{
    m_previewModel->clear();
    QFile f(m_filePath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QTextStream ts(&f);
    QChar delim = currentDelimiter();
    const bool skipHeader = m_skipHeaderCheck->isChecked();
    int maxRows = 10;
    int row = 0;
    while (!ts.atEnd() && row < maxRows) {
        QString line = ts.readLine();
        if (skipHeader && row == 0) {
            row++;
            continue;
        }
        QStringList cols = splitLine(line, delim);
        if (m_previewModel->columnCount() < cols.size())
            m_previewModel->setColumnCount(cols.size());
        QList<QStandardItem*> items;
        for (const QString &c : cols) {
            items << new QStandardItem(c);
        }
        m_previewModel->appendRow(items);
        row++;
    }
    // setup mapping combos if not yet
    if (m_mappingCombos.isEmpty() && m_previewModel->columnCount() > 0) {
        auto *grid = new QGridLayout;
        QStringList fields{tr("Ignore"), tr("Point ID"), tr("Northing"), tr("Easting"), tr("Elevation"), tr("Description")};
        for (int i=0;i<m_previewModel->columnCount();++i) {
            QComboBox *cb = new QComboBox(this);
            cb->addItems(fields);
            m_mappingCombos.append(cb);
            grid->addWidget(new QLabel(tr("Column %1").arg(i+1), this), 0, i);
            grid->addWidget(cb, 1, i);
        }
        layout()->setMenuBar(nullptr);
        layout()->insertLayout(1, grid);
    }
}

void ImportCsvDialog::parseFile()
{
    m_points.clear();
    QFile f(m_filePath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QTextStream ts(&f);
    QChar delim = currentDelimiter();
    const bool skipHeader = m_skipHeaderCheck->isChecked();
    int row = 0;
    while (!ts.atEnd()) {
        QString line = ts.readLine();
        if (skipHeader && row == 0) {
            row++;
            continue;
        }
        row++;
        QStringList cols = splitLine(line, delim);
        SurveyPoint pt;
        bool ok = true;
        for (int i=0;i<m_mappingCombos.size() && i<cols.size();++i) {
            int map = m_mappingCombos[i]->currentIndex();
            if (map==0) continue; // ignore
            const QString &val = cols[i].trimmed();
            switch (map) {
            case 1: pt.id = val; break;
            case 2: pt.northing = val.toDouble(&ok); break;
            case 3: pt.easting = val.toDouble(&ok); break;
            case 4: pt.elevation = val.toDouble(&ok); break;
            case 5: pt.description = val; break;
            default: break;
            }
            if (!ok) break;
        }
        if (ok && !pt.id.isEmpty())
            m_points.append(pt);
    }
}

void ImportCsvDialog::accept()
{
    parseFile();
    QDialog::accept();
}

