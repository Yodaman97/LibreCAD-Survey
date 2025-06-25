#ifndef SURVEYTOOLS_H
#define SURVEYTOOLS_H

#include "qc_plugininterface.h"

class SurveyToolsPlugin : public QObject, QC_PluginInterface
{
    Q_OBJECT
    Q_INTERFACES(QC_PluginInterface)
    Q_PLUGIN_METADATA(IID LC_DocumentInterface_iid FILE "surveytools.json")

public:
    PluginCapabilities getCapabilities() const override;
    QString name() const override;
    void execComm(Document_Interface *doc, QWidget *parent, QString cmd) override;
};

#endif // SURVEYTOOLS_H
