// Copyright University of Lyon, 2012 - 2017
// Distributed under the GNU Lesser General Public License Version 2.1 (LGPLv2)
// (Refer to accompanying file LICENSE.md or copy at
//  https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html )

////////////////////////////////////////////////////////////////////////////////
#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP
////////////////////////////////////////////////////////////////////////////////
#include <QMainWindow>
#include <QTreeWidgetItem>
#include <QResizeEvent>
#include <QDateTime>
#include "gui/applicationGui.hpp"
#include "gui/osg/osgQtWidget.hpp"
#include "gui/moc/treeView.hpp"
#include "gui/osg/osgScene.hpp"
#include "gui/osg/osgPicking.hpp"
#include "core/scene.hpp"

#include <QDir>
#include <QActionGroup>
////////////////////////////////////////////////////////////////////////////////
namespace Ui {
    class MainWindow;
}
////////////////////////////////////////////////////////////////////////////////
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = 0);
    ~MainWindow();

private:
public:
    Ui::MainWindow* m_ui;   ///< Qt ui
    osgQtWidget* m_osgView; ///< osgQt view widget
    PickHandler* m_pickhandler;
    TreeView* m_treeView;   ///< Qt treeview

    ApplicationGui m_app;
    osg::ref_ptr<OsgScene> m_osgScene;    ///< osg scene (for rendering)

    int m_forceLod;         ///< -1: auto, 0 1 2 3 4 : lod x
    int m_pickingMode;      ///< 0: face, 1: building
    bool m_useTemporal;     ///< use temporal slider (also used for citygml temporal export)
    bool m_temporalAnim;    ///< temporal animation ? (play button clicked ?)
    int m_unlockLevel;      ///< admin mode (to hide dev menus)

    QTimer m_timer;         ///< anim timer
    QDateTime m_currentDate;	///< current date on the temporal slider

    void initTemporalTools();

    void addRecentFile(const QString& filepath);
    void removeRecentFile(const QString& filepath);
    void updateRecentFiles();
    bool loadFile(const QString& filepath);
    bool loadCSV(const QString& CSVfilepath, const QString &DIRfilepath);

    void reset();           ///< reset ui : treeview, text box...

    void updateTextBox(const std::stringstream& ss);
    void updateTextBox(const vcity::URI& uri);
    void updateTextBoxWithSelectedNodes();

    // admin mode (show all menus)
    void unlockFeatures(const QString& pass);

    void ChangecheckBoxTemporalToolsState();

    QLineEdit* getFilter();


signals:
   // Signal emitted when HorizontalSlider (time slider) value changes.
   // It is for instance used in Sunlight Plugin (visualisation part)
   void timeSliderChanged(QDateTime d);

private:
    // plugins
    QDir pluginsDir;
    QStringList pluginFileNames;
    QMenu *pluginMenu;
    QAction *aboutPluginsAct;

    void loadPlugins();
    void populateMenus(QObject *plugin);
    void addToMenu(QObject *plugin, const QStringList &texts, QMenu *menu,
        const char *member, QActionGroup *actionGroup = 0);

    void loadShpFile(const QString& filepath);
    // plugins

    public slots:
    void loadScene();
    void loadSceneRecursive();
    void loadSceneBBox();
    void loadDocuments();
    void resetScene();
    void clearSelection();
    void optionPickBuiling();
    void optionPickFace();
    void optionInfoBubbles();
    void optionShadow();
    void optionSkybox();
    void slotSettings();
    void optionShowTemporalTools();
    void optionShowAdvancedTools();
    void clearRecentFiles(bool removeAll = true);
    void updateTemporalParams(int value = -1);
    void toggleUseTemporal();
    void updateTemporalSlider();
    void exportCityGML();
    void exportOsg();
    void exportOsga();
    void exportJSON();
    void exportOBJ();
    void exportOBJsplit();
    void debugDumpOsg();
    void slotDumpScene();
    void slotDumpSelectedNodes();
    void slotCutCityGMLwithShapefile();
    void slotSplitCityGMLBuildings();
    void slotTilingCityGML();
    void slotBuildBuildingAABBs();
    void slotCutMNTwithShapefile();
    void slotCreateRoadOnMNT();
    void slotCreateVegetationOnMNT();
    void TilingCityGML( QString CityGMLPath, std::string OutputPath, int TileX, int TileY, std::string Suffix, bool AssignOrCut );
    void slotExtractFlatRoofs();

    void about();

    void test1();
    void test2();
    void test3();
    void test4();
    void test5();

    private slots:
    void openRecentFile();

    void generateAllLODs();
    void generateLOD0();
    void generateLOD1();
    void generateLOD0OnFile();
    void generateLOD1OnFile();
    void generateLOD2();
    void generateLOD3();
    void generateLOD4();

    void slotFixBuilding();
    void slotObjToCityGML();

    void slotChangeDetection();

    void slotOptimOSG();

    void slotRenderLOD0();
    void slotRenderLOD1();
    void slotRenderLOD2();
    void slotRenderLOD3();
    void slotRenderLOD4();

    void slotTemporalAnim();
    void slotTemporalAnimUpdate();

    // plugins
    void applyPlugin();
    void aboutPlugins();
    // plugins
};
////////////////////////////////////////////////////////////////////////////////
#endif // MAINWINDOW_HPP
