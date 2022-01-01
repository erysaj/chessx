#ifndef PGNWIDGET_H
#define PGNWIDGET_H

#include <array>

#include <QtWidgets>
#include <QtQuickWidgets>

#include "board.h"
#include "gamex.h"

//class NotationOptions
//{
//public:
//    NotationOptions();

//    /** Font to use for move text (number, san and nags) */
//    const QFont &moveFont() const { return m_moveFont; }
//    /** Font to use for main line's move text */
//    const QFont &mainFont() const { return m_mainFont; }
//    /** Font to use for comments text */
//    const QFont &textFont() const { return m_textFont; }

//    const QColor &moveColor() const { return m_variationColor; }
//    const QColor &mainColor() const { return m_mainlineColor; }
//    const QColor &nagsColor() const { return m_nagsColor; }
//    const QColor &commentColor() const { return m_commentColor; }

//    bool showDiagrams() const { return m_showDiagrams; }
//    const QSize &diagramSize() const { return m_diagramSize; }
//    AnnotatedVariation::VisitingOptions visitingOptions() const;

//    int inlineDepthThreshold() const { return m_inlineDepthThreshold; }

//private:
//    QFont m_moveFont;
//    QFont m_mainFont;
//    QFont m_textFont;
//    QColor m_mainlineColor;
//    QColor m_variationColor;
//    QColor m_commentColor;
//    QColor m_nagsColor;
//    bool m_showDiagrams;
//    QSize m_diagramSize;
//    int m_inlineDepthThreshold;
//};

class PgnWidget: public QWidget {
    Q_OBJECT

public:
    PgnWidget(QWidget* parent = nullptr);
    ~PgnWidget();

    void showMove(int id);
    void reload(const GameX& game, bool trainingMode);

signals:
    void anchorClicked(const QUrl &url);

private:
    QQuickWidget *m_quickView;
};

#endif
