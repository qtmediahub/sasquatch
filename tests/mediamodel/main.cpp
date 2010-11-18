#include <QtGui>
#include "mediamodel.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MediaModel *model = new MediaModel(QDir::currentPath());
    //MediaModel *model = new MediaModel("/home/girish/tmp/lazyloader/");
    QListView *listView = new QListView;
    listView->setModel(model);
    listView->resize(400, 400);
    listView->show();

    return app.exec();
}

