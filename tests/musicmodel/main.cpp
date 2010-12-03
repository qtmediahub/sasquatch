#include <QtGui>
#include "musicmodel.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MusicModel *model = new MusicModel;
    model->addSearchPath("/home/girish/research/qtmediahub/hub/", "qtmediahub");
    model->start();

//    QEventLoop lo;
//    QTimer::singleShot(3000, &lo, SLOT(quit()));
//    lo.exec();
//    model->dump();
//    return 0;
//    qDebug() << "lets see now.................................";
//    qDebug() << "Top row count : " << model->rowCount();
//    QModelIndex mi = model->index(0, 0, QModelIndex());
//    QModelIndex ci = model->index(3, 0, mi);
//    qDebug() << ci.data(MusicModel::FilePathRole);
//    return 0;
    //model->addSearchPath("/home/girish/research/qtmediahub/hub/", "qtmediahub");
    //model->addSearchPath("/home/girish/research/qtmediahub/hub/", "qtmediahub");

    //MediaModel *model = new MediaModel("/home/girish/tmp/lazyloader/");
    QTreeView *listView = new QTreeView;
    listView->setModel(model);
    listView->resize(400, 400);
    listView->show();

    return app.exec();
}

