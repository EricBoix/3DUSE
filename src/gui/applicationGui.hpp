// Copyright University of Lyon, 2012 - 2017
// Distributed under the GNU Lesser General Public License Version 2.1 (LGPLv2)
// (Refer to accompanying file LICENSE.md or copy at
//  https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html )

////////////////////////////////////////////////////////////////////////////////
#ifndef __APPLICATIONGUI_HPP__
#define __APPLICATIONGUI_HPP__
////////////////////////////////////////////////////////////////////////////////
#include <QTextBrowser>
#include "core/application.hpp"
#include "controllerGui.hpp"
#include "osg/osgScene.hpp"
#include "moc/treeView.hpp"
////////////////////////////////////////////////////////////////////////////////
//class MainWindow;
class PickHandler;
////////////////////////////////////////////////////////////////////////////////
/// \brief The ApplicationGui class
/// Stores Common gui application data
class ApplicationGui : public vcity::Application
{
public:
    ApplicationGui();
    virtual ~ApplicationGui() override;

    ControllerGui& getControllerGui();
    void setControllerGui(ControllerGui* cont);

    osg::ref_ptr<OsgScene> getOsgScene();
    void setOsgScene(osg::ref_ptr<OsgScene> scene);

    PickHandler* getPickHandler();

    TreeView* getTreeView();
    void setTreeView(TreeView* treeview);

    QTextBrowser* getTextBrowser();
    void setTextBowser(QTextBrowser* textBrowser);

    MainWindow* getMainWindow();
    void setMainWindow(MainWindow* window);

private:
    ControllerGui* m_controllerGui;     ///< controller, needs to be allocated outside
    osg::ref_ptr<OsgScene> m_osgScene;  ///< osg scene (for rendering)
    TreeView* m_treeView;               ///< Qt treeview
    QTextBrowser* m_textBrowser;        ///< Text browser to wrtite msg
    MainWindow* m_mainWindow;       ///< Main window
};
////////////////////////////////////////////////////////////////////////////////
ApplicationGui& appGui();
////////////////////////////////////////////////////////////////////////////////
#endif // __APPLICATIONGUI_HPP__
