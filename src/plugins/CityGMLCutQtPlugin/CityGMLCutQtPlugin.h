// Copyright University of Lyon, 2012 - 2017
// Distributed under the GNU Lesser General Public License Version 2.1 (LGPLv2)
// (Refer to accompanying file LICENSE.md or copy at
//  https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html )

#ifndef CITYGMLCUTQTPLUGIN_H
#define CITYGMLCUTQTPLUGIN_H

#include "gui/pluginInterface.h"

#include <QStringList>
#include "Dialogs/DialogCityGMLCut1.h"

class CityGMLCutQtPlugin : 
  public QObject,
  public Generic_PluginInterface
{
    Q_OBJECT
    Q_INTERFACES(Generic_PluginInterface)	
#if(BUILD_GUI_QT5) // Refer to the end of .cxx for QT4 equivalent
    Q_PLUGIN_METADATA(IID "CityGMLCutQtPlugin")
#endif
        
    QStringList Generic_plugins() const
	{
		return QStringList() << "CityGMLCutQtPlugin";
	}

    bool Generic_plugin(const QString &plugin)
    {
		DialogCityGMLCut1 dial1;
		//dial1.setGMLFiles("C:\\Users\\noname\\Desktop\\cut\\MNT_CUT_TROUS\\Lyon03_MNT.gml", "outT\\bug5PLUG.gml");
		//dial1.setBoundingBox(1845500, 5174000, 1846000, 5174500);
		dial1.setVerbose(false);
		if (dial1.exec() == QDialog::Accepted)
		{
			QString in, out;
			dial1.getGMLFiles(in, out);

			unsigned int xmin,ymin,xmax,ymax;
			dial1.getBoundingBox(xmin,ymin,xmax,ymax);

			bool verbose;
			verbose = dial1.getVerbose();

			cmdline(in, out, xmin, ymin, xmax ,ymax, verbose);

			return true;
		}

		return false;
    }

private:
	int cmdline(QString s_IN, QString s_OUT, unsigned int ui_G_xmin, unsigned int ui_G_ymin, unsigned int ui_G_xmax, unsigned int ui_G_ymax, bool b_VERBOSE);
};

#endif // CITYGMLCUTQTPLUGIN_H
