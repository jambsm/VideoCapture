#ifndef VIDEOCAPTHREAD_H
#define VIDEOCAPTHREAD_H

#include <QObject>
#include <videorec.h>
class VideoCapThread : public QObject
{
    Q_OBJECT
public:
    explicit VideoCapThread(QObject *parent = nullptr);
    VideoRec *rec;
    void onThreadExit();
public slots:
    void doinBack();

signals:

};

#endif // VIDEOCAPTHREAD_H
