#ifndef IMPORTCSVDIALOG_H
#define IMPORTCSVDIALOG_H

#include <QDialog>
#include <QVector>
#include "pointmanager.h"

class QTableView;
class QComboBox;
class QStandardItemModel;
class QCheckBox;

class ImportCsvDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ImportCsvDialog(const QString &filePath, QWidget *parent = nullptr);
    QVector<SurveyPoint> importedPoints() const { return m_points; }

private slots:
    void updatePreview();
    void accept() override;

private:
    QChar currentDelimiter() const;
    QStringList splitLine(const QString &line, QChar delim) const;
    void parseFile();

    QVector<SurveyPoint> m_points;
    QString m_filePath;

    QTableView *m_previewView {nullptr};
    QStandardItemModel *m_previewModel {nullptr};
    QComboBox *m_delimCombo {nullptr};
    QVector<QComboBox*> m_mappingCombos;
    QCheckBox *m_skipHeaderCheck {nullptr};
};

#endif // IMPORTCSVDIALOG_H
