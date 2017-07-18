// Copyright University of Lyon, 2012 - 2017
// Distributed under the GNU Lesser General Public License Version 2.1 (LGPLv2)
// (Refer to accompanying file LICENSE.md or copy at
//  https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html )

#ifndef CITYGMLFloodARQTPLUGIN_H
#define CITYGMLFloodARQTPLUGIN_H

#include "../gui/pluginInterface.h"

#include <QStringList>
#include "Dialogs/dialogFloodAR.hpp"

class CityGMLFloodARQtPlugin : 
  public QObject,
  public Generic_PluginInterface
{
    Q_OBJECT
    Q_INTERFACES(Generic_PluginInterface)	
#if(BUILD_GUI_QT5) // Refer to the end of .cxx for QT4 equivalent
    Q_PLUGIN_METADATA(IID "CityGMLFloodARQtPlugin")
#endif
        
    QStringList Generic_plugins() const
	{
		return QStringList() << "CityGMLFloodARQtPlugin";
	}

    bool Generic_plugin(const QString &plugin)
    {
		dialogFloodAR dial1;
		if (dial1.exec() == QDialog::Accepted)
		{
			your_code_here();

			return true;
		}

		return false;
    }

private:
	int your_code_here();
};

#endif // CITYGMLFloodARQTPLUGIN_H
