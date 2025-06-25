#include "pointmanager.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QTextStream>
#include <QDebug>
#include "rs_vector.h"
#include "document_interface.h"
#include "importcsvdialog.h"
#include <utility>
#include <algorithm>
#include <QtMath>

// PointsModel implementation
PointsModel::PointsModel(QVector<SurveyPoint> &points, QObject *parent)
    : QAbstractTableModel(parent)
    , m_points(points)
{
}

int PointsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_points.size();
}

int PointsModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return ColumnCount;
}

QVariant PointsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole && role != Qt::EditRole)
        return {};

    const SurveyPoint &pt = m_points.at(index.row());
    switch (index.column()) {
    case Id: return pt.id;
    case Northing: return pt.northing;
    case Easting: return pt.easting;
    case Elevation: return pt.elevation;
    case Description: return pt.description;
    default: return {};
    }
}

bool PointsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    SurveyPoint &pt = m_points[index.row()];
    switch (index.column()) {
    case Id: pt.id = value.toString(); break;
    case Northing: pt.northing = value.toDouble(); break;
    case Easting: pt.easting = value.toDouble(); break;
    case Elevation: pt.elevation = value.toDouble(); break;
    case Description: pt.description = value.toString(); break;
    default: return false;
    }
    emit dataChanged(index, index);
    return true;
}

QVariant PointsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case Id: return tr("ID");
        case Northing: return tr("Northing");
        case Easting: return tr("Easting");
        case Elevation: return tr("Elevation");
        case Description: return tr("Description");
        default: break;
        }
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

Qt::ItemFlags PointsModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags fl = QAbstractTableModel::flags(index);
    if (index.isValid())
        fl |= Qt::ItemIsEditable;
    return fl;
}

void PointsModel::appendPoint(const SurveyPoint &point)
{
    const int row = m_points.size();
    beginInsertRows(QModelIndex(), row, row);
    m_points.append(point);
    endInsertRows();
}

void PointsModel::removePoint(int row)
{
    if (row < 0 || row >= m_points.size())
        return;
    beginRemoveRows(QModelIndex(), row, row);
    m_points.removeAt(row);
    endRemoveRows();
}

void PointsModel::refresh()
{
    beginResetModel();
    endResetModel();
}

PointManagerDialog::PointManagerDialog(Document_Interface *doc, QWidget *parent)
    : QDialog(parent)
    , m_doc(doc)
{
    setWindowTitle(tr("Point Manager"));

    m_table = new QTableView(this);
    m_model = new PointsModel(m_points, this);
    m_table->setModel(m_model);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(m_table, &QTableView::clicked,
            this, &PointManagerDialog::onRowSelected);
    
    m_points = {
        {"1", 100.0, 200.0, 10.0, "Point 1"},
        {"2", 110.0, 210.0, 11.5, "Point 2"},
        {"3", 120.0, 220.0, 12.1, "Point 3"},
        {"4", 130.0, 230.0, 13.2, "Point 4"},
        {"5", 140.0, 240.0, 14.0, "Point 5"}
    };
    m_model->refresh();

    auto *addBtn = new QPushButton(tr("Add Point"), this);
    auto *removeBtn = new QPushButton(tr("Remove Selected"), this);
    auto *exportBtn = new QPushButton(tr("Export to CSV"), this);
    auto *importBtn = new QPushButton(tr("Import CSV"), this);
    auto *syncBtn = new QPushButton(tr("Sync from Drawing"), this);

    connect(addBtn, &QPushButton::clicked, this, &PointManagerDialog::addPoint);
    connect(removeBtn, &QPushButton::clicked, this, &PointManagerDialog::removeSelected);
    connect(exportBtn, &QPushButton::clicked, this, &PointManagerDialog::exportCsv);
    connect(importBtn, &QPushButton::clicked, this, &PointManagerDialog::importCsv);
    connect(syncBtn, &QPushButton::clicked, this, &PointManagerDialog::syncFromDrawing);

    auto *btnLayout = new QHBoxLayout;
    btnLayout->addWidget(addBtn);
    btnLayout->addWidget(removeBtn);
    btnLayout->addWidget(exportBtn);
    btnLayout->addWidget(importBtn);
    btnLayout->addWidget(syncBtn);

    m_countLabel = new QLabel(this);
    auto *layout = new QVBoxLayout(this);
    layout->addWidget(m_table);
    layout->addWidget(m_countLabel);
    layout->addLayout(btnLayout);
    setLayout(layout);
    resize(600, 400);
    updateCount();
    if (m_doc) {
        const auto res = QMessageBox::question(this, tr("Sync"),
                                               tr("Load points from drawing?"));
        if (res == QMessageBox::Yes)
            syncFromDrawing();
    }
}

void PointManagerDialog::updateCount()
{
    if (m_countLabel)
        m_countLabel->setText(tr("Points: %1").arg(m_points.size()));
}

void PointManagerDialog::addPoint()
{
    int nextNum = m_points.isEmpty() ? 1 : m_points.size() + 1;
    SurveyPoint pt{QString::number(nextNum), 0.0, 0.0, 0.0, tr("New Point")};
    m_model->appendPoint(pt);
    updateCount();
    m_table->selectRow(m_model->rowCount() - 1);
}

void PointManagerDialog::removeSelected()
{
    auto indexes = m_table->selectionModel()->selectedRows();
    if (indexes.isEmpty()) {
        return;
    }
    int row = indexes.first().row();
    if (row >= 0) {
        m_model->removePoint(row);
        updateCount();
    }
}

