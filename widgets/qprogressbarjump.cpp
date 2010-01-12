#include "qprogressbarjump.h"
#include <QPainter>
#include <QApplication>
#include <QDebug>

QProgressBarJump::QProgressBarJump(QWidget*parent) : QProgressBar(parent)
{
    dblClickTimer.setSingleShot(true);
    dblClickTimer.setInterval(qApp->doubleClickInterval());
}

void QProgressBarJump::mousePressEvent ( QMouseEvent * event )
{
    if (event->button() == Qt::RightButton)
    { // Help and jump to the end (min, max) that is nearer to the current position
            if (maximum() - value() < value() - minimum())
                setValue(maximum());
            else
                setValue(minimum());    
	 event->accept();
    }
    else if (event->button() == Qt::LeftButton)
    {
        if (orientation() == Qt::Vertical)
            setValue(minimum() + ((maximum()-minimum()) * (height()-event->y())) / height() ) ;
        else
            setValue(minimum() + ((maximum()-minimum()) * event->x()) / width() ) ;

        if (dblClickTimer.isActive()) {
            dblClickTimer.stop();
            if (maximum() - value() < value() - minimum())
                setValue(maximum());
            else
                setValue(minimum());
        } else {
            dblClickTimer.stop();
            dblClickTimer.start();
        }
        event->accept();
    } 
    QProgressBar::mousePressEvent(event);
    emit jumped(value());
    emit finishedEditing(this);
}

void QProgressBarJump::paintEvent( QPaintEvent * event )
{
    QProgressBar::paintEvent(event);
    if (text.isEmpty()) return;
    QPainter painter(this);
    QFontMetrics fm(font());
    painter.drawText((width()-fm.width(text))/2,(height()+fm.ascent()-fm.descent())/2,text);
}

void QProgressBarJump::setText(const QString& text)
{
    this->text = text;
    update();
}

void QProgressBarJump::reset() {
    text = QString();
    QProgressBar::reset();
}
