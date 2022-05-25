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

    // �����źźͲ�    
    connect(view, SIGNAL(loadStarted()), this, SLOT(wetStart()));
    connect(view, SIGNAL(loadProgress(int)), this, SLOT(setProgress(int)));
    connect(view, SIGNAL(titleChanged(QString)), this, SLOT(adjustTitle()));
    connect(view, SIGNAL(loadFinished(bool)), this, SLOT(finishLoading(bool)));
    connect(this, SIGNAL(lastWindowClosed()), this, SLOT(quit()));

    locationEdit = new QLineEdit(this);
    connect(locationEdit, SIGNAL(returnPressed()), this, SLOT(changeLocation()));

    // �򹤾�����Ӷ����Ͳ���
    ui->mainToolBar->addAction(view->pageAction(QWebPage::Back));
    ui->mainToolBar->addAction(view->pageAction(QWebPage::Forward));
    ui->mainToolBar->addAction(view->pageAction(QWebPage::Reload));
    ui->mainToolBar->addAction(view->pageAction(QWebPage::Stop));
    ui->mainToolBar->addWidget(locationEdit);

    ui->LE_Demo->setText();
    // ���ò����س�ʼ��ҳ��ַ
//    locationEdit->setText(companyWebsite);
//    view->load(QUrl(companyWebsite));
}

MainWindow::~MainWindow()
{
    delete ui;
}

// �ı�·��
void MainWindow::changeLocation()
{
    QUrl url = QUrl(locationEdit->text());
    view->load(url);
    view->setFocus();
}

// ���½���
void MainWindow::setProgress(int p)
{
    progress = p;
    adjustTitle();
}

// ���±�����ʾ
void MainWindow::adjustTitle()
{
    if ( progress <= 0 || progress >= 100) {
        setWindowTitle(view->title());
    } else {
        setWindowTitle(QString("%1 (%2%)").arg(view->title()).arg(progress));
    }
}

// ������ɺ���д���
void MainWindow::finishLoading(bool finished)
{
    static uint ErrOpenCount;
    if (finished) {
        TimerReTry->stop();
        ErrOpenCount = 0;
        progress = 100;
        setWindowTitle(view->title());

        time_End = (double)clock();
        qDebug()<<locationEdit->text()<<QString::fromLocal8Bit("��ʱ:")<<(time_End - time_Start)/1000.0<<"s";
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
            *ֱ�ӵ���LoadWebPage()�����޷����¼�����ҳ���������޷������¼�
            *Ŀǰ���ö�ʱ���������¼�
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
        qDebug()<<QString::fromLocal8Bit("�Ҳ���΢�ų���");
        return 0;
    }
    ShowWindow(hwnd, SW_MAXIMIZE);
    SetWindowPos(hwnd, HWND_TOP,0, 0, 1000, 700, SWP_SHOWWINDOW);//΢�Ŵ����ö�
    SetCursorPos(18,148);//�����ƶ�
    /*
    *ģ�������̰�������
    */
    mouse_event(MOUSEEVENTF_LEFTDOWN,0,0,0,0);
    mouse_event(MOUSEEVENTF_LEFTUP,0,0,0,0);
    SetCursorPos(18,100);
    mouse_event(MOUSEEVENTF_LEFTDOWN,0,0,0,0);
    mouse_event(MOUSEEVENTF_LEFTUP,0,0,0,0);

    keybd_event(17,0,0,0);//ģ�ⰴ��ctrl
    keybd_event('F',0,0,0);//ģ�ⰴ��F
    Sleep(100);
    keybd_event(17,0,KEYEVENTF_KEYUP,0);
    keybd_event('F',0,KEYEVENTF_KEYUP,0);
    Sleep(1000);
//    TCHAR szText[] = L("�ļ���������"); //������ϵ�� ����۲�����Ⱥ
    QString qszText = QString::fromLocal8Bit("�ļ���������");
//    wchar_t *szText = (wchar_t*)reinterpret_cast<const wchar_t *>(qszText.utf16());
    TCHAR *szText = (wchar_t*)qszText.utf16();
    if (OpenClipboard(hwnd)) {//ϵͳ���а����
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

        Sleep(1000);//�ȴ�΢��������ϵ��
        keybd_event(13,0,0,0);//���� �س���
        keybd_event(13,0,KEYEVENTF_KEYUP,0);
//        TCHAR *sendText = L(QStringToTCHAR(GetWebResult));//���͵���Ϣ
//        TCHAR sendText[] = L"abc";//���͵���Ϣ
        TCHAR *sendText = NULL;
        int timerUsed = (int)timer > 10 ? 4 : 3;
        QString qSendText = QString::fromLocal8Bit("��˾���������򿪣���ʱԼ%1��").arg(timerUsed);
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
        for(int i=0; i<1; i++){//������Ϣ����
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
//        SetWindowPos(hwnd, HWND_TOP,0, 0, 1000, 700, SWP_HIDEWINDOW);//���ͽ�������΢��
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
