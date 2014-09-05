// -*-c++-*- VCity project, 3DUSE, Liris, 2013, 2014
////////////////////////////////////////////////////////////////////////////////
#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP
////////////////////////////////////////////////////////////////////////////////
#include <QMainWindow>
#include <QTreeWidgetItem>
#include <QResizeEvent>
#include "gui/applicationGui.hpp"
#include "gui/osg/osgQtWidget.hpp"
#include "gui/moc/treeView.hpp"
#include "gui/osg/osgScene.hpp"
#include "gui/osg/osgPicking.hpp"
#include "core/scene.hpp"
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

    void addRecentFile(const QString& filepath);
    void removeRecentFile(const QString& filepath);
    void updateRecentFiles();
    bool loadFile(const QString& filepath);

    void reset();           ///< reset ui : treeview, text box...

    void updateTextBox(const std::stringstream& ss);
    void updateTextBox(const vcity::URI& uri);
    //const std::stringstream& genSelectedNodes
    void updateTextBoxWithSelectedNodes();

    // admin mode (show all menus)
    void unlockFeatures(const QString& pass);

    QLineEdit* getFilter();

public slots:
    void loadScene();
    void loadSceneRecursive();
    void loadSceneBBox();
    void resetScene();
    void clearSelection();
    void optionPickBuiling();
    void optionPickFace();
    void optionInfoBubbles();
    void optionShadow();
    void slotSettings();
    void optionShowTemporalTools();
    void optionShowAdvancedTools();
    void clearRecentFiles(bool removeAll = true);
    void updateTemporalParams(int value=-1);
    void toggleUseTemporal();
    void exportCityGML();
    void exportOsg();
    void exportOsga();
    void exportJSON();
    void exportOBJ();
    void exportOBJsplit();
    void debugDumpOsg();
    void slotDumpScene();
    void slotDumpSelectedNodes();

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
};
////////////////////////////////////////////////////////////////////////////////
#endif // MAINWINDOW_HPP