void PointManagerDialog::exportCsv()
{
    const QString file = QFileDialog::getSaveFileName(this, tr("Export to CSV"),
                                                    QString(), tr("CSV Files (*.csv)"));
    if (file.isEmpty())
        return;
    QFile f(file);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Error"), tr("Cannot write file"));
        return;
    }
    QTextStream stream(&f);
    for (const SurveyPoint &pt : std::as_const(m_points)) {
        stream << pt.id << ',' << pt.northing << ',' << pt.easting << ','
               << pt.elevation << ',' << '"' << pt.description << "\n";
    }
    f.close();
}

void PointManagerDialog::importCsv()
{
    const QString file = QFileDialog::getOpenFileName(this, tr("Import CSV"), QString(), tr("CSV Files (*.csv);;All Files (*)"));
    if (file.isEmpty())
        return;
    ImportCsvDialog dlg(file, this);
    if (dlg.exec() != QDialog::Accepted)
        return;
    const QVector<SurveyPoint> pts = dlg.importedPoints();
    for (const SurveyPoint &pt : pts) {
        bool exists = std::any_of(m_points.begin(), m_points.end(), [&](const SurveyPoint &p){ return p.id == pt.id; });
        if (!exists) {
            m_model->appendPoint(pt);
            addEntitiesForPoint(pt);
        }
    }
    updateCount();
}

void PointManagerDialog::addEntitiesForPoint(const SurveyPoint &pt)
{
    if (!m_doc)
        return;
    m_doc->setLayer("SURVEY_PNTS");
    QPointF p(pt.easting, pt.northing);
    m_doc->addPoint(&p);
    if (!pt.id.isEmpty()) {
        QPointF off(p.x() + 0.5, p.y() + 0.5);
        m_doc->addText(pt.id, QString(), &off, 1.0, 0.0, DPI::HAlignLeft, DPI::VAlignBottom);
    }
    if (!pt.description.isEmpty()) {
        QPointF off(p.x() + 0.5, p.y() - 0.5);
        m_doc->addText(pt.description, QString(), &off, 1.0, 0.0, DPI::HAlignLeft, DPI::VAlignBottom);
    }
    if (!qFuzzyIsNull(pt.elevation)) {
        QPointF off(p.x() - 0.5, p.y() + 0.5);
        m_doc->addText(QString::number(pt.elevation), QString(), &off, 1.0, 0.0, DPI::HAlignRight, DPI::VAlignBottom);
    }
    m_doc->updateView();
}

void PointManagerDialog::syncFromDrawing()
{
    if (!m_doc)
        return;

    QList<Plug_Entity*> ents;
    if (!m_doc->getAllEntities(&ents, true))
        return;

    struct Label { RS_Vector pos; QString layer; QString text; };
    QVector<Label> labels;
    QVector<SurveyPoint> foundPts;

    for (Plug_Entity *e : ents) {
        QHash<int,QVariant> data;
        e->getData(&data);
        const int type = data.value(DPI::ETYPE).toInt();
        if (type == DPI::TEXT || type == DPI::MTEXT) {
            Label l{RS_Vector(data.value(DPI::STARTX).toDouble(),
                              data.value(DPI::STARTY).toDouble()),
                    data.value(DPI::LAYER).toString(),
                    data.value(DPI::TEXTCONTENT).toString()};
            labels.append(l);
        }
    }

    for (Plug_Entity *e : ents) {
        QHash<int,QVariant> data;
        e->getData(&data);
        if (data.value(DPI::ETYPE).toInt() != DPI::POINT)
            continue;
        QString layer = data.value(DPI::LAYER).toString();
        if (layer.compare("PNTS", Qt::CaseInsensitive) != 0)
            continue;

        SurveyPoint pt;
        pt.easting = data.value(DPI::STARTX).toDouble();
        pt.northing = data.value(DPI::STARTY).toDouble();
        RS_Vector pos(pt.easting, pt.northing);

        for (const Label &l : std::as_const(labels)) {
            if (pos.distanceTo(l.pos) > 3.0)
                continue;
            if (!l.layer.compare("PNTNO", Qt::CaseInsensitive)) {
                bool ok = false;
                int id = l.text.trimmed().toInt(&ok);
                if (ok)
                    pt.id = QString::number(id);
            } else if (!l.layer.compare("PNTELEV", Qt::CaseInsensitive)) {
                bool ok = false;
                double e = l.text.trimmed().toDouble(&ok);
                if (ok)
                    pt.elevation = e;
            } else if (!l.layer.compare("PNTDESC", Qt::CaseInsensitive)) {
                pt.description = l.text.trimmed();
            }
        }

        if (pt.id.isEmpty()) {
            qDebug() << "Skipping point without ID at" << pt.easting << pt.northing;
            continue;
        }
        foundPts.append(pt);
    }

    qDeleteAll(ents);

    if (foundPts.isEmpty())
        return;

    bool overwrite = false;
    if (!m_points.isEmpty()) {
        auto res = QMessageBox::question(this, tr("Overwrite existing IDs?"),
                                        tr("Overwrite points with same ID?"),
                                        QMessageBox::Yes | QMessageBox::No,
                                        QMessageBox::No);
        overwrite = res == QMessageBox::Yes;
    }

    for (const SurveyPoint &pt : std::as_const(foundPts)) {
        auto it = std::find_if(m_points.begin(), m_points.end(),
                               [&](const SurveyPoint &p) { return p.id == pt.id; });
        if (it != m_points.end()) {
            if (overwrite)
                *it = pt;
        } else {
            m_points.append(pt);
        }
    }

    m_model->refresh();
    updateCount();
}

void PointManagerDialog::onRowSelected(const QModelIndex &index)
{
    if (!index.isValid() || !m_doc)
        return;
    if (index.row() < 0 || index.row() >= m_points.size())
        return;

    const SurveyPoint &pt = m_points.at(index.row());
    QPointF center(pt.easting, pt.northing);
    m_doc->zoomToPoint(center, 10.0);
}
