#ifndef CLICKABLELABEL_H
#define CLICKABLELABEL_H

#include <QLabel>
#include <QMouseEvent>
#include <Qt>

class ClickableLabel : public QLabel
{
    Q_OBJECT

signals:
    void clicked(QPoint);

public:
    explicit ClickableLabel(QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
    ~ClickableLabel();
    void mousePressEvent(QMouseEvent* event);

};

#endif // CLICKABLELABEL_H
