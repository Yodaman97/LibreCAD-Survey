#ifndef POINTMANAGER_H
#define POINTMANAGER_H

#include <QDialog>
#include <QAbstractTableModel>
#include <QTableView>
#include <QVector>
#include <QLabel>
#include <QString>
#include "document_interface.h"

struct SurveyPoint {
    QString id;
    double northing;
    double easting;
    double elevation;
    QString description;
};

class PointsModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit PointsModel(QVector<SurveyPoint> &points, QObject *parent = nullptr);

    enum Columns { Id, Northing, Easting, Elevation, Description, ColumnCount };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void appendPoint(const SurveyPoint &point);
    void removePoint(int row);
    void refresh();

private:
    QVector<SurveyPoint> &m_points;
};

class PointManagerDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PointManagerDialog(Document_Interface *doc, QWidget *parent = nullptr);

private slots:
    void addPoint();
    void removeSelected();
    void exportCsv();
    void importCsv();
    void syncFromDrawing();
    void onRowSelected(const QModelIndex &index);

private:
    void updateCount();
    void addEntitiesForPoint(const SurveyPoint &pt);

    QTableView *m_table {nullptr};
    PointsModel *m_model {nullptr};
    QLabel *m_countLabel {nullptr};
    QVector<SurveyPoint> m_points;
    Document_Interface *m_doc {nullptr};
};

#endif // POINTMANAGER_H
