#include "Window.h"
#include "ui_window.h"
#include "GLWidget.h"
#include <QSpinBox>
#include <QSlider>


Window::Window(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Window)
{
    ui->setupUi(this);

    QObject::connect(findChild<QSpinBox*>("speedSpinBox"), SIGNAL(valueChanged(int)), findChild<QSlider*>("speedSlider"), SLOT(setValue(int)));
    QObject::connect(findChild<QSlider*>("speedSlider"), SIGNAL(valueChanged(int)), findChild<QSpinBox*>("speedSpinBox"), SLOT(setValue(int)));
    QObject::connect(findChild<QSlider*>("speedSlider"), SIGNAL(valueChanged(int)), findChild<GLWidget*>("glWidget"), SLOT(setSpeed(int)));

    QObject::connect(findChild<QRadioButton*>("distortVertices"), SIGNAL(clicked()), this, SLOT(distortRadio1Clicked()));
    QObject::connect(findChild<QRadioButton*>("distortTexCoords"), SIGNAL(clicked()), this, SLOT(distortRadio2Clicked()));

    QObject::connect(findChild<QRadioButton*>("imageRadioButton1"), SIGNAL(clicked()), this, SLOT(imageRadio1Clicked()));
    QObject::connect(findChild<QRadioButton*>("imageRadioButton2"), SIGNAL(clicked()), this, SLOT(imageRadio2Clicked()));
    QObject::connect(findChild<QRadioButton*>("imageRadioButton3"), SIGNAL(clicked()), this, SLOT(imageRadio3Clicked()));
}

Window::~Window()
{
    delete ui;
}

void Window::distortRadio1Clicked()
{
    findChild<GLWidget*>("glWidget")->setDistort(0);
}

void Window::distortRadio2Clicked()
{
    findChild<GLWidget*>("glWidget")->setDistort(1);
}

void Window::imageRadio1Clicked()
{
    findChild<GLWidget*>("glWidget")->setTexture(0);
}

void Window::imageRadio2Clicked()
{
    findChild<GLWidget*>("glWidget")->setTexture(1);
}

void Window::imageRadio3Clicked()
{
    findChild<GLWidget*>("glWidget")->setTexture(2);
}
