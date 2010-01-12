#ifndef QPROGRESSBARJUMP_H
#define QPROGRESSBARJUMP_H

#include <QProgressBar>
#include <QMouseEvent>
#include <QFontMetrics>
#include <QTimer>

class QProgressBarJump : public QProgressBar
{
    Q_OBJECT
protected:
    virtual void mousePressEvent ( QMouseEvent * event );
    void paintEvent( QPaintEvent * event );
public:
    QProgressBarJump(QWidget* parent = 0);
    void setText(const QString& text);
    void reset();
private:
    QString text;
    QTimer dblClickTimer;

Q_SIGNALS:
        void jumped(int);
	void finishedEditing(QWidget*);
};

#endif // QPROGRESSBARJUMP_H
