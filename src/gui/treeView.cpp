////////////////////////////////////////////////////////////////////////////////
#include "moc/treeView.hpp"
#include "moc/mainWindow.hpp"
#include "moc/dialogAddLayer.hpp"
#include "moc/dialogEditLayer.hpp"
#include "moc/dialogEditTile.hpp"
#include "moc/dialogEditBldg.hpp"
#include "moc/dialogDynFlag.hpp"
#include "moc/dialogFlag.hpp"
#include "moc/dialogTag.hpp"
#include "core/application.hpp"
#include <iostream>
#include <QMenu>
#include <QLineEdit>
#include <osg/PositionAttitudeTransform>
////////////////////////////////////////////////////////////////////////////////
TreeView::TreeView(QTreeWidget* tree, MainWindow* widget)
    : m_tree(tree), m_mainWindow(widget)
{

}
////////////////////////////////////////////////////////////////////////////////
void TreeView::init()
{
    m_tree->setContextMenuPolicy(Qt::ActionsContextMenu);

    // right click menu actions
    m_actionAddTile = new QAction("Add tile", NULL);
    m_actionEditTile = new QAction("Edit tile", NULL);
    m_actionDeleteTile = new QAction("Delete tile", NULL);
    m_actionAddLayer = new QAction("Add layer", NULL);
    m_actionEditLayer = new QAction("Edit layer", NULL);
    m_actionDeleteLayer = new QAction("Delete layer", NULL);
    m_actionAddBuilding = new QAction("Add building", NULL);
    m_actionEditBuilding = new QAction("Edit building", NULL);
    m_actionDeleteBuilding = new QAction("Delete building", NULL);
    m_actionAddFlag = new QAction("Add Flag", NULL);
    m_actionAddDynFlag = new QAction("Add dynamic Flag", NULL);
    m_actionAddTag = new QAction("Add Tag", NULL);
    m_actionEditFlag = new QAction("Edit Flag", NULL);
    m_actionEditDynFlag = new QAction("Edit dynamic Flag", NULL);
    m_actionEditTag = new QAction("Edit Tag", NULL);
    m_actionDeleteFlag = new QAction("Delete Flag", NULL);
    m_actionDeleteDynFlag = new QAction("Delete dynamic Flag", NULL);
    m_actionDeleteTag = new QAction("Delete Tag", NULL);

    // connect right click menu actions
    connect(m_actionAddTile, SIGNAL(triggered()), this, SLOT(slotAddTile()));
    connect(m_actionEditTile, SIGNAL(triggered()), this, SLOT(slotEditTile()));
    connect(m_actionDeleteTile, SIGNAL(triggered()), this, SLOT(slotDeleteTile()));
    connect(m_actionAddLayer, SIGNAL(triggered()), this, SLOT(slotAddLayer()));
    connect(m_actionEditLayer, SIGNAL(triggered()), this, SLOT(slotEditLayer()));
    connect(m_actionDeleteLayer, SIGNAL(triggered()), this, SLOT(slotDeleteLayer()));
    connect(m_actionAddBuilding, SIGNAL(triggered()), this, SLOT(slotAddBuilding()));
    connect(m_actionEditBuilding, SIGNAL(triggered()), this, SLOT(slotEditBuilding()));
    connect(m_actionDeleteBuilding, SIGNAL(triggered()), this, SLOT(slotDeleteBuilding()));
    connect(m_actionAddFlag, SIGNAL(triggered()), this, SLOT(slotAddFlag()));
    connect(m_actionAddDynFlag, SIGNAL(triggered()), this, SLOT(slotAddDynFlag()));
    connect(m_actionAddTag, SIGNAL(triggered()), this, SLOT(slotAddTag()));
    connect(m_actionEditFlag, SIGNAL(triggered()), this, SLOT(slotEditFlag()));
    connect(m_actionEditDynFlag, SIGNAL(triggered()), this, SLOT(slotEditDynFlag()));
    connect(m_actionEditTag, SIGNAL(triggered()), this, SLOT(slotEditTag()));
    connect(m_actionDeleteFlag, SIGNAL(triggered()), this, SLOT(slotDeleteFlag()));
    connect(m_actionDeleteDynFlag, SIGNAL(triggered()), this, SLOT(slotDeleteDynFlag()));
    connect(m_actionDeleteTag, SIGNAL(triggered()), this, SLOT(slotDeleteTag()));

    /*connect(m_actionEditLayer, SIGNAL(triggered()), this, SLOT(slotEditLayer()));
    connect(m_actionEditBuilding, SIGNAL(triggered()), this, SLOT(slotEditBldg()));

    connect(m_actionAddFlag, SIGNAL(triggered()), m_mainWindow, SLOT(optionAddFlag()));
    connect(m_actionAddDynFlag, SIGNAL(triggered()), m_mainWindow, SLOT(optionAddDynFlag()));
    connect(m_actionAddTag, SIGNAL(triggered()), m_mainWindow, SLOT(optionAddTag()));*/


    //connect(select, SIGNAL(triggered()), this, SLOT(selectNodeHandler()));
    //connect(insertChild, SIGNAL(triggered()), this, SLOT(insertChildHandler()));
    //connect(deleteNode, SIGNAL(triggered()), this, SLOT(deleteNodeHandler()));
    //connect(showInfo, SIGNAL(triggered()), this, SLOT(showInfoHandler()));


    // connect other signals
    // checkbox signal
    connect(m_tree, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(slotItemChanged(QTreeWidgetItem*, int)));

    // right click handling : to activate actions depending on node type
    connect(m_tree, SIGNAL(itemPressed(QTreeWidgetItem*, int)), this, SLOT(slotSelectNode(QTreeWidgetItem*,int)));

    // show info
    //connect(m_tree, SIGNAL(itemClicked(QTreeWidgetItem*,int)), m_mainWindow, SLOT(showInfoHandler()));
    connect(m_tree, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(slotItemClicked(QTreeWidgetItem*,int)));

    //
    //connect(m_tree, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(slotItemActivated(QTreeWidgetItem*,int)));



    reset();
}
////////////////////////////////////////////////////////////////////////////////
void TreeView::reset()
{
    m_tree->clear();
    m_tree->reset();

    QTreeWidgetItem* root = addItemRoot();
    m_tree->addTopLevelItem(root);

    QTreeWidgetItem* layer = addItemLayer("layer_CityGML");
    root->addChild(layer);
}
////////////////////////////////////////////////////////////////////////////////
QTreeWidgetItem* TreeView::addItemGeneric(const QString& name, const QString& type) // TODO : rename createItemGeneric
{
    QTreeWidgetItem* item = new QTreeWidgetItem(QStringList(name));
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(0, Qt::Checked);
    item->setText(1, type);

    return item;
}
////////////////////////////////////////////////////////////////////////////////
QTreeWidgetItem* TreeView::addItemGeneric(const vcity::URI& uri, const QString& name, const QString& type)
{
    QTreeWidgetItem* item = getNode(uri);
    if(item)
    {
        item->addChild(addItemGeneric(name, type));
    }

    return item;
}
////////////////////////////////////////////////////////////////////////////////
QTreeWidgetItem* TreeView::addItemRoot()
{
    QTreeWidgetItem* item = addItemGeneric("root", "Root");
    return item;
}
////////////////////////////////////////////////////////////////////////////////
QTreeWidgetItem* TreeView::addItemLayer(const QString& name)
{
    QTreeWidgetItem* item = addItemGeneric(name, "Layer");
    return item;
}
////////////////////////////////////////////////////////////////////////////////
void TreeView::deleteItem(const vcity::URI& uri)
{
    QTreeWidgetItem* tile = getNode(uri);
    if(tile)
    {
        QTreeWidgetItem* parent = tile->parent();
        parent->removeChild(tile);
    }
}
////////////////////////////////////////////////////////////////////////////////
vcity::URI TreeView::getURI(QTreeWidgetItem* item) const
{
    vcity::URI uri;
    uri.setType(item->text(1).toStdString());
    QTreeWidgetItem* current = item;

    while(current && current->text(1) != "Root")
    {
        uri.append(current->text(0).toStdString());
        current = current->parent();
    }

    return uri;
}
////////////////////////////////////////////////////////////////////////////////
QTreeWidget* TreeView::getTree()
{
    return m_tree;
}
////////////////////////////////////////////////////////////////////////////////
QTreeWidgetItem* TreeView::getCurrentItem()
{
    return m_tree->currentItem();
}
////////////////////////////////////////////////////////////////////////////////
void TreeView::addLayer(const vcity::URI& uri)
{
    QTreeWidgetItem* layer = addItemLayer(uri.getNode(0).c_str());
    m_tree->topLevelItem(0)->addChild(layer);
}
////////////////////////////////////////////////////////////////////////////////
void TreeView::setLayerName(const vcity::URI& uri, const std::string& name)
{
    QTreeWidgetItem* item = getNode(uri);
    if(item)
    {
        item->setText(0, name.c_str());
    }
}
////////////////////////////////////////////////////////////////////////////////
void TreeView::deleteLayer(const vcity::URI& uri)
{
    QTreeWidgetItem* tile = getNode(uri);
    if(tile)
    {
        QTreeWidgetItem* parent = tile->parent();
        parent->removeChild(tile);
    }
}
////////////////////////////////////////////////////////////////////////////////
void addTileRec(QTreeWidgetItem* parent, citygml::CityObject* node)
{
    QTreeWidgetItem* item = new QTreeWidgetItem(parent, QStringList(node->getId().c_str()));
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(0, Qt::Checked);
    item->setText(1, node->getTypeAsString().c_str());

    citygml::CityObjects& cityObjects = node->getChildren();
    //std::cout << "child size : " << cityObject.size() << std::endl;
    citygml::CityObjects::iterator it = cityObjects.begin();
    for( ; it != cityObjects.end(); ++it)
    {
        addTileRec(item, *it);
    }
}
////////////////////////////////////////////////////////////////////////////////
void TreeView::addTile(const vcity::URI& uriLayer, vcity::Tile& tile)
{
    m_tree->blockSignals(true);

    QTreeWidgetItem* root = m_tree->topLevelItem(0);
    //QTreeWidgetItem* root = 0;
    QTreeWidgetItem* layer = getNode(uriLayer);

    QTreeWidgetItem* item = new QTreeWidgetItem(layer, QStringList(tile.getName().c_str()));
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(0, Qt::Checked);
    item->setText(1, "Tile");

    citygml::CityModel* citymodel = tile.getCityModel();
    citygml::CityObjects& cityObjects = citymodel->getCityObjectsRoots();
    //std::cout << "root size : " << cityObjects.size() << std::endl;
    citygml::CityObjects::iterator it = cityObjects.begin();
    for( ; it != cityObjects.end(); ++it)
    {
        addTileRec(item, *it);
    }

    m_tree->expandToDepth(0);

    m_tree->blockSignals(false);
}
////////////////////////////////////////////////////////////////////////////////
void TreeView::setTileName(const vcity::URI& uri, std::string& name)
{
    QTreeWidgetItem* item = getNode(uri);
    if(item)
    {
        item->setText(0, name.c_str());
    }
}
////////////////////////////////////////////////////////////////////////////////
void TreeView::deleteTile(const vcity::URI& uri)
{
    QTreeWidgetItem* tile = getNode(uri);
    if(tile)
    {
        QTreeWidgetItem* parent = tile->parent();
        parent->removeChild(tile);
    }
}
////////////////////////////////////////////////////////////////////////////////
void TreeView::selectItem(const vcity::URI& uri)
{
    QTreeWidgetItem* item = getNode(uri);
    if(item)
    {
        item->setSelected(true);
    }
}
////////////////////////////////////////////////////////////////////////////////
void resetSelectionRec(QTreeWidgetItem* item)
{
    item->setSelected(false);

    int count = item->childCount();
    for(int i=0; i<count; ++i)
    {
        QTreeWidgetItem* current = item->child(i);
        resetSelectionRec(current);
    }
}
////////////////////////////////////////////////////////////////////////////////
void TreeView::resetSelection()
{
    QTreeWidgetItem* root = m_tree->topLevelItem(0);
    resetSelectionRec(root);
}
////////////////////////////////////////////////////////////////////////////////
QTreeWidgetItem* TreeView::getNode(const vcity::URI& uri)
{
    QTreeWidgetItem* root = m_tree->topLevelItem(0);
    QTreeWidgetItem* current = root;

    int depth = uri.getDepth();
    int maxDepth = depth;

    if(depth == 0)
    {
        return nullptr;
    }

    do
    {
        int count = current->childCount();
        for(int i=0; i<count; ++i)
        {
            QTreeWidgetItem* item = current->child(i);
            //vcity::log() << item->text(0).toStdString() << " -> " << uri.getNode(maxDepth-depth) << "\n";
            if(item->text(0).toStdString() == uri.getNode(maxDepth-depth))
            {
                current = item;
                if(depth == 1) return item;
                break;
            }
        }
        --depth;
    } while(depth > 0);

    return nullptr;
}
////////////////////////////////////////////////////////////////////////////////
void TreeView::slotSelectNode(QTreeWidgetItem* item, int /*column*/)
{
    //std::cout << "select node : " << item->text(0).toStdString() << "," << item->text(1).toStdString() << std::endl;

    // insane brute force hack :
    // every right click, remove all actions on the QTreeWidget
    // and depending on the type of the node clicked, activate needed actions

    resetActions();

    if(item->text(1) == "Root")
    {
        //std::cout << "Root" << std::endl;
        m_tree->addAction(m_actionAddLayer);
    }
    else if(item->text(1) == "Layer")
    {
        //std::cout << "Layer" << std::endl;
        m_tree->addAction(m_actionDeleteLayer);
        m_tree->addAction(m_actionEditLayer);
        m_tree->addAction(m_actionAddTile);
    }
    else if(item->text(1) == "Tile")
    {
        //std::cout << "Layer" << std::endl;
        m_tree->addAction(m_actionDeleteTile);
        m_tree->addAction(m_actionEditTile);
        m_tree->addAction(m_actionAddBuilding);
    }
    else if(item->text(1) == "Building")
    {
        //std::cout << "Building" << std::endl;
        m_tree->addAction(m_actionDeleteBuilding);
        m_tree->addAction(m_actionEditBuilding);
        m_tree->addAction(m_actionAddTag);
        m_tree->addAction(m_actionAddFlag);
        m_tree->addAction(m_actionAddDynFlag);
    }
    else if(item->text(1) == "Tag")
    {
        //std::cout << "Tag" << std::endl;
        m_tree->addAction(m_actionDeleteTag);
        m_tree->addAction(m_actionEditTag);
    }
    else if(item->text(1) == "Flag")
    {
        //std::cout << "Flag" << std::endl;
        m_tree->addAction(m_actionDeleteFlag);
        m_tree->addAction(m_actionEditFlag);
    }
    else if(item->text(1) == "DynFlag")
    {
        //std::cout << "Flag" << std::endl;
        m_tree->addAction(m_actionDeleteDynFlag);
        m_tree->addAction(m_actionEditDynFlag);
    }
}
////////////////////////////////////////////////////////////////////////////////
void TreeView::slotItemChanged(QTreeWidgetItem* item, int column)
{
    Qt::CheckState state = item->checkState(0);
    bool show = true;
    if(state == Qt::CheckState::Unchecked) show = false;
    appGui().getOsgScene()->showNode(getURI(item), show);
}
////////////////////////////////////////////////////////////////////////////////
void TreeView::slotItemClicked(QTreeWidgetItem* item, int)
{
    vcity::URI uri = getURI(item);

    if(uri.getDepth() > 0)
    {
        m_mainWindow->updateTextBox(uri);

        appGui().getControllerGui().resetSelection();
        appGui().getControllerGui().addSelection(uri);
    }
}
////////////////////////////////////////////////////////////////////////////////
void searchNode(TreeView* tv, QTreeWidgetItem* node, const QString& filter)
{
    if(node)
    {
        int count = node->childCount();
        for(int i=0; i<count; ++i)
        {
            QTreeWidgetItem* item = node->child(i);
            if(item->text(0).contains(filter, Qt::CaseSensitivity::CaseInsensitive))
            {
                item->setSelected(true);

                // select node
                appGui().getControllerGui().addSelection(tv->getURI(item));
                //vcity::URI uri = tv->getURI(item);
                //appGui().getMainWindow()->m_pickhandler->toggleSelected(uri);
            }
            searchNode(tv, item, filter);
        }
    }
}
////////////////////////////////////////////////////////////////////////////////
void TreeView::slotFilter()
{
    //std::cout << "filter : " << appGui().getMainWindow()->getFilter()->text().toStdString() << std::endl;

    appGui().getControllerGui().resetSelection();
    searchNode(this, m_tree->topLevelItem(0), appGui().getMainWindow()->getFilter()->text());
}
////////////////////////////////////////////////////////////////////////////////
void TreeView::slotAddTile()
{

}
////////////////////////////////////////////////////////////////////////////////
void TreeView::slotEditTile()
{
    DialogEditTile diag;
    diag.editTile(getURI(getCurrentItem()));
}
////////////////////////////////////////////////////////////////////////////////
void TreeView::slotDeleteTile()
{
    appGui().getControllerGui().deleteTile(getURI(getCurrentItem()));
}
////////////////////////////////////////////////////////////////////////////////
void TreeView::slotAddLayer()
{
    DialogAddLayer diag;
    diag.addLayer();
}
////////////////////////////////////////////////////////////////////////////////
void TreeView::slotEditLayer()
{
    DialogEditLayer diag;
    diag.editLayer(getURI(getCurrentItem()));
}
////////////////////////////////////////////////////////////////////////////////
void TreeView::slotDeleteLayer()
{
    appGui().getControllerGui().deleteLayer(getURI(getCurrentItem()));
}
////////////////////////////////////////////////////////////////////////////////
void TreeView::slotAddBuilding()
{

}
////////////////////////////////////////////////////////////////////////////////
void TreeView::slotEditBuilding()
{
    DialogEditBldg diag;
    diag.setName(m_tree->currentItem()->text(0));
    //diag.setEnvelope(0, 1, 0, 1);

    vcity::URI uri = getURI(getCurrentItem());

    osg::ref_ptr<osg::Node> node = m_mainWindow->m_osgScene->getNode(uri);

    if(node && node->asGeode())
    {
        //std::cout << "geode" << std::endl;
        node = node->getParent(0);
    }

    if(node)
        if(node->asTransform())
            if(node->asTransform()->asPositionAttitudeTransform())
            {
                osg::ref_ptr<osg::PositionAttitudeTransform> pos = node->asTransform()->asPositionAttitudeTransform();
                osg::Vec3d v = pos->getPosition();
                diag.setOffset(v.x(),v.y());
                //pos->setPosition(osg::Vec3d(x, y, 0));
                //std::cout << "pos : " << pos << std::endl;
            }


    //diag.setOffset(2, 2);

    if(diag.exec())
    {
        //diag.setName(m_tree->currentItem()->text(0));

        osg::ref_ptr<osg::Node> node = m_mainWindow->m_osgScene->getNode(uri);

        if(node && node->asGeode())
        {
            //std::cout << "geode" << std::endl;
            node = node->getParent(0);
        }

        if(node)
            if(node->asTransform())
                if(node->asTransform()->asPositionAttitudeTransform())
                {
                    osg::ref_ptr<osg::PositionAttitudeTransform> pos = node->asTransform()->asPositionAttitudeTransform();
                    double x,y;
                    diag.getOffset(x,y);
                    pos->setPosition(osg::Vec3d(x, y, 0));
                    //std::cout << "pos : " << pos << std::endl;
                }


        //std::cout << node << std::endl;
    }
}
////////////////////////////////////////////////////////////////////////////////
void TreeView::slotDeleteBuilding()
{

}
////////////////////////////////////////////////////////////////////////////////
void TreeView::slotAddFlag()
{
    DialogFlag diag;
    diag.addFlag(getURI(m_tree->currentItem()));
}
////////////////////////////////////////////////////////////////////////////////
void TreeView::slotAddDynFlag()
{
    DialogDynFlag diag;
    diag.addDynFlag(getURI(m_tree->currentItem()));
}
////////////////////////////////////////////////////////////////////////////////
void TreeView::slotAddTag()
{
    DialogTag diag;
    diag.addTag(getURI(m_tree->currentItem()));
}
////////////////////////////////////////////////////////////////////////////////
void TreeView::slotEditFlag()
{

}
////////////////////////////////////////////////////////////////////////////////
void TreeView::slotEditDynFlag()
{

}
////////////////////////////////////////////////////////////////////////////////
void TreeView::slotEditTag()
{

}
////////////////////////////////////////////////////////////////////////////////
void TreeView::slotDeleteFlag()
{

}
////////////////////////////////////////////////////////////////////////////////
void TreeView::slotDeleteDynFlag()
{

}
////////////////////////////////////////////////////////////////////////////////
void TreeView::slotDeleteTag()
{

}
////////////////////////////////////////////////////////////////////////////////
 void TreeView::resetActions()
 {
     m_tree->removeAction(m_actionAddTile);
     m_tree->removeAction(m_actionEditTile);
     m_tree->removeAction(m_actionDeleteTile);
     m_tree->removeAction(m_actionAddLayer);
     m_tree->removeAction(m_actionEditLayer);
     m_tree->removeAction(m_actionDeleteLayer);
     m_tree->removeAction(m_actionAddBuilding);
     m_tree->removeAction(m_actionEditBuilding);
     m_tree->removeAction(m_actionDeleteBuilding);
     m_tree->removeAction(m_actionAddFlag);
     m_tree->removeAction(m_actionAddDynFlag);
     m_tree->removeAction(m_actionAddTag);
     m_tree->removeAction(m_actionEditFlag);
     m_tree->removeAction(m_actionEditTag);
     m_tree->removeAction(m_actionDeleteFlag);
     m_tree->removeAction(m_actionDeleteTag);

 }
////////////////////////////////////////////////////////////////////////////////
