#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMainWindow>
class QWebView;
class QLineEdit;
class QTime;
namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();



private:
    Ui::MainWindow *ui;
    QWebView *view;
    QLineEdit *locationEdit;
    int progress;

protected slots:
    void changeLocation(); // 改变路径
    void setProgress(int); // 更新进度
    void adjustTitle();    // 更新标题显示
    void finishLoading(bool); // 加载完成后进行处理
    void wetStart(void);

    void LoadWebPage();
public:
//    QTimer *webTimer;
    double time_Start;
    double time_End;
    QTimer *TimerReTry;
    QString companyWebsite;
    quint8 weChatOperate(const double &);
};


#endif // MAINWINDOW_H
