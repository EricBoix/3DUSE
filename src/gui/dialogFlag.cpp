#include "moc/dialogFlag.hpp"
#include "ui_dialogFlag.h"
#include "gui/applicationGui.hpp"
#include <QSettings>
#include <QFileDialog>
#include "moc/mainWindow.hpp"
////////////////////////////////////////////////////////////////////////////////
DialogFlag::DialogFlag(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogFlag)
{
    ui->setupUi(this);
    connect(ui->toolButton,SIGNAL(clicked()), this, SLOT(addFeatureBox()));
}
////////////////////////////////////////////////////////////////////////////////
DialogFlag::~DialogFlag()
{
    delete ui;
}
////////////////////////////////////////////////////////////////////////////////
void DialogFlag::addFlag(const vcity::URI& uri)
{
    appGui().getMainWindow()->m_osgView->setActive(false);

    citygml::CityObject* obj = nullptr;

    //if(m_ui->treeWidget->currentItem())
    {
        //std::cout << "select node : " << m_ui->treeWidget->currentItem()->text(0).toStdString() << std::endl;
        obj = vcity::app().getScene().getCityObjectNode(uri);

        if(obj)
        {
            // add lod
            ui->comboBox->addItem(uri.getLastNode().c_str());
        }
        ui->comboBox->addItem("NULL");
        ui->comboBox->addItem("NEW");
    }

    int res = exec();

    //std::cout << "diag res : " << res << std::endl;

    if(res && obj) // && m_ui->treeWidget->currentItem())
    {
        citygml::CityObject* geom = nullptr;
        //std::cout << ui.comboBox->currentText().toStdString() << std::endl;

        QString item2text;

        if(ui->comboBox->currentText() == "NEW")
        {
            // load
            std::cout << "load new" << std::endl;

            QSettings settings("liris", "virtualcity");
            QString lastdir = settings.value("lastdir").toString();
            QString filename = QFileDialog::getOpenFileName(0, "Load scene file", lastdir);
            if(!filename.isEmpty())
            {
                citygml::ParserParams params;
                citygml::CityModel* mdl = citygml::load(filename.toStdString(), params);
                citygml::CityObject* bldg = mdl->getCityObjectsRoots()[0];
                geom = bldg;
                geom->m_path = filename.toStdString();
                std::cout << "nb : " << mdl->getCityObjectsRoots().size()<< std::endl;

                // create osg geometry
                /*size_t pos = filename.toStdString().find_last_of("/\\");
                std::string path = filename.toStdString().substr(0, pos);
                ReaderOsgCityGML readerOsgGml(path);

                osg::ref_ptr<osg::Group> grp = readerOsgGml.createCityObject(bldg);

                if(bldg->getType() == citygml::COT_Building)
                {
                    int yearOfConstruction;
                    int yearOfDemolition;

                    std::istringstream(bldg->getAttribute("yearOfConstruction")) >> yearOfConstruction;
                    std::istringstream(bldg->getAttribute("yearOfDemolition")) >> yearOfDemolition;

                    grp->setUserValue("yearOfConstruction", yearOfConstruction);
                    grp->setUserValue("yearOfDemolition", yearOfDemolition);
                }

                bldg->setOsgNode(grp);*/
                item2text = bldg->getId().c_str();
            }
        }
        else if(ui->comboBox->currentText() == "NULL")
        {
            geom = NULL;
            item2text = "NULL";
        }
        else
        {
            // use existing
            geom = vcity::app().getScene().getCityObjectNode(uri); //findNode(ui.comboBox->currentText().toStdString());
            std::cout << "use existing : " << geom << std::endl;
            item2text = ui->comboBox->currentText();
        }

        citygml::BuildingFlag* flag = new citygml::BuildingFlag(geom);
        flag->m_name = ui->lineEdit->text().toStdString();
        flag->m_parent = obj;
        obj->addFlag(flag);

        QTreeWidgetItem* item = new QTreeWidgetItem(QStringList(flag->getStringId().c_str()));
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(0, Qt::Checked);
        item->setText(1, "Flag");

        QTreeWidgetItem* item2 = new QTreeWidgetItem(QStringList(item2text));
        item->addChild(item2);

        //appGui().m_ui treeWidget->currentItem()->addChild(item);
        appGui().getTreeView()->addItemGeneric(uri, flag->getStringId().c_str(), "Flag");
    }
    appGui().getMainWindow()->m_osgView->setActive(true);
}
////////////////////////////////////////////////////////////////////////////////
void DialogFlag::addFeatureBox()
{
    QHBoxLayout* hb = new QHBoxLayout();
    QLineEdit* key = new QLineEdit();
    QLineEdit* val = new QLineEdit();
    key->setToolTip("Feature name");
    val->setToolTip("Value");
    hb->addWidget(key);
    hb->addWidget(val);
    ui->verticalLayout_2->addLayout(hb);
}
////////////////////////////////////////////////////////////////////////////////
