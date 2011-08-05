#ifndef SKINSELECTOR_H
#define SKINSELECTOR_H

#include <QDialog>

class QListWidgetItem;
class Skin;

class SkinSelector : public QDialog
{
    Q_OBJECT
public:
    SkinSelector(const QHash<QString, Skin *> &skins, QWidget *parent = 0);

signals:
    void skinSelected(Skin *skin);

private slots:
    void handleSkinSelection(QListWidgetItem* item);
};

#endif // SKINSELECTOR_H

