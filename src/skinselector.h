#ifndef SKINSELECTOR_H
#define SKINSELECTOR_H

#include <QDialog>

class Frontend;
class QListWidgetItem;

class SkinSelector : public QDialog
{
    Q_OBJECT
public:
    SkinSelector(Frontend *frontend, QWidget *parent = 0);

private slots:
    void handleSkinSelection(QListWidgetItem* item);

private:
    Frontend *m_frontend;
};

#endif // SKINSELECTOR_H

