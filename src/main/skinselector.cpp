#include "skinselector.h"
#include "skin.h"

#include "frontend.h"
#include <QtGui>

SkinSelector::SkinSelector(const QHash<QString, Skin *> &skins, QWidget *parent)
    : QDialog(parent)
{
    QVBoxLayout *vbox = new QVBoxLayout(this);
    QListWidget *skinsView = new QListWidget(this);

    connect(skinsView, SIGNAL(itemActivated(QListWidgetItem*)),
            this, SLOT(handleSkinSelection(QListWidgetItem*)));

    foreach(Skin *skin, skins) {
        QListWidgetItem *item = new QListWidgetItem(skin->name());
        item->setData(Qt::UserRole, qVariantFromValue<Skin *>(skin));
        skinsView->addItem(item);
    }

    vbox->addWidget(skinsView);
}

void SkinSelector::handleSkinSelection(QListWidgetItem* item) 
{
    emit skinSelected(qvariant_cast<Skin *>(item->data(Qt::UserRole)));
    accept();
}

