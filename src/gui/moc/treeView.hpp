// Copyright University of Lyon, 2012 - 2017
// Distributed under the GNU Lesser General Public License Version 2.1 (LGPLv2)
// (Refer to accompanying file LICENSE.md or copy at
//  https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html )

////////////////////////////////////////////////////////////////////////////////
#ifndef __TREEVIEW_HPP__
#define __TREEVIEW_HPP__
////////////////////////////////////////////////////////////////////////////////
#include <QTreeWidget>
#include "libcitygml/URI.hpp"
#include "core/application.hpp"

////////////////////////////////////////////////////////////////////////////////
class MainWindow;
class DialogShpTool;
////////////////////////////////////////////////////////////////////////////////
/// \brief The TreeView class, handles everything relative to the Qt treeview
///
class TreeView : public QObject
{
    Q_OBJECT

public:
    TreeView(QTreeWidget* tree, MainWindow* widget);
    virtual ~TreeView() override;

    /// \brief init Init the treeview
    void init();

    /// \brief reset Reset the treeview (root item and one layer)
    void reset();

    QTreeWidgetItem* addItemGeneric(const vcity::URI& uri, const QString& name, const QString& type);

    QTreeWidgetItem* createItemGeneric(const QString& name, const QString& type, const bool checkable = true);
    QTreeWidgetItem* createItemRoot();
    QTreeWidgetItem* createItemLayer(const QString& name, const QString& type);

    void deleteItem(const std::string& URI);
    void deleteItem(const vcity::URI& URI);

    /// \brief getURI Compute URI of a treeview item
    /// \param item TreeView item
    /// \return URI
    vcity::URI getURI(QTreeWidgetItem* item) const;

    QTreeWidget* getTree();
    QTreeWidgetItem* getCurrentItem();

    void addLayer(const vcity::URI& uri);
    void setLayerName(const vcity::URI& uri, const std::string& name);
    void deleteLayer(const vcity::URI& uri);

    void addTile(const vcity::URI& uriLayer, vcity::Tile& tile);
    void setTileName(const vcity::URI& uri, const std::string& name);
    void deleteTile(const vcity::URI& uri);

    void addInfo(const vcity::URI& uriLayer, std::vector<osgInfo*> v_info);


	void addAssimpNodeRecursively(QTreeWidgetItem* parent, const osg::ref_ptr<osg::Node> node, std::string strLevel);
	void addAssimpNode(const vcity::URI& uriLayer, const osg::ref_ptr<osg::Node> node);
	void setAssimpNodeName(const vcity::URI& uri, const std::string& name);
    void deleteAssimpNode(const vcity::URI& uri);

	void addMntAscNode(const vcity::URI& uriLayer, const osg::ref_ptr<osg::Node> node);
	void addLasNode(const vcity::URI& uriLayer, const osg::ref_ptr<osg::Node> node);
    void addShpNode(const vcity::URI& uriLayer, const std::string& nodeName);

	QTreeWidgetItem* addVersion(QTreeWidgetItem* parent, const std::string& name);
	QTreeWidgetItem* addWorkspace(QTreeWidgetItem* parent, const std::string& name);
    QTreeWidgetItem* addDocument(QTreeWidgetItem* parent, const std::string& name);
    QTreeWidgetItem* addReference(QTreeWidgetItem* parent, const std::string& name);

    void addCityObject(QTreeWidgetItem* parent, citygml::CityObject* node);

    /// \brief selectItem Select item in treeview
    /// \param uri URI pointing to selected node
    void selectItem(const vcity::URI& uri);

    /// \brief resetSelection Deselect all items
    void resetSelection();

    /// \brief getNode Find an item using a URI
    /// \param uri URI pointing to the wanted item/node
    /// \return QTreeWidgetItem corresponding to uri
    QTreeWidgetItem* getNode(const vcity::URI& uri);

private slots:
    void slotAddTile();
    void slotEditTile();
    void slotDeleteTile();
    void slotEditAssimpNode();
    void slotDeleteAssimpNode();
    void slotAddLayer();
    void slotEditLayer();
    void slotDeleteLayer();
    void slotAddBuilding();
    void slotEditBuilding();
    void slotDeleteBuilding();
    void slotCheckAll();
    void slotUnCheckAll();
    void slotAddDoc();
	void slotExportJSON();
	void slotEditShp();
	//ajout yearOfConstruction/yearOfDemolition
	void slotAddYearOfConst();
	void slotAddYearOfDemol();
	void slotAddLink();

    void slotSelectNode(QTreeWidgetItem* item, int column);
    void slotItemChanged(QTreeWidgetItem* item, int column);
    void slotItemClicked(QTreeWidgetItem* item,int column);
    void slotItemDoubleClicked(QTreeWidgetItem* item,int column);
    //void slotItemActivated(QTreeWidgetItem* item, int column);

    void slotFilter();

private:
    void resetActions();

    QTreeWidget* m_tree;
    MainWindow* m_mainWindow;
	DialogShpTool* dialogShpTool;

    // all registered actions
    QAction* m_actionAddTile;
    QAction* m_actionEditTile;
    QAction* m_actionDeleteTile;
    QAction* m_actionEditAssimpNode;
    QAction* m_actionDeleteAssimpNode;
    QAction* m_actionAddLayer;
    QAction* m_actionEditLayer;
    QAction* m_actionDeleteLayer;
    QAction* m_actionAddBuilding;
    QAction* m_actionEditBuilding;
    QAction* m_actionDeleteBuilding;
    QAction* m_actionSelectAll;
    QAction* m_actionDeSelectAll;
    QAction* m_actionAddDoc;
	QAction* m_actionExportJSON;
	QAction* m_actionEditShp;
	//ajout yearOfConstruction/yearOfDemolition
	QAction* m_actionAddYearOfConst;
	QAction* m_actionAddYearOfDemol;
	QAction* m_actionAddLink;
};
////////////////////////////////////////////////////////////////////////////////
#endif // __TREEVIEW_HPP__
