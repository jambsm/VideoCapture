#include "audiocapthread.h"

audiocapthread::audiocapthread(QObject *parent) : QObject(parent)
{
 rec =new audiorec();
}

void audiocapthread::onThreadExit(){
 rec->SetFlag(true);
}
void audiocapthread::doinBack(){

    rec->StartcaptureAudio();

}
