#include "skinselector.h"
#include "skin.h"

#include "frontend.h"
#include <QtGui>

SkinSelector::SkinSelector(Frontend *frontend, QWidget *parent)
    : QDialog(parent), m_frontend(frontend)
{
    QVBoxLayout *vbox = new QVBoxLayout(this);
    QListWidget *skinsView = new QListWidget(this);

    connect(skinsView, SIGNAL(itemActivated(QListWidgetItem*)),
            this, SLOT(handleSkinSelection(QListWidgetItem*)));

    foreach(Skin *skin, frontend->skins()) {
        QListWidgetItem *item = new QListWidgetItem(skin->name());
        item->setData(Qt::UserRole, qVariantFromValue<Skin *>(skin));
        skinsView->addItem(item);
    }

    vbox->addWidget(skinsView);
}

void SkinSelector::handleSkinSelection(QListWidgetItem* item) 
{
    m_frontend->setSkin(qvariant_cast<Skin *>(item->data(Qt::UserRole)));
    accept();
}

