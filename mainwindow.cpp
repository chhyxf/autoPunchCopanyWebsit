#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QWebView>
#include <QLineEdit>
#include <QTimer>
#include <sys/time.h>
#include <windows.h>

#include <QMdiSubWindow>
#include <QMessageBox>
#include <QMouseEvent>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    companyWebsite = "https://www.revival.net.cn/";
    progress = 0;
    view = new QWebView(this);
    TimerReTry = new QTimer(this);
    setCentralWidget(view);
    resize(800, 600);

    // 关联信号和槽    
    connect(view, SIGNAL(loadStarted()), this, SLOT(wetStart()));
    connect(view, SIGNAL(loadProgress(int)), this, SLOT(setProgress(int)));
    connect(view, SIGNAL(titleChanged(QString)), this, SLOT(adjustTitle()));
    connect(view, SIGNAL(loadFinished(bool)), this, SLOT(finishLoading(bool)));
    connect(this, SIGNAL(lastWindowClosed()), this, SLOT(quit()));

    locationEdit = new QLineEdit(this);
    connect(locationEdit, SIGNAL(returnPressed()), this, SLOT(changeLocation()));

    // 向工具栏添加动作和部件
    ui->mainToolBar->addAction(view->pageAction(QWebPage::Back));
    ui->mainToolBar->addAction(view->pageAction(QWebPage::Forward));
    ui->mainToolBar->addAction(view->pageAction(QWebPage::Reload));
    ui->mainToolBar->addAction(view->pageAction(QWebPage::Stop));
    ui->mainToolBar->addWidget(locationEdit);

    ui->LE_Demo->setText();
    // 设置并加载初始网页地址
//    locationEdit->setText(companyWebsite);
//    view->load(QUrl(companyWebsite));
}

MainWindow::~MainWindow()
{
    delete ui;
}

// 改变路径
void MainWindow::changeLocation()
{
    QUrl url = QUrl(locationEdit->text());
    view->load(url);
    view->setFocus();
}

// 更新进度
void MainWindow::setProgress(int p)
{
    progress = p;
    adjustTitle();
}

// 更新标题显示
void MainWindow::adjustTitle()
{
    if ( progress <= 0 || progress >= 100) {
        setWindowTitle(view->title());
    } else {
        setWindowTitle(QString("%1 (%2%)").arg(view->title()).arg(progress));
    }
}

// 加载完成后进行处理
void MainWindow::finishLoading(bool finished)
{
    static uint ErrOpenCount;
    if (finished) {
        TimerReTry->stop();
        ErrOpenCount = 0;
        progress = 100;
        setWindowTitle(view->title());

        time_End = (double)clock();
        qDebug()<<locationEdit->text()<<QString::fromLocal8Bit("耗时:")<<(time_End - time_Start)/1000.0<<"s";
        weChatOperate((time_End - time_Start)/1000.0);
//        view->close();
    } else {
        setWindowTitle("web page loading error!");
        ErrOpenCount++;

        if(QMessageBox::Retry == QMessageBox::warning(
                                 this,
                                 "O,NO!",
                                 QString::fromLocal8Bit("WebPageOpen Error %1 times!").arg(ErrOpenCount),
                                 QMessageBox::Ok,
                                 QMessageBox::Retry
                                )
          )
        {
           /*
            *直接调用LoadWebPage()函数无法重新加载网页，估计是无法进入事件
            *目前借用定时器来进入事件
            */
            TimerReTry = new QTimer(this);
            connect(TimerReTry,SIGNAL(timeout()),this,SLOT(LoadWebPage()));
            TimerReTry->start(10);
        }
    }
}

void MainWindow::wetStart()
{
    time_Start = (double)clock();
}

