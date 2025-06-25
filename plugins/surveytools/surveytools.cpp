#include "surveytools.h"
#include "pointmanager.h"

QString SurveyToolsPlugin::name() const
{
    return tr("Survey Tools");
}

PluginCapabilities SurveyToolsPlugin::getCapabilities() const
{
    PluginCapabilities pluginCapabilities;
    pluginCapabilities.menuEntryPoints << PluginMenuLocation("Surveying", tr("Point Manager"));
    return pluginCapabilities;
}

void SurveyToolsPlugin::execComm(Document_Interface *doc, QWidget *parent, [[maybe_unused]] QString cmd)
{
    PointManagerDialog dlg(doc, parent);
    dlg.exec();
}
