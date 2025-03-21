#ifndef VITRUALKEYBOARD_H
#define VITRUALKEYBOARD_H

#include <QWidget>

namespace Ui {
class vitrualkeyboard;
}
class vitrualkeyboard : public QWidget
{
    Q_OBJECT
public:
    explicit vitrualkeyboard(QWidget *parent = 0);
    ~vitrualkeyboard();
private:
    Ui::vitrualkeyboard *ui;
};

#endif // VITRUALKEYBOARD_H