quint8 MainWindow::weChatOperate(const double &timer)
{
    QString GetWebResult;

    HWND hwnd = FindWindowA("WeChatMainWndForPC",NULL);

    if(hwnd == NULL){
        MessageBox(
             NULL,
            (LPCWSTR)L"WeChatOpenError!",
            (LPCWSTR)L"O,NO!",
            MB_ICONINFORMATION
        );
        qDebug()<<QString::fromLocal8Bit("找不到微信程序");
        return 0;
    }
    ShowWindow(hwnd, SW_MAXIMIZE);
    SetWindowPos(hwnd, HWND_TOP,0, 0, 1000, 700, SWP_SHOWWINDOW);//微信窗口置顶
    SetCursorPos(18,148);//窗口移动
    /*
    *模拟鼠标键盘按键操作
    */
    mouse_event(MOUSEEVENTF_LEFTDOWN,0,0,0,0);
    mouse_event(MOUSEEVENTF_LEFTUP,0,0,0,0);
    SetCursorPos(18,100);
    mouse_event(MOUSEEVENTF_LEFTDOWN,0,0,0,0);
    mouse_event(MOUSEEVENTF_LEFTUP,0,0,0,0);

    keybd_event(17,0,0,0);//模拟按下ctrl
    keybd_event('F',0,0,0);//模拟按下F
    Sleep(100);
    keybd_event(17,0,KEYEVENTF_KEYUP,0);
    keybd_event('F',0,KEYEVENTF_KEYUP,0);
    Sleep(1000);
//    TCHAR szText[] = L("文件传输助手"); //发送联系人 深圳睿佰博工作群
    QString qszText = QString::fromLocal8Bit("文件传输助手");
//    wchar_t *szText = (wchar_t*)reinterpret_cast<const wchar_t *>(qszText.utf16());
    TCHAR *szText = (wchar_t*)qszText.utf16();
    if (OpenClipboard(hwnd)) {//系统剪切板操作
            EmptyClipboard();
            HANDLE hData = GlobalAlloc(GMEM_MOVEABLE, 14);
            LPWSTR pData = (LPWSTR)GlobalLock(hData);
            CopyMemory(pData, szText, 14);
            GlobalUnlock(hData);
            SetClipboardData(CF_UNICODETEXT, hData);
            CloseClipboard();
        }
        keybd_event(17,0,0,0);
        keybd_event('V',0,0,0);
        Sleep(100);
        keybd_event(17,0,KEYEVENTF_KEYUP,0);
        keybd_event('V',0,KEYEVENTF_KEYUP,0);//ctrl + V

        Sleep(1000);//等待微信搜索联系人
        keybd_event(13,0,0,0);//按下 回车键
        keybd_event(13,0,KEYEVENTF_KEYUP,0);
//        TCHAR *sendText = L(QStringToTCHAR(GetWebResult));//发送的消息
//        TCHAR sendText[] = L"abc";//发送的消息
        TCHAR *sendText = NULL;
        int timerUsed = (int)timer > 10 ? 4 : 3;
        QString qSendText = QString::fromLocal8Bit("公司官网正常打开，用时约%1秒").arg(timerUsed);
        sendText = (wchar_t*)qSendText.utf16();

        if (OpenClipboard(hwnd)) {
            EmptyClipboard();
            HANDLE hData = GlobalAlloc(GMEM_MOVEABLE,40 );
            LPWSTR pData = (LPWSTR)GlobalLock(hData);
            CopyMemory(pData, sendText, 40);
            GlobalUnlock(hData);
            SetClipboardData(CF_UNICODETEXT, hData);
            CloseClipboard();
        }
        for(int i=0; i<1; i++){//发送消息次数
            Sleep(500);
            keybd_event(17,0,0,0);
            keybd_event('V',0,0,0);
            Sleep(100);
            keybd_event(17,0,KEYEVENTF_KEYUP,0);
            keybd_event('V',0,KEYEVENTF_KEYUP,0);
            Sleep(1000);
            keybd_event(13,0,0,0);
            keybd_event(13,0,KEYEVENTF_KEYUP,0);
        }
//        SetWindowPos(hwnd, HWND_TOP,0, 0, 1000, 700, SWP_HIDEWINDOW);//发送结束隐藏微信
        return 1;
}
void MainWindow::LoadWebPage()
{
    TimerReTry->stop();
    locationEdit->setText(companyWebsite);
    view->load(QUrl(companyWebsite));
}

void MainWindow::on_BT_Demo_clicked()
{
    ui->LE_Demo->text();

}
