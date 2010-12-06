#ifndef MUSICMODEL_H
#define MUSICMODEL_H

#include "../mediamodel.h"

class MusicModel : public MediaModel
{
    Q_OBJECT
public:
    MusicModel(QObject *parent = 0);
    ~MusicModel();
};

#endif // MUSICMODEL_H

