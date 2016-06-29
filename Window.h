#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>

namespace Ui {
class Window;
}

class Window : public QWidget
{
    Q_OBJECT

public:
    explicit Window(QWidget *parent = 0);
    ~Window();

private:
    Ui::Window *ui;

public slots:
    void distortRadio1Clicked();
    void distortRadio2Clicked();

    void imageRadio1Clicked();
    void imageRadio2Clicked();
    void imageRadio3Clicked();
};

#endif // WINDOW_H
