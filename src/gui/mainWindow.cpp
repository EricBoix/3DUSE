////////////////////////////////////////////////////////////////////////////////
#include "moc/mainWindow.hpp"
#include "ui_mainWindow.h"
#include "moc/dialogLoadBBox.hpp"
#include "moc/dialogSettings.hpp"
#include "moc/dialogAbout.hpp"

#include "controllerGui.hpp"

#include <QFileDialog>
#include <QCheckBox>
#include <QDirIterator>
#include <QSettings>
#include <QListView>
#include <QMessageBox>
#include <QDate>

#include "citygml.hpp"
#include "export/exportCityGML.hpp"
#include "export/exportJSON.hpp"
#include "export/exportOBJ.hpp"

#include "gui/osg/osgScene.hpp"
#include "gdal_priv.h"
#include "cpl_conv.h" // for CPLMalloc()
#include "ogrsf_frmts.h"
#include "osg/osgGDAL.hpp"

#include "core/BatimentShape.hpp"
#include <geos/geom/GeometryFactory.h>

/*#include "assimp/Importer.hpp"
#include "assimp/PostProcess.h"
#include "assimp/Scene.h"*/

#include "osg/osgAssimp.hpp"

#include "osg/osgMnt.hpp"
////////////////////////////////////////////////////////////////////////////////

