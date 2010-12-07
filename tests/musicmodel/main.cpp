#include <QtGui>
#include "musicmodel.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MusicModel *model = new MusicModel;
    model->addSearchPath("/home/girish/research/qtmediahub/hub/", "qtmediahub");

    QTreeView *listView = new QTreeView;
    listView->setModel(model);
    listView->resize(400, 400);
    listView->show();

    return app.exec();
}

