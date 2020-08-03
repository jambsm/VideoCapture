#include "videocapthread.h"
#include <qdebug.h>
VideoCapThread::VideoCapThread(QObject *parent) : QObject(parent)
{
 rec=new VideoRec();
}

void VideoCapThread::doinBack(){


    rec->StartcaptureVideo();
}

void VideoCapThread::onThreadExit(){
   rec->SetFlag(true);
}
