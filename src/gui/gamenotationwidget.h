#ifndef GAMENOTATIONWIDGET_H
#define GAMENOTATIONWIDGET_H

#include <array>

#include <QUrl>
#include <QAction>
#include <QWidget>
#include <QGraphicsScene>
#include <QGraphicsWidget>
#include <QGraphicsLayout>

#include "nag.h"
#include "editaction.h"
#include "gamex.h"

class ChessBrowser;
class QGraphicsView;
class Output;


/**
 * Base class for all custom widget items.
 */
class GraphicsControl: public QGraphicsWidget
{
    Q_OBJECT

public:
    /** Item's state flags */
    enum StateFlags: unsigned
    {
        State_Default           = 0x00,
        State_Selected          = 0x01,
        State_Highlighted       = 0x02,
    };

    GraphicsControl(QGraphicsItem *parent = nullptr)
        : QGraphicsWidget(parent)
    {}

    bool hasState(GraphicsControl::StateFlags mask) const { return (m_statesMask & mask) != 0; }
    void setState(GraphicsControl::StateFlags mask, bool value);

signals:
    void stateChanged(GraphicsControl::StateFlags changeMask);

protected:
    // flags management
    virtual void handleStateChanged(GraphicsControl::StateFlags changeMask);

private:
    GraphicsControl::StateFlags m_statesMask = State_Default;
    void setStatesMask(GraphicsControl::StateFlags mask);
};


/**
 * Base class for all "clickable" items
 */
class NotationLink: public GraphicsControl
{
    Q_OBJECT

public:
    NotationLink(QGraphicsItem *parent = nullptr)
        : GraphicsControl(parent)
    {}

    const QUrl &href() const { return m_href; }
    void setHref(const QUrl &url) { m_href = url; }

signals:
    void linkClicked(const QUrl &href);

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override final;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override final;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

    virtual void handleClick(Qt::MouseButton button);

private:
    using SUPER = GraphicsControl;
    QUrl m_href;
};


class NotationTextLink: public NotationLink
{
    Q_OBJECT

public:
    NotationTextLink(QGraphicsItem *parent = nullptr);

    void setText(const QString &text) { m_textItem->setText(text); }

    void setColor(const QColor &color) { m_textItem->setBrush(color); }
    void setFont(const QFont &font) { m_textItem->setFont(font); }

    void setGeometry(const QRectF &rect) override;
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const override;

protected:
    void handleStateChanged(StateFlags changeMask) override;

private:
    using SUPER = NotationLink;

    QGraphicsRectItem *m_backgroundItem = nullptr;
    QGraphicsSimpleTextItem *m_textItem = nullptr;

    void updateBackground();
};


class NotationMoveLink: public NotationLink
{
    Q_OBJECT

public:
    enum Part
    {
        kPart_MoveCounter = 0,
        kPart_MoveText,
        kPart_NagText,

        kParts_Count,
    };

    NotationMoveLink(const QString &moveCounter, const QString &moveText, const QString &nagText, QGraphicsItem *parent = nullptr);

    void setGeometry(const QRectF &rect) override;
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const override;

private:
    std::array<QGraphicsSimpleTextItem*, kParts_Count> m_parts;

    void setup(Part part, const QString &text);
};

class NotationOptions
{
public:
    NotationOptions();

    /** Font to use for move text (number, san and nags) */
    const QFont &moveFont() const { return m_moveFont; }
    /** Font to use for main line's move text */
    const QFont &mainFont() const { return m_mainFont; }
    /** Font to use for comments text */
    const QFont &textFont() const { return m_textFont; }

    const QColor &moveColor() const { return m_variationColor; }
    const QColor &mainColor() const { return m_mainlineColor; }
    const QColor &nagsColor() const { return m_nagsColor; }
    const QColor &commentColor() const { return m_commentColor; }

    bool showDiagrams() const { return m_showDiagrams; }
    const QSize &diagramSize() const { return m_diagramSize; }
    AnnotatedVariation::VisitingOptions visitingOptions() const;

    int inlineDepthThreshold() const { return m_inlineDepthThreshold; }

private:
    QFont m_moveFont;
    QFont m_mainFont;
    QFont m_textFont;
    QColor m_mainlineColor;
    QColor m_variationColor;
    QColor m_commentColor;
    QColor m_nagsColor;
    bool m_showDiagrams;
    QSize m_diagramSize;
    int m_inlineDepthThreshold;
};

class VariationContainerWidget: public QGraphicsWidget, protected AnnotatedVariation::IVisitor
{
    Q_OBJECT

public:
    VariationContainerWidget(int depth, QGraphicsItem *parent = nullptr);

    /** Rebuild children to reflect given variation */
    void setVariation(const AnnotatedVariation &variation);

    const QHash<MoveId, NotationLink*> links() const { return m_links; }

signals:
    void linkClicked(const QUrl &url);

protected:
    const NotationOptions &options() const { return m_options; }
    int baseDepth() const { return m_baseDepth; }

    static QStringList cutText(const QString &text);
    NotationTextLink *createTextItem(const QString &text);

private:
    virtual void prepare() = 0;
    virtual void finish() {}

private:
    NotationOptions m_options;
    int m_baseDepth;
protected:
    QHash<MoveId, NotationLink*> m_links;
};

class NotationScene: public QGraphicsScene
{
    Q_OBJECT

public:
    NotationScene(QObject *parent = nullptr);

    void reload(const GameX &game, const QSize &viewSize);
    QGraphicsItem* selectMove(MoveId moveId);

signals:
    void linkClicked(const QUrl &url);

private:
    QHash<MoveId, NotationLink*> m_moveLinks;
    MoveId m_currentMoveId = NO_MOVE;
};


class GameNotationWidget : public QWidget
{
    Q_OBJECT

public:
    GameNotationWidget(QWidget* parent = nullptr);
    ~GameNotationWidget();

    QString getHtml() const;
    QString getText() const;

    QString generateText(const GameX& game, bool trainingMode);
    void reload(const GameX& game, bool trainingMode);

    QMap<Nag, QAction*> nagActions() const;

public slots:
    /** Store current configuration. */
    void saveConfig();
    /** Restore current configuration. */
    void slotReconfigure();
    /** Scroll to show given mode. */
    void showMove(int id);

signals:
    void anchorClicked(const QUrl &url);
    void actionRequested(const EditAction& action);
    void queryActiveGame(const GameX** game);
    void signalMergeGame(GameId gameIndex, QString source);

private:
    void configureFont();

    ChessBrowser *m_browser;
    QGraphicsView *m_graphicsView;
    NotationScene *m_scene;
    Output *m_output;
};

#endif
