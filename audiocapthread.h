#ifndef AUDIOCAPTHREAD_H
#define AUDIOCAPTHREAD_H

#include <QObject>
#include "audiorec.h"
class audiocapthread : public QObject
{
    Q_OBJECT
public:
    audiorec *rec;
    explicit audiocapthread(QObject *parent = nullptr);
    void onThreadExit();
public slots:
    void doinBack();
signals:

};

#endif // AUDIOCAPTHREAD_H