geos::geom::Geometry* ShapeGeo = NULL;
std::vector<std::pair<double, double>> Hauteurs;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), m_ui(new Ui::MainWindow), m_useTemporal(false), m_temporalAnim(false), m_unlockLevel(0)
{
    m_ui->setupUi(this);

    m_app.setMainWindow(this);

    // create Qt treeview
    m_treeView = new TreeView(m_ui->treeWidget, this);
    m_app.setTreeView(m_treeView);
    m_app.setTextBowser(m_ui->textBrowser);

    // create controller
    m_app.setControllerGui(new ControllerGui());

    // create osgQt view widget
    m_osgView = new osgQtWidget(m_ui->mainGrid);
    m_pickhandler = new PickHandler();
    m_osgView->setPickHandler(m_pickhandler);
    m_ui->mainGridLayout->addWidget(m_osgView->getWidget(), 0, 0);

    // create osg scene
    m_osgScene = new OsgScene();
    m_app.setOsgScene(m_osgScene);

    // setup osgQt view
    m_osgView->setSceneData(m_osgScene);

    // init gdal
    GDALAllRegister();
    OGRRegisterAll();

    // connect slots
    connect(m_ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));
    connect(m_ui->actionLoad, SIGNAL(triggered()), this, SLOT(loadScene()));
    connect(m_ui->actionLoad_recursive, SIGNAL(triggered()), this, SLOT(loadSceneRecursive()));
    //connect(m_ui->actionLoad_bbox, SIGNAL(triggered()), this, SLOT(loadSceneBBox()));
    connect(m_ui->actionExport_citygml, SIGNAL(triggered()), this, SLOT(exportCityGML()));
    connect(m_ui->actionExport_osg, SIGNAL(triggered()), this, SLOT(exportOsg()));
    connect(m_ui->actionExport_tiled_osga, SIGNAL(triggered()), this, SLOT(exportOsga()));
    connect(m_ui->actionExport_JSON, SIGNAL(triggered()), this, SLOT(exportJSON()));
    connect(m_ui->actionExport_OBJ, SIGNAL(triggered()), this, SLOT(exportOBJ()));
    connect(m_ui->actionExport_OBJ_split, SIGNAL(triggered()), this, SLOT(exportOBJsplit()));
    //connect(m_ui->actionDelete_node, SIGNAL(triggered()), this, SLOT(deleteNode()));
    connect(m_ui->actionReset, SIGNAL(triggered()), this, SLOT(resetScene()));
    connect(m_ui->actionClearSelection, SIGNAL(triggered()), this, SLOT(clearSelection()));
    connect(m_ui->actionBuilding, SIGNAL(triggered()), this, SLOT(optionPickBuiling()));
    connect(m_ui->actionFace, SIGNAL(triggered()), this, SLOT(optionPickFace()));
    connect(m_ui->actionInfo_bubbles, SIGNAL(triggered()), this, SLOT(optionInfoBubbles()));
    connect(m_ui->actionShadows, SIGNAL(triggered()), this, SLOT(optionShadow()));
    connect(m_ui->actionSettings, SIGNAL(triggered()), this, SLOT(slotSettings()));
    //connect(m_ui->actionAdd_Tag, SIGNAL(triggered()), this, SLOT(optionAddTag()));
    //connect(m_ui->actionAdd_Flag, SIGNAL(triggered()), this, SLOT(optionAddFlag()));
    connect(m_ui->actionShow_temporal_tools, SIGNAL(triggered()), this, SLOT(optionShowTemporalTools()));
    connect(m_ui->checkBoxTemporalTools, SIGNAL(clicked()), this, SLOT(toggleUseTemporal()));
    connect(m_ui->actionShow_advanced_tools, SIGNAL(triggered()), this, SLOT(optionShowAdvancedTools()));
    //connect(m_ui->treeWidget, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(handleTreeView(QTreeWidgetItem*, int)));
    connect(m_ui->horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(updateTemporalParams(int)));
    connect(m_ui->horizontalSlider, SIGNAL(sliderReleased()), this, SLOT(updateTemporalParams()));
    //connect(m_ui->buttonBrowserTemporal, SIGNAL(clicked()), this, SLOT(toggleUseTemporal()));
    connect(m_ui->actionDump_osg, SIGNAL(triggered()), this, SLOT(debugDumpOsg()));
    connect(m_ui->actionDump_scene, SIGNAL(triggered()), this, SLOT(slotDumpScene()));
    connect(m_ui->actionDump_selected_nodes, SIGNAL(triggered()), this, SLOT(slotDumpSelectedNodes()));
    connect(m_ui->actionAbout, SIGNAL(triggered()), this, SLOT(about()));
    connect(m_ui->actionOptim_osg, SIGNAL(triggered()), this, SLOT(slotOptimOSG()));

    connect(m_ui->toolButton, SIGNAL(clicked()), this, SLOT(slotTemporalAnim()));

    // render lod signals
    connect(m_ui->actionForce_LOD0, SIGNAL(triggered()), this, SLOT(slotRenderLOD0()));
    connect(m_ui->actionForce_LOD1, SIGNAL(triggered()), this, SLOT(slotRenderLOD1()));
    connect(m_ui->actionForce_LOD2, SIGNAL(triggered()), this, SLOT(slotRenderLOD2()));
    connect(m_ui->actionForce_LOD3, SIGNAL(triggered()), this, SLOT(slotRenderLOD3()));
    connect(m_ui->actionForce_LOD4, SIGNAL(triggered()), this, SLOT(slotRenderLOD4()));

    // generate LODs signals
    connect(m_ui->actionAll_LODs, SIGNAL(triggered()), this, SLOT(generateAllLODs()));
    connect(m_ui->actionLOD0, SIGNAL(triggered()), this, SLOT(generateLOD0()));
    connect(m_ui->actionLOD1, SIGNAL(triggered()), this, SLOT(generateLOD1()));
    connect(m_ui->actionLOD2, SIGNAL(triggered()), this, SLOT(generateLOD2()));
    connect(m_ui->actionLOD3, SIGNAL(triggered()), this, SLOT(generateLOD3()));
    connect(m_ui->actionLOD4, SIGNAL(triggered()), this, SLOT(generateLOD4()));

    connect(m_ui->actionFix_building, SIGNAL(triggered()), this, SLOT(slotFixBuilding()));

    connect(m_ui->actionTest_1, SIGNAL(triggered()), this, SLOT(test1()));
    connect(m_ui->actionTest_2, SIGNAL(triggered()), this, SLOT(test2()));
    connect(m_ui->actionTest_3, SIGNAL(triggered()), this, SLOT(test3()));
    connect(m_ui->actionTest_4, SIGNAL(triggered()), this, SLOT(test4()));
    connect(m_ui->actionTest_5, SIGNAL(triggered()), this, SLOT(test5()));

    // filter search
    connect(m_ui->filterButton, SIGNAL(clicked()), m_treeView, SLOT(slotFilter()));

    connect(&m_timer, SIGNAL(timeout()), this, SLOT(slotTemporalAnimUpdate()));

    m_ui->horizontalSlider->setEnabled(m_useTemporal);
    m_ui->dateTimeEdit->setEnabled(m_useTemporal);
    m_ui->toolButton->setEnabled(m_useTemporal);

    reset();

    updateRecentFiles();

    m_treeView->init();

    //m_ui->statusBar->showMessage("none");
    
    setlocale(LC_ALL, "C"); // MT : important for Linux
}
////////////////////////////////////////////////////////////////////////////////
MainWindow::~MainWindow()
{
    delete m_osgView;
    delete m_ui;
}
////////////////////////////////////////////////////////////////////////////////
// Recent files
////////////////////////////////////////////////////////////////////////////////
void MainWindow::addRecentFile(const QString& filepath)
{
    QSettings settings("liris", "virtualcity");
    QStringList list = settings.value("recentfiles").toStringList();

    list.removeAll(filepath);
    list.prepend(filepath);
    if(list.size() >= 10)
    {
        list.removeLast();
    }

    settings.setValue("recentfiles", list);
}
////////////////////////////////////////////////////////////////////////////////
void MainWindow::removeRecentFile(const QString& filepath)
{
    QSettings settings("liris", "virtualcity");
    QStringList list = settings.value("recentfiles").toStringList();
    list.removeAll(filepath);
    settings.setValue("recentfiles", list);
}
////////////////////////////////////////////////////////////////////////////////
void MainWindow::updateRecentFiles()
{
    QSettings settings("liris", "virtualcity");
    QStringList list = settings.value("recentfiles").toStringList();
    list.removeDuplicates();
    settings.setValue("recentfiles", list);

    clearRecentFiles(false);

    foreach(QString str, list)
    {
        QAction* action = new QAction(str, this);
        m_ui->menuRecent_files->addAction(action);
        connect(action, SIGNAL(triggered()), this, SLOT(openRecentFile()));
        action->setData(str);
    }

    // add reset item last
    QAction* action = new QAction("Clear", this);
    m_ui->menuRecent_files->addAction(action);
    connect(action, SIGNAL(triggered()), this, SLOT(clearRecentFiles()));
}
////////////////////////////////////////////////////////////////////////////////
void MainWindow::clearRecentFiles(bool removeAll)
{
    m_ui->menuRecent_files->clear();

    if(removeAll)
    {
        QSettings settings("liris", "virtualcity");
        settings.setValue("recentfiles", QStringList());
    }
}
////////////////////////////////////////////////////////////////////////////////
void MainWindow::openRecentFile()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if(action)
    {
        loadFile(action->data().toString());
        updateRecentFiles();
    }
}
////////////////////////////////////////////////////////////////////////////////
// Open files
////////////////////////////////////////////////////////////////////////////////
bool MainWindow::loadFile(const QString& filepath)
{
    // date check
    if(QDate::currentDate() > QDate(2015, 01, 01))
    {
        QMessageBox(QMessageBox::Critical,  "Error", "Expired").exec();
        return false;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    QSettings settings("liris", "virtualcity");
    QFileInfo file(filepath);

    if(!file.exists())
    {
        removeRecentFile(filepath);
        QApplication::restoreOverrideCursor();
        return false;
    }

    QString ext = file.suffix().toLower();
    if(ext == "citygml" || ext == "gml")
    {
        std::cout << "load citygml file : " << filepath.toStdString() << std::endl;

        // add tile
        vcity::Tile* tile = new vcity::Tile(filepath.toStdString());
        vcity::URI uriLayer = m_app.getScene().getDefaultLayer("LayerCityGML")->getURI();
        vcity::log() << uriLayer.getStringURI() << "\n";
        appGui().getControllerGui().addTile(uriLayer, *tile);

        addRecentFile(filepath);

        /* QStringList list = settings.value("recentfiles").toStringList();
         list.append(filepath);
         settings.setValue("recentfiles", list);*/
    }
	// Assimp importer
	else if(ext == "assimp" || ext ==  "dae" || ext ==  "blend" || ext ==  "3ds" || ext ==  "ase" || ext ==  "obj" || ext ==  "xgl" || ext ==  "ply" || ext ==  "dxf" || ext ==  "lwo" || ext ==  "lws" ||
		ext == "lxo" || ext ==  "stl" || ext ==  "x" || ext ==  "ac" || ext ==  "ms3d" || ext ==  "scn" || ext ==  "xml" || ext ==  "irrmesh" || ext ==  "irr" ||
		ext == "mdl" || ext ==  "md2" || ext ==  "md3" || ext ==  "pk3" || ext ==  "md5" || ext ==  "smd" || ext ==  "m3" || ext ==  "3d" || ext ==  "q3d" || ext ==  "off" || ext ==  "ter")
    {
		/*Assimp::Importer importer;
		const aiScene *scene = importer.ReadFile(filepath.toStdString(), aiProcessPreset_TargetRealtime_Fast); // aiProcessPreset_TargetRealtime_Quality
 
		aiMesh *mesh = scene->mMeshes[0]; // assuming you only want the first mesh*/

		// ---

		std::string readOptionString = "";
		osg::ref_ptr<osgDB::Options> readOptions = new osgDB::Options(readOptionString.c_str());
		ReadResult readResult = readNode(filepath.toStdString(), readOptions);
		if (readResult.success())
		{
			osg::ref_ptr<osg::Node> node = readResult.getNode();

			// set assimpNode name
			static int id = 0;
			std::stringstream ss;
			ss << "assimpNode" << id++;
			node->setName(ss.str());

			vcity::URI uriLayer = m_app.getScene().getDefaultLayer("LayerAssimp")->getURI();
			vcity::log() << uriLayer.getStringURI() << "\n";
			appGui().getControllerGui().addAssimpNode(uriLayer, node);

			addRecentFile(filepath);
		}
	}
	// MntAsc importer
	else if(ext ==  "asc")
    {
		MNT mnt;

		if (mnt.charge(filepath.toStdString().c_str(), "ASC"))
		{
            osg::ref_ptr<osg::Node> node = mnt.buildAltitudesGrid(-m_app.getSettings().getDataProfile().m_offset.x, -m_app.getSettings().getDataProfile().m_offset.y);

			// set mntAscNode name
			static int id = 0;
			std::stringstream ss;
			ss << "mntAscNode" << id++;
			node->setName(ss.str());

			vcity::URI uriLayer = m_app.getScene().getDefaultLayer("LayerMnt")->getURI();
			vcity::log() << uriLayer.getStringURI() << "\n";
			appGui().getControllerGui().addMntAscNode(uriLayer, node);

			addRecentFile(filepath);

			//mnt.sauve_log(std::string("mntAsc.txt").c_str(), std::string("mntAsc.tga").c_str()); // mntAsc.tga bidon
			//mnt.sauve_partie(std::string("mntAsc_partie.txt").c_str(), 0, 0, mnt.get_dim_x(), mnt.get_dim_y());
			//mnt.sauve_partie_XML(std::string("mntAsc_partie_xml.txt").c_str(), 0, 0, mnt.get_dim_x(), mnt.get_dim_y());
		}
	}
    else if(ext == "shp")
    {
        std::cout << "load shp file : " << filepath.toStdString() << std::endl;
        OGRDataSource* poDS = OGRSFDriverRegistrar::Open(filepath.toStdString().c_str(), FALSE);
		
        m_osgScene->m_layers->addChild(buildOsgGDAL(poDS));

		buildGeosShape(poDS, &ShapeGeo, &Hauteurs);
        if(poDS)
        {
            vcity::URI uriLayer = m_app.getScene().getDefaultLayer("LayerShp")->getURI();
            appGui().getControllerGui().addShpNode(uriLayer, poDS);

            //addRecentFile(filepath);

            //m_osgScene->m_layers->addChild(buildOsgGDAL(poDS));
        }

		//OGRSFDriverRegistrar::GetRegistrar()->ReleaseDataSource(poDS);
    }
    else if(ext == "dxf")
    {
        std::cout << "load dxf file : " << filepath.toStdString() << std::endl;
        OGRDataSource* poDS = OGRSFDriverRegistrar::Open(filepath.toStdString().c_str(), FALSE);

        m_osgScene->m_layers->addChild(buildOsgGDAL(poDS));
    }
    else if(ext == "ecw")
    {
        std::cout << "load ecw file : " << filepath.toStdString() << std::endl;
        GDALDataset* poDataset = (GDALDataset*) GDALOpen( filepath.toStdString().c_str(), GA_ReadOnly );
        if( poDataset == NULL )
        {
            std::cout << "load ecw file : " << filepath.toStdString() << std::endl;
        }
    }
    else
    {
        std::cout << "bad file : " << filepath.toStdString() << std::endl;
        QApplication::restoreOverrideCursor();
        return false;
    }

    QApplication::restoreOverrideCursor();

    return true;
}
////////////////////////////////////////////////////////////////////////////////
void MainWindow::loadScene()
{
    m_osgView->setActive(false);

    std::cout<<"Load Scene"<<std::endl;

    QSettings settings("liris", "virtualcity");
    QString lastdir = settings.value("lastdir").toString();
    QStringList filenames = QFileDialog::getOpenFileNames(this, "Load scene files", lastdir);

    for(int i = 0; i < filenames.count(); ++i)
    {
        QFileInfo file(filenames[i]);
        bool success = loadFile(file.absoluteFilePath());
        if(success)
        {
            // save path
            QFileInfo file(filenames[i]);
            //std::cout << "lastdir : " << file.dir().absolutePath().toStdString() << std::endl;
            settings.setValue("lastdir", file.dir().absolutePath());
        }
    }
    //std::cout << "lastdir set : " << settings.value("lastdir").toString().toStdString() << std::endl;

    updateRecentFiles();

    m_osgView->setActive(true);
}
////////////////////////////////////////////////////////////////////////////////
void buildRecursiveFileList(const QDir& dir, QStringList& list)
{
    QDirIterator iterator(dir.absolutePath(), QDirIterator::Subdirectories);
    while(iterator.hasNext())
    {
        iterator.next();
        if(!iterator.fileInfo().isDir())
        {
            QString filename = iterator.filePath();
            if(filename.endsWith(".citygml", Qt::CaseInsensitive) || filename.endsWith(".gml", Qt::CaseInsensitive))
            {
                list.append(filename);
                qDebug("Found %s matching pattern.", qPrintable(filename));
            }
        }
    }
}
////////////////////////////////////////////////////////////////////////////////
void MainWindow::loadSceneRecursive()
{
    m_osgView->setActive(false);

    //std::cout<<"Load Scene recursive"<<std::endl;
    /*QSettings settings("liris", "virtualcity");
    QString lastdir = settings.value("lastdir").toString();

    QString dirpath = QFileDialog::getExistingDirectory(0, "Load scene files : choose a directory", lastdir, 0);
    //std::cout << "dir : " << dirpath.toStdString() << std::endl;
    if(dirpath != "")
    {
        QDir dir(dirpath);
        QStringList files;
        buildRecursiveFileList(dir, files);

        for(int i = 0; i < files.count(); ++i)
        {
            loadFile(files[i]);
        }

        settings.setValue("lastdir", dirpath);
    }

    //std::cout << "lastdir set : " << settings.value("lastdir").toString().toStdString() << std::endl;

    updateRecentFiles();*/


    QFileDialog w;
    w.setFileMode(QFileDialog::Directory);
    //w.setFileMode(QFileDialog::AnyFile);
    //w.setOption(QFileDialog::DontUseNativeDialog,false);
    QStringList filters;
    filters.append("Any type (*.gml *.citygml *.xml *.shp)");
    filters.append("Citygml files (*.gml *.citygml)");
    filters.append("Xml files (*.xml)");
    filters.append("Shape files (*.shp)");
    filters.append("Any files (*)");
    w.setNameFilters(filters);
    /*QListView *l = w.findChild<QListView*>("listView");
    if(l)
    {
        l->setSelectionMode(QAbstractItemView::MultiSelection);
    }*/
    QTreeView *t = w.findChild<QTreeView*>();
    if(t)
    {
        t->setSelectionMode(QAbstractItemView::MultiSelection);
    }
    if(w.exec())
    {
        QStringList file = w.selectedFiles();
        foreach (QString s, file)
        {
            QFileInfo file(s);
            // do some useful stuff here
            std::cout << "Working on file " << s.toStdString() << " type : " << file.isDir()<< ", " << file.isFile() << std::endl;
            if(file.isDir())
            {
                QDir dir(s);
                QStringList files;
                buildRecursiveFileList(dir, files);

                for(int i = 0; i < files.count(); ++i)
                {
                    loadFile(files[i]);
                }
            }
            else if(file.isFile())
            {
                loadFile(s);
            }
            else
            {
                // fail
            }
        }
    }

    m_osgView->setActive(true);
}
////////////////////////////////////////////////////////////////////////////////
void MainWindow::loadSceneBBox()
{
    DialogLoadBBox diag;
    diag.exec();
}
////////////////////////////////////////////////////////////////////////////////
void MainWindow::updateTextBox(const std::stringstream& ss)
{
    m_ui->textBrowser->setText(ss.str().c_str());
}
////////////////////////////////////////////////////////////////////////////////
void MainWindow::updateTextBox(const vcity::URI& uri)
{
    std::stringstream ss;
    ss << uri.getStringURI() << std::endl;

    citygml::CityObject* obj = vcity::app().getScene().getCityObjectNode(uri);
    if(obj)
    {
        ss << "ID : " << obj->getId() << std::endl;
        ss << "Type : " << obj->getTypeAsString() << std::endl;
        //obj->l
        ss << "Temporal : " << obj->isTemporal() << std::endl;

        ss << "Attributes : " << std::endl;
        citygml::AttributesMap attribs = obj->getAttributes();
        citygml::AttributesMap::const_iterator it = attribs.begin();
        while ( it != attribs.end() )
        {
            ss << " + " << it->first << ": " << it->second << std::endl;
            ++it;
        }

        // get textures
        // parse geometry
        std::vector<citygml::Geometry*>& geoms = obj->getGeometries();
        std::vector<citygml::Geometry*>::iterator itGeom = geoms.begin();
        for(; itGeom != geoms.end(); ++itGeom)
        {
            // parse polygons
            std::vector<citygml::Polygon*>& polys = (*itGeom)->getPolygons();
            std::vector<citygml::Polygon*>::iterator itPoly = polys.begin();
            for(; itPoly != polys.end(); ++itPoly)
            {
                citygml::LinearRing* ring = (*itPoly)->getExteriorRing();
                std::vector<TVec3d>& vertices = ring->getVertices();
                std::vector<TVec3d>::iterator itVertices = vertices.begin();
                ss << "Linear ring (" << (*itGeom)->getId() << " : " << (*itPoly)->getId() << ") : ";
                for(; itVertices != vertices.end(); ++itVertices)
                {
                    // do stuff with points...
                    TVec3d point = *itVertices;
                    ss << point;
                }
                ss << std::endl;

                ss << "Texcoords : ";
                citygml::TexCoords texCoords = (*itPoly)->getTexCoords();
                for(citygml::TexCoords::const_iterator itTC = texCoords.begin(); itTC < texCoords.end(); ++itTC)
                {
                    ss << *itTC;
                }
                ss << std::endl;

                const citygml::Texture* tex = (*itPoly)->getTexture();
                if(tex)
                {
                    ss << "Texture (" << (*itGeom)->getId() << " : " << (*itPoly)->getId() << ") : " << tex->getUrl() << std::endl;
                }
            }

        }

        //m_pickhandler->resetPicking();
        //m_pickhandler->addNodePicked(uri);
    }

    updateTextBox(ss);
}
////////////////////////////////////////////////////////////////////////////////
void MainWindow::updateTextBoxWithSelectedNodes()
{

}
////////////////////////////////////////////////////////////////////////////////
void MainWindow::unlockFeatures(const QString& pass)
{
    if(pass == "pass1")
    {
        m_unlockLevel = 1;
    }
    else if(pass == "pass2")
    {
        m_unlockLevel = 2;
    }
    else
    {
        m_unlockLevel = 0;
    }

    switch(m_unlockLevel)
    {
    case 2:
        m_ui->menuDebug->menuAction()->setVisible(true);
        m_ui->menuTest->menuAction()->setVisible(true);
        m_ui->menuTools->menuAction()->setVisible(true);
        m_ui->menuRender->menuAction()->setVisible(true);
        m_ui->actionExport_osg->setVisible(true);
        m_ui->actionExport_tiled_osga->setVisible(true);
        m_ui->actionLoad_bbox->setVisible(true);
        m_ui->actionLoad_recursive->setVisible(true);
        m_ui->actionShow_advanced_tools->setVisible(true);
        m_ui->actionHelp->setVisible(true);
        //m_ui->tab_16->setVisible(true);
        //break; // missing break on purpose
    case 1:
        m_ui->hsplitter_bottom->setVisible(true);
        m_ui->widgetTemporal->setVisible(true);
        m_ui->actionShow_temporal_tools->setVisible(true);
        break;
    case 0:
        m_ui->menuDebug->menuAction()->setVisible(false);
        m_ui->menuTest->menuAction()->setVisible(false);
        m_ui->menuTools->menuAction()->setVisible(false);
        m_ui->menuRender->menuAction()->setVisible(false);
        m_ui->actionExport_osg->setVisible(false);
        m_ui->actionExport_tiled_osga->setVisible(false);
        m_ui->actionLoad_bbox->setVisible(false);
        m_ui->actionLoad_recursive->setVisible(false);
        m_ui->actionShow_advanced_tools->setVisible(false);
        m_ui->actionHelp->setVisible(false);
        m_ui->tab_16->setVisible(false); m_ui->tabWidget->removeTab(1);
        m_ui->widgetTemporal->setVisible(false);
        m_ui->hsplitter_bottom->setVisible(false);
        m_ui->actionShow_temporal_tools->setVisible(false);
        break;
    default:
        break;
    }
}
////////////////////////////////////////////////////////////////////////////////
QLineEdit* MainWindow::getFilter()
{
    return m_ui->filterLineEdit;
}
////////////////////////////////////////////////////////////////////////////////
// Tools
////////////////////////////////////////////////////////////////////////////////
void MainWindow::reset()
{
    // reset text box
    m_ui->textBrowser->clear();
    unlockFeatures("pass2");
    m_ui->mainToolBar->hide();
    //m_ui->statusBar->hide();
}
////////////////////////////////////////////////////////////////////////////////
void MainWindow::resetScene()
{
    // reset scene
    m_app.getScene().reset();

    // reset osg scene
    m_osgScene->reset();

    // reset ui
    reset();
}
////////////////////////////////////////////////////////////////////////////////
void MainWindow::clearSelection()
{
    appGui().getControllerGui().resetSelection();
}
////////////////////////////////////////////////////////////////////////////////
void MainWindow::optionPickBuiling()
{
    m_ui->actionFace->setChecked(false);
    m_ui->actionBuilding->setChecked(true);

    std::cout << "pick building" << std::endl;
    m_osgView->getPickHandler()->setPickingMode(1);
}
////////////////////////////////////////////////////////////////////////////////
void MainWindow::optionPickFace()
{
    m_ui->actionBuilding->setChecked(false);
    m_ui->actionFace->setChecked(true);

    std::cout << "pick face" << std::endl;
    m_osgView->getPickHandler()->setPickingMode(0);
}
////////////////////////////////////////////////////////////////////////////////
void MainWindow::optionInfoBubbles()
{

}
////////////////////////////////////////////////////////////////////////////////
void MainWindow::optionShadow()
{
    bool v = m_ui->actionShadows->isChecked();
    m_osgScene->setShadow(v);

    std::cout << "toggle shadow" << std::endl;
}
////////////////////////////////////////////////////////////////////////////////
void MainWindow::slotSettings()
{
    DialogSettings diag;
    diag.doSettings();
}
////////////////////////////////////////////////////////////////////////////////
void MainWindow::optionShowTemporalTools()
{
    if(m_ui->actionShow_temporal_tools->isChecked())
    {
        m_ui->hsplitter_bottom->show();
    }
    else
    {
        m_ui->hsplitter_bottom->hide();
    }

    std::cout << "show temporal tools" << std::endl;
}
////////////////////////////////////////////////////////////////////////////////
void MainWindow::optionShowAdvancedTools()
{
    /*bool v = m_ui->actionShow_advanced_tools->isChecked();
    if(v)
    {
        m_ui->menuDebug->show();
    }
    else
    {
        m_ui->menuDebug->hide();
    }*/
}
////////////////////////////////////////////////////////////////////////////////
void MainWindow::updateTemporalParams(int value)
{
    if(value == -1) value = m_ui->horizontalSlider->value();
    QDate date(1900, 1, 1);
    date = date.addDays(value);
    //m_ui->buttonBrowserTemporal->setText(date.toString());
    m_ui->dateTimeEdit->setDate(date);

    //std::cout << "set year : " << date.year() << std::endl;

    QDateTime datetime(date);
    if(m_useTemporal)   m_osgScene->setDate(datetime);
}
////////////////////////////////////////////////////////////////////////////////
void MainWindow::toggleUseTemporal()
{
    m_useTemporal = !m_useTemporal;

    if(m_useTemporal)
    {
        QDate date(1900, 1, 1);
        date = date.addDays(m_ui->horizontalSlider->value());
        QDateTime datetime(date);
        m_osgScene->setDate(datetime);
    }
    else
    {
        QDate date(0, 0, 0);
        QDateTime datetime(date);
        m_osgScene->setDate(datetime); // reset
        m_timer.stop();
    }

    m_ui->horizontalSlider->setEnabled(m_useTemporal);
    m_ui->dateTimeEdit->setEnabled(m_useTemporal);
    m_ui->toolButton->setEnabled(m_useTemporal);

    std::cout << "toggle temporal tool" << std::endl;
}
////////////////////////////////////////////////////////////////////////////////
void MainWindow::exportCityGML()
{
    QString filename = QFileDialog::getSaveFileName();

    citygml::ExporterCityGML exporter;

    // check temporal params
    if(m_useTemporal)
    {
        exporter.setTemporalExport(true);
        exporter.setDate(m_ui->dateTimeEdit->dateTime());
    }

    // check if something is picked
    //const std::set<std::string>& nodes = m_pickhandler->getNodesPicked(); // TODO : update this with a uri list
    const std::vector<vcity::URI>& uris = appGui().getSelectedNodes();
    if(uris.size() > 0)
    {
        std::cout << "Citygml export cityobject : " << uris[0].getStringURI() << std::endl;
        std::vector<citygml::CityObject*> objs;
        for(const vcity::URI& uri : uris)
        {
            citygml::CityObject* obj = m_app.getScene().getCityObjectNode(uris[0]); // use getNode
            if(obj) objs.push_back(obj);
        }
        //citygml::exportCitygml(model, "test.citygml");
        exporter.exportCityObject(objs, filename.toStdString());
    }
    else
    {
        std::cout << "Citygml export citymodel" << std::endl;
        // use first tile
		vcity::LayerCityGML* layer = dynamic_cast<vcity::LayerCityGML*>(m_app.getScene().getDefaultLayer("LayerCityGML"));
        citygml::CityModel* model = layer->getTiles()[0]->getCityModel();
        //citygml::exportCitygml(model, "test.citygml");
        exporter.exportCityModel(*model, filename.toStdString());
    }
}
////////////////////////////////////////////////////////////////////////////////
void MainWindow::exportOsg()
{
    osg::ref_ptr<osg::Node> node = m_osgScene->m_layers;
    bool res = osgDB::writeNodeFile(*node, "scene.osg");
    std::cout << "export osg : " << res << std::endl;
}
////////////////////////////////////////////////////////////////////////////////
void MainWindow::exportOsga()
{
    osg::ref_ptr<osg::Node> node = m_osgScene;

    //osg::ref_ptr<osgDB::Archive> archive = osgDB::openArchive("scene.osga", osgDB::Archive::CREATE);
    //osg::ref_ptr<osgDB::ReaderWriter> rw =
    //osg::ref_ptr<osgDB::ReaderWriter::ReadResult> res = osgDB::ReaderWriter::openArchive("scene.osga", osgDB::ReaderWriter::CREATE);
    //osg::ref_ptr<osgDB::Archive> archive = res->getArchive();
    osgDB::writeNodeFile(*node, "scene.osga");

    /*if(archive.valid())
    {
        archive->writeNode(*node, "none");
    }
    archive->close();*/
}
////////////////////////////////////////////////////////////////////////////////
void MainWindow::exportJSON()
{
    QString filename = QFileDialog::getSaveFileName();
    QFileInfo fileInfo(filename);
    filename = fileInfo.path() + "/" + fileInfo.baseName();
    citygml::ExporterJSON exporter;

    const std::vector<vcity::URI>& uris = appGui().getSelectedNodes();
    if(uris.size() > 0)
    {
        if(uris[0].getType() == "Tile")
        {
            citygml::CityModel* model = m_app.getScene().getTile(uris[0])->getCityModel();
            if(model) exporter.exportCityModel(*model, filename.toStdString(), "test");
        }
        else
        {
            // only tiles
        }
    }
}
////////////////////////////////////////////////////////////////////////////////
void MainWindow::exportOBJ()
{
    QString filename = QFileDialog::getSaveFileName();
    QFileInfo fileInfo(filename);
    filename = fileInfo.path() + "/" + fileInfo.baseName();
    citygml::ExporterOBJ exporter;
    exporter.setOffset(m_app.getSettings().getDataProfile().m_offset.x, m_app.getSettings().getDataProfile().m_offset.y);

    const std::vector<vcity::URI>& uris = appGui().getSelectedNodes();
    if(uris.size() > 0)
    {
        if(uris[0].getType() == "Tile")
        {
            citygml::CityModel* model = m_app.getScene().getTile(uris[0])->getCityModel();
            if(model) exporter.exportCityModel(*model, filename.toStdString());
        }
        else
        {
            citygml::CityObject* obj = m_app.getScene().getCityObjectNode(uris[0]);
            if(obj) exporter.exportCityObject(*obj, filename.toStdString());
        }
    }
}
////////////////////////////////////////////////////////////////////////////////
void MainWindow::exportOBJsplit()
{
    QString filename = QFileDialog::getSaveFileName();
    QFileInfo fileInfo(filename);
    filename = fileInfo.path() + "/" + fileInfo.baseName();
    citygml::ExporterOBJ exporter;
    exporter.setOffset(m_app.getSettings().getDataProfile().m_offset.x, m_app.getSettings().getDataProfile().m_offset.y);
    exporter.addFilter(citygml::COT_All, "");
    exporter.addFilter(citygml::COT_WallSurface, "Wall");
    exporter.addFilter(citygml::COT_RoofSurface, "Roof");
    exporter.addFilter(citygml::COT_TINRelief, "Terrain");
    exporter.addFilter(citygml::COT_LandUse, "LandUse");
    exporter.addFilter(citygml::COT_Road, "Road");
    exporter.addFilter(citygml::COT_Door, "Door");
    exporter.addFilter(citygml::COT_Window, "Window");

    const std::vector<vcity::URI>& uris = appGui().getSelectedNodes();
    if(uris.size() > 0)
    {
        if(uris[0].getType() == "Tile")
        {
            citygml::CityModel* model = m_app.getScene().getTile(uris[0])->getCityModel();
            if(model) exporter.exportCityModel(*model, filename.toStdString());
        }
        else
        {
            citygml::CityObject* obj = m_app.getScene().getCityObjectNode(uris[0]);
            if(obj) exporter.exportCityObject(*obj, filename.toStdString());
        }
    }
}
////////////////////////////////////////////////////////////////////////////////
void MainWindow::debugDumpOsg()
{
    m_osgScene->dump();
    m_app.getScene().dump();
}
////////////////////////////////////////////////////////////////////////////////
void MainWindow::slotDumpScene()
{
    m_app.getScene().dump();
}
////////////////////////////////////////////////////////////////////////////////
void MainWindow::slotDumpSelectedNodes()
{
    vcity::log() << "Selected nodes uri : \n";
    for(std::vector<vcity::URI>::const_iterator it = appGui().getSelectedNodes().begin(); it < appGui().getSelectedNodes().end(); ++it)
    {
        vcity::log() << it->getStringURI() << "\n";
    }
}
////////////////////////////////////////////////////////////////////////////////
void MainWindow::generateAllLODs()
{

}
////////////////////////////////////////////////////////////////////////////////
void MainWindow::generateLOD0()
{
    // get all selected nodes (with a uri)
    const std::vector<vcity::URI>& uris = vcity::app().getSelectedNodes();
    if(uris.size() > 0)
    {
        // do all nodes selected
        for(std::vector<vcity::URI>::const_iterator it = uris.begin(); it < uris.end(); ++it)
        {
            vcity::app().getAlgo().generateLOD0(*it);
            // TODO
            appGui().getControllerGui().update(*it);
        }
    }
}
////////////////////////////////////////////////////////////////////////////////
void MainWindow::generateLOD1()
{
	vcity::app().getAlgo().generateLOD1(ShapeGeo, Hauteurs);
}
////////////////////////////////////////////////////////////////////////////////
void MainWindow::generateLOD2()
{
	vcity::app().getAlgo().CompareTiles();
}
////////////////////////////////////////////////////////////////////////////////
void MainWindow::generateLOD3()
{	
	vcity::app().getAlgo().generateLOD0Scene(ShapeGeo); 
}
////////////////////////////////////////////////////////////////////////////////
void MainWindow::generateLOD4()
{

}
////////////////////////////////////////////////////////////////////////////////
void MainWindow::slotFixBuilding()
{
    // get all selected nodes (with a uri)
    const std::vector<vcity::URI>& uris = vcity::app().getSelectedNodes();
    vcity::app().getAlgo2().fixBuilding(uris);

    // TODO
    //appGui().getControllerGui().update(uri);
}
////////////////////////////////////////////////////////////////////////////////
void MainWindow::slotOptimOSG()
{
    appGui().getOsgScene()->optim();
}
////////////////////////////////////////////////////////////////////////////////
void MainWindow::slotRenderLOD0()
{
    appGui().getOsgScene()->forceLOD(0);
}
////////////////////////////////////////////////////////////////////////////////
void MainWindow::slotRenderLOD1()
{
    appGui().getOsgScene()->forceLOD(1);
}
////////////////////////////////////////////////////////////////////////////////
void MainWindow::slotRenderLOD2()
{
    appGui().getOsgScene()->forceLOD(2);
}
////////////////////////////////////////////////////////////////////////////////
void MainWindow::slotRenderLOD3()
{
    appGui().getOsgScene()->forceLOD(3);
}
////////////////////////////////////////////////////////////////////////////////
void MainWindow::slotRenderLOD4()
{
    appGui().getOsgScene()->forceLOD(4);
}
////////////////////////////////////////////////////////////////////////////////
void MainWindow::slotTemporalAnim()
{
    m_temporalAnim = !m_temporalAnim;
    if(m_temporalAnim)
    {
        m_timer.start(500);
    }
    else
    {
        m_timer.stop();
    }
}
////////////////////////////////////////////////////////////////////////////////
void MainWindow::slotTemporalAnimUpdate()
{
    m_ui->horizontalSlider->setValue(m_ui->horizontalSlider->value()+365);
    //std::cout << m_ui->horizontalSlider->value() << std::endl;
}
////////////////////////////////////////////////////////////////////////////////
void MainWindow::about()
{
    // TODO : add Liris image and text
    //QMessageBox::about(this, "VCity", "VCity is an environment editor");
    DialogAbout diag;
    diag.exec();
}
////////////////////////////////////////////////////////////////////////////////
void buildJson()
{
    QString dataPath("/mnt/docs/data/dd_backup/GIS_Data/Donnees_IGN");
    std::string basePath("/tmp/json/");
    int idOffsetX = 1286;
    int idOffsetY = 13714;
    double offsetX = 643000.0;
    double offsetY = 6857000.0;
    double stepX = 500.0;
    double stepY = 500.0;

    int i = 0;
    QDirIterator iterator(dataPath, QDirIterator::Subdirectories);
    while(iterator.hasNext())
    {
        iterator.next();
        if(!iterator.fileInfo().isDir())
        {
            QString filename = iterator.filePath();
            if(filename.endsWith(".citygml", Qt::CaseInsensitive) || filename.endsWith(".gml", Qt::CaseInsensitive))
            {
                citygml::ParserParams params;
                citygml::CityModel* citygmlmodel = citygml::load(filename.toStdString(), params);
                if(citygmlmodel)
                {
                    QFileInfo fileInfo(filename);
                    std::string id = filename.toStdString();
                    id = id.substr(id.find("EXPORT_")+7);
                    id = id.substr(0, id.find_first_of("/\\"));
                    int idX = std::stoi(id.substr(0,id.find('-')));
                    int idY = std::stoi(id.substr(id.find('-')+1));
                    std::string f = "tile_" + std::to_string(idX) + '-' + std::to_string(idY);
                    std::cout << filename.toStdString() << " -> " << basePath+f << "\n";

                    std::cout << "id : " << idX << ", " << idY << std::endl;

                    citygml::ExporterJSON exporter;
                    exporter.setBasePath(basePath);
                    exporter.setOffset(offsetX+stepX*(idX-idOffsetX), offsetY+stepY*(idX-idOffsetY));
                    exporter.setTileSize(stepX, stepY);
                    exporter.exportCityModel(*citygmlmodel, f, id);
                    delete citygmlmodel;
                }
            }
        }
    }
    std::cout << std::endl;
}
////////////////////////////////////////////////////////////////////////////////
void buildJsonLod()
{
    QString dataPath("C:/Users/Game Trap/Downloads/Data/Paris/paris/tiles");
    std::string basePath("lod1/");
    int idOffsetX = 1286;
    int idOffsetY = 13714;
    double offsetX = 643000.0;
    double offsetY = 6857000.0;
    double stepX = 500.0;
    double stepY = 500.0;

    int i = 0;
    QDirIterator iterator(dataPath, QDirIterator::Subdirectories);
    while(iterator.hasNext())
    {
        iterator.next();
        if(!iterator.fileInfo().isDir())
        {
            QString filename = iterator.filePath();
            if(filename.endsWith(".shp", Qt::CaseInsensitive))
            {
                QFileInfo fileInfo(filename);
                std::string id = filename.toStdString();
                id = id.substr(id.find("tile_")+5);
                id = id.substr(0, id.find('.'));
                //std::cout << id; std::cout << " - " << id.substr(0,id.find('_')) << " / " << id.substr(id.find('_')+1) << std::endl;
                int idX = std::stoi(id.substr(0,id.find('_')));
                int idY = std::stoi(id.substr(id.find('_')+1));
                std::string f = "tile_" + std::to_string(idX) + '-' + std::to_string(idY);
                std::cout << filename.toStdString() << " -> " << basePath+f << "\n";

                std::cout << "id : " << idX << ", " << idY << std::endl;

                OGRDataSource* poDS = OGRSFDriverRegistrar::Open(filename.toStdString().c_str(), FALSE);
                buildGeosShape(poDS, &ShapeGeo, &Hauteurs);
                vcity::app().getAlgo().generateLOD1(ShapeGeo, Hauteurs);

                citygml::ExporterJSON exporter;
                exporter.setBasePath(basePath);
                exporter.setOffset(offsetX+stepX*(idX-idOffsetX), offsetY+stepY*(idX-idOffsetY));
                exporter.setTileSize(stepX, stepY);
                citygml::CityModel* model = vcity::app().getAlgo().getCitymodel();
                if(model)
                {
                    exporter.exportCityModel(*model, f, id);
                    delete model;
                }
                OGRSFDriverRegistrar::GetRegistrar()->ReleaseDataSource(poDS);
            }
        }
    }
    std::cout << std::endl;
}
////////////////////////////////////////////////////////////////////////////////
void MainWindow::test1()
{
    //loadFile("/home/maxime/docs/data/dd_gilles/IGN_Data/dpt_75/BDTOPO-75/BDTOPO/1_DONNEES_LIVRAISON_2011-12-00477/BDT_2-1_SHP_LAMB93_D075-ED113/E_BATI/BATI_INDIFFERENCIE.SHP");

	loadFile("C:/Users/Game Trap/Dropbox/Vcity/Donnees_Sathonay/SATHONAY_CAMP_BATIS_2009.gml");
	loadFile("C:/Users/Game Trap/Dropbox/Vcity/Donnees_Sathonay/SATHONAY_CAMP_BATIS_2012.gml");

	vcity::app().getAlgo().CompareTiles();


}
////////////////////////////////////////////////////////////////////////////////
void MainWindow::test2()
{
    //loadFile("/home/maxime/docs/data/dd_gilles/IGN_Data/dpt_75/BDTOPO-75/BDTOPO/1_DONNEES_LIVRAISON_2011-12-00477/BDT_2-1_SHP_LAMB93_D075-ED113/E_BATI/BATI_INDIFFERENCIE.SHP");
    /*loadFile("/home/maxime/docs/data/dd_gilles/3DPIE_Donnees_IGN_unzip/EXPORT_1296-13725/export-CityGML/ZoneAExporter.gml");
    loadFile("/home/maxime/docs/data/dd_gilles/3DPIE_Donnees_IGN_unzip/EXPORT_1296-13724/export-CityGML/ZoneAExporter.gml");
    loadFile("/home/maxime/docs/data/dd_gilles/3DPIE_Donnees_IGN_unzip/EXPORT_1295-13725/export-CityGML/ZoneAExporter.gml");
    loadFile("/home/maxime/docs/data/dd_gilles/3DPIE_Donnees_IGN_unzip/EXPORT_1295-13724/export-CityGML/ZoneAExporter.gml");

    loadFile("/home/maxime/docs/data/dd_gilles/3DPIE_Donnees_IGN_unzip/EXPORT_1296-13726/export-CityGML/ZoneAExporter.gml");
    loadFile("/home/maxime/docs/data/dd_gilles/3DPIE_Donnees_IGN_unzip/EXPORT_1295-13726/export-CityGML/ZoneAExporter.gml");
    loadFile("/home/maxime/docs/data/dd_gilles/3DPIE_Donnees_IGN_unzip/EXPORT_1294-13726/export-CityGML/ZoneAExporter.gml");
    loadFile("/home/maxime/docs/data/dd_gilles/3DPIE_Donnees_IGN_unzip/EXPORT_1294-13725/export-CityGML/ZoneAExporter.gml");*/
	loadFile("C:/Users/Game Trap/Downloads/Data/Lyon01/CADASTRE_SHP/BatiTest.shp");
	loadFile("C:/Users/Game Trap/Downloads/Data/Lyon01/Jeux de test/LYON_1ER_00136.gml");

	vcity::app().getAlgo().generateLOD0Scene(ShapeGeo);
}
////////////////////////////////////////////////////////////////////////////////
void MainWindow::test3()
{
    //loadFile("/home/maxime/docs/data/dd_gilles/3DPIE_Donnees_IGN_unzip/EXPORT_1295-13728/export-CityGML/ZoneAExporter.gml");
    //loadFile("/home/maxime/docs/data/dd_gilles/3DPIE_Donnees_IGN_unzip/EXPORT_1296-13728/export-CityGML/ZoneAExporter.gml");
    //loadFile("/home/maxime/docs/data/dd_gilles/3DPIE_Donnees_IGN_unzip/EXPORT_1296-13727/export-CityGML/ZoneAExporter.gml");
    //loadFile("/home/maxime/docs/data/dd_gilles/3DPIE_Donnees_IGN_unzip/EXPORT_1297-13727/export-CityGML/ZoneAExporter.gml");

    // test obj
}
////////////////////////////////////////////////////////////////////////////////
void MainWindow::test4()
{
    //loadFile("/home/maxime/docs/data/dd_gilles/3DPIE_Donnees_IGN_unzip/EXPORT_1305-13719/export-CityGML/ZoneAExporter.gml");
    //loadFile("/home/maxime/docs/data/dd_gilles/3DPIE_Donnees_IGN_unzip/EXPORT_1304-13719/export-CityGML/ZoneAExporter.gml");
    //loadFile("/home/maxime/docs/data/dd_gilles/3DPIE_Donnees_IGN_unzip/EXPORT_1305-13720/export-CityGML/ZoneAExporter.gml");
    //loadFile("/home/maxime/docs/data/dd_gilles/3DPIE_Donnees_IGN_unzip/EXPORT_1304-13720/export-CityGML/ZoneAExporter.gml");

    // test json
    //buildJson();
    buildJsonLod();
}
////////////////////////////////////////////////////////////////////////////////
citygml::LinearRing* cpyOffsetLinearRing(citygml::LinearRing* ring, float offset)
{
    citygml::LinearRing* ringOffset = new citygml::LinearRing(ring->getId()+"_offset", true);

    std::vector<TVec3d>& vertices = ring->getVertices();
    for(std::vector<TVec3d>::iterator itVertices = vertices.begin(); itVertices != vertices.end(); ++itVertices)
    {
        TVec3d point = *itVertices;
        point.z += offset;
        ringOffset->addVertex(point);
    }

    return ringOffset;
}
///////////////////////////////////////////////////////////////////////////////////
void test5rec(citygml::CityObject* obj)
{
    std::vector<citygml::Polygon*> polyBuf;

    // parse geometry
    std::vector<citygml::Geometry*>& geoms = obj->getGeometries();
    for(std::vector<citygml::Geometry*>::iterator itGeom = geoms.begin(); itGeom != geoms.end(); ++itGeom)
    {
        // parse polygons
        std::vector<citygml::Polygon*>& polys = (*itGeom)->getPolygons();
        for(std::vector<citygml::Polygon*>::iterator itPoly = polys.begin(); itPoly != polys.end(); ++itPoly)
        {
            // get linear ring
            citygml::LinearRing* ring = (*itPoly)->getExteriorRing();
            citygml::LinearRing* ringOffset = cpyOffsetLinearRing(ring, 100);

            citygml::Polygon* poly = new citygml::Polygon((*itPoly)->getId()); // ((*itPoly)->getId()+"_"+ringOffset->getId());
            poly->addRing(ringOffset);
            //(*itGeom)->addPolygon(poly);
            polyBuf.push_back(poly);
        }

        for(std::vector<citygml::Polygon*>::iterator it = polyBuf.begin(); it < polyBuf.end(); ++it)
        {
            (*itGeom)->addPolygon(*it);
        }
    }

    citygml::CityObjects& cityObjects = obj->getChildren();
    for(citygml::CityObjects::iterator itObj = cityObjects.begin(); itObj != cityObjects.end(); ++itObj)
    {
        test5rec(*itObj);
    }
}
////////////////////////////////////////////////////////////////////////////////
void MainWindow::test5()
{
    if(appGui().getSelectedNodes().size() > 0)
    {
        citygml::CityObject* obj = appGui().getScene().getCityObjectNode(appGui().getSelectedNodes()[0]);
        test5rec(obj);
        appGui().getControllerGui().update(appGui().getSelectedNodes()[0]);
    }
}
////////////////////////////////////////////////////////////////////////////////
