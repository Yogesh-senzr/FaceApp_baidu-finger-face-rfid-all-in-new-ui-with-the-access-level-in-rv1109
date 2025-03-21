#include "vitrualkeyboard.h"
#include "ui_vitrualkeyboard.h"

vitrualkeyboard::vitrualkeyboard(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::vitrualkeyboard)
{
    ui->setupUi(this);
}

vitrualkeyboard::~vitrualkeyboard()
{
    delete ui;
}
