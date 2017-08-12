#pragma once

#include <QtWidgets/QWidget>
#include "ui_test.h"

class test : public QWidget
{
    Q_OBJECT

public:
    test(QWidget *parent = Q_NULLPTR);

private:
    Ui::testClass ui;
};
