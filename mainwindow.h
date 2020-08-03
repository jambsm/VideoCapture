#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include<QMenu>
#include<QMenuBar>
#include<QToolBar>
#include <qthread.h>
#include "videocapthread.h"
#include <QAudioRecorder>
#include<QtMultimedia/QAudioProbe>
#include<QtMultimedia/QCamera>
#include<QList>
#include <QCameraInfo>
#include"audiocapthread.h"
extern "C"
{
    #include "include/libavcodec/avcodec.h"
    #include "include/libavformat/avformat.h"
    #include "include/libswscale/swscale.h"
    #include "include/libavdevice/avdevice.h"
}

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void setupMenu();
   void setupWidget();
   void initvar();
   QAudioRecorder *recorder;
   QAudioProbe *probe;
   QCamera *curCamera=Q_NULLPTR;
      int H264ToMp4();

public slots:
    int Startcapture();
    int Stopcapture();

private:
    Ui::MainWindow *ui;
    QMenu *FileMenu,*EditMenu,*AboutMenu;
    QAction *action;
    QMenuBar *menubar;
    QToolBar *qtoolbar;
 QThread *videosubThread,*audiosubthread;
  VideoCapThread *my_videothread;
  audiocapthread *my_audiothread;
 signals:
 void startThread();
   // int  YUVToMp4();

    //int flush_encoder(AVFormatContext *fmt_ctx,unsigned int stream_index);
};
#endif // MAINWINDOW_H
