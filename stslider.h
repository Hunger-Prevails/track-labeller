#ifndef STSLIDER
#define STSLIDER

#include <QSlider>
#include <QProxyStyle>

/////////user define player slider
class StStyle : public QProxyStyle
{
public:
    int styleHint(StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const
    {
        if (QStyle::SH_Slider_AbsoluteSetButtons)
        {
            return (Qt::LeftButton | Qt::MidButton | Qt::RightButton);
        }
        return QProxyStyle::styleHint(hint, option, widget, returnData);
    }
};

class StSlider : public QSlider
{
    Q_OBJECT
public:
    explicit StSlider(Qt::Orientation orientation, QWidget *parent = 0);

protected:
    void keyPressEvent(QKeyEvent *ev);
    void keyReleaseEvent(QKeyEvent *ev);

};///////////

#endif // STSLIDER

