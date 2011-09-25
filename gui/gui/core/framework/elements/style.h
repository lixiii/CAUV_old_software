#ifndef __CAUV_GUI_ELEMENT_STYLE_H__
#define __CAUV_GUI_ELEMENT_STYLE_H__

#include <liquid/style.h>

namespace cauv{
namespace gui{

    const static liquid::ArcStyle Image_Arc_Style = {{
        QColor(255,255,255,235), 12,
        QColor(255,255,255), 6, 
        QColor(186,255,152),
        1, 12, 2, 16
    },{
        QColor(0,0,0,0), 12,
        QColor(0,0,0,128), 8,
        QColor(0,0,0,128),
        0, 14, 4, 17
    }
};

const static liquid::ArcStyle Param_Arc_Style = {{
        QColor(235,235,235,235), 12,
        QColor(235,235,235), 6,
        QColor(186,255,152),        
        1, 12, 2, 16
    },{
        QColor(0,0,0,0), 12,
        QColor(0,0,0,128), 8,
        QColor(0,0,0,128),
        0, 14, 4, 17
    }
};

const static qreal Cut_S = (14/16.0);
const static liquid::NodeStyle F_Node_Style = {
    QPen(QBrush(QColor(0,0,0,128)), 1, Qt::SolidLine, Qt::FlatCap),
    QBrush(QColor(243,243,243)),
    24, 24, {
        30,
        QPen(Qt::NoPen),
        QBrush(QColor(0,0,0,160)), {
            //QPen(QBrush(QColor(0,0,0,64)), 1, Qt::SolidLine, Qt::FlatCap),
            QPen(Qt::NoPen),
            QBrush(QColor(255,255,255)),
            QFont("Verdana", 12, 1)
        },{
            QPen(Qt::NoPen),
            //QPen(QBrush(QColor(0,0,0,64)), 1, Qt::SolidLine, Qt::FlatCap),
            QBrush(QColor(255,255,255)),
            QFont("Verdana", 8, 1)
        }
    },
    QPen(QColor(255,255,255)),
    QPen(QBrush(QColor(190,190,190)), 2, Qt::SolidLine, Qt::RoundCap),
    14*Cut_S, 4*Cut_S, 16*Cut_S,

    /*InputStyle*/ {
        {9*Cut_S, 1*Cut_S, 12*Cut_S},
        {9*Cut_S, 1*Cut_S, 0*Cut_S},
        {QPen(QBrush(QColor(128,128,128)), 1, Qt::SolidLine, Qt::FlatCap),
         QBrush(QColor(235,235,235))},
        {QPen(QBrush(QColor(160,160,160)), 1, Qt::SolidLine, Qt::FlatCap),
         QBrush(QColor(255,255,255))}
        /*{QPen(QBrush(QColor(88,126,168)), 1, Qt::SolidLine, Qt::FlatCap),
         QBrush(QColor(151,200,255))},
        {QPen(QBrush(QColor(134,102,154)), 1, Qt::SolidLine, Qt::FlatCap),
         QBrush(QColor(208,181,225))}*/
    }
};

const static liquid::NodeStyle AI_Node_Style = F_Node_Style;


} // namespace gui
} // namespace cauv

#endif // ndef __CAUV_GUI_ELEMENT_STYLE_H__

