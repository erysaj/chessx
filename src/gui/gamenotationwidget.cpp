#include "gamenotationwidget.h"

#include <QtWidgets>

#include "graphicsflowlayout.h"
#include "chessbrowser.h"
#include "settings.h"
#include "output.h"
#include "boardview.h"


//("/GameText/SymbolicNag", true);
//("/GameText/VariationIndentLevel", 1);
//("/GameText/VariationIndentSize", 3);
//("/GameText/CommentIndent", "OnlyMainline");
//("/GameText/HeaderColor", "blue");
//("/GameText/ShowHeader", false);

static QUrl makeCommentUrl(MoveId moveId, GameX::Position position)
{
    QUrl url(QString("cmt:%1").arg(moveId));
    if (position == GameX::BeforeMove)
    {
        url.setScheme("precmt");
    }
    return url;
}

static QUrl makeMoveUrl(MoveId moveId)
{
    return { QString("move:%1").arg(moveId) };
}

namespace {

class DiagramItem : public QGraphicsLayoutItem, public QGraphicsItem
{
public:
    DiagramItem(const BoardX &board, const QSize size, QGraphicsItem *parent = nullptr);

    // Inherited from QGraphicsLayoutItem
    void setGeometry(const QRectF &geom) override;
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const override;

    // Inherited from QGraphicsItem
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

private:
    QPixmap m_pix;
};

class NotationCell: public NotationLink
{
public:
    NotationCell(QGraphicsItem *parent = nullptr);

    void setText(const QString &text) { m_textItem->setText(text); }

    void setGeometry(const QRectF &rect) override;

protected:
    void handleStateChanged(StateFlags changeMask) override;

private:
    using SUPER = NotationLink;

    QGraphicsRectItem *m_backgroundItem = nullptr;
    QGraphicsSimpleTextItem *m_textItem = nullptr;

    void updateBackground();
};


class BlockVariationContainer: public VariationContainerWidget
{
public:
    BlockVariationContainer(int depth, QGraphicsItem *parent = nullptr);

protected:
    void visitComment(const QString &text, MoveId moveId, GameX::Position position) override;
    void visitMoves(const MoveIterator &first, const MoveIterator &last) override;
    void visitDiagram(const BoardX &board) override;
    void visitVariation(const AnnotatedVariation &variation) override;

private:
    void prepare() override;

    void addMove(MoveIterator &begin, const MoveIterator &end);

    NotationLink *makeMoveLink(const QString &moveCounter, const MoveIterator &moveIt);

    /** Add item to the current flow layout */
    void addItem(QGraphicsLayoutItem *item);
    /** Finish the current flow layout to start the next line */
    void flush();

    int m_currDepth;
    QGraphicsLinearLayout *m_layout;
    GraphicsFlowLayout *m_currLayout;
};


class TableVariationContainer: public VariationContainerWidget
{
public:
    TableVariationContainer(QGraphicsItem *parent = nullptr);

protected:
    void visitComment(const QString &text, MoveId moveId, GameX::Position position) override;
    void visitMoves(const MoveIterator &first, const MoveIterator &last) override;
    void visitDiagram(const BoardX &board) override;
    void visitVariation(const AnnotatedVariation &variation) override;

    /** Displayed in move numbers column */
    virtual QGraphicsLayoutItem *createGridMoveIndicatorItem(int moveNumber);
    /** Displays "..." text if fragment starts by blacks' move or breaks after whites' move */
    virtual QGraphicsLayoutItem *createGridContinuationItem();
    /** Displayed in the last cell of the variation ending by whites' move */
    virtual QGraphicsLayoutItem *createGridEmptyItem();
    /** Displays move text */
    virtual NotationLink *createGridMoveLink(const MoveIterator &moveIt);

    virtual QGraphicsLayoutItem *createDiagram(const BoardX &board);

private:
    enum Columns
    {
        Column_MoveNumber = 0,
        Column_White,
        Column_Black,
        Column_Extra,
    };

    void prepare() override;
    NotationLink *makeMoveLink(const MoveIterator &moveIt);

    QGraphicsGridLayout *m_layout;
};


} // end of anonymous namespace


DiagramItem::DiagramItem(const BoardX &board, const QSize size, QGraphicsItem *parent)
    : QGraphicsLayoutItem(), QGraphicsItem(parent)
    , m_pix(QPixmap::fromImage(BoardView::renderImageForBoard(board, size)))
{
    setGraphicsItem(this);
}

void DiagramItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);
    Q_UNUSED(option);

    QRectF bounds(QPointF(0, 0), geometry().size());
    const QSize pixSize = m_pix.size();

    // paint a rect around the pixmap (with gradient)
    QPointF pixPos(bounds.left(), bounds.center().y() - 0.5 * pixSize.height());
    painter->drawPixmap(pixPos, m_pix);
}

QRectF DiagramItem::boundingRect() const
{
    return QRectF(QPointF(0, 0), geometry().size());
}

void DiagramItem::setGeometry(const QRectF &geom)
{
    prepareGeometryChange();
    QGraphicsLayoutItem::setGeometry(geom);
    setPos(geom.topLeft());
}

QSizeF DiagramItem::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    switch (which)
    {
    case Qt::MinimumSize:
    case Qt::PreferredSize:
        return m_pix.size();
    case Qt::MaximumSize:
        return QSizeF(1000, 1000);
    default:
        break;
    }
    return constraint;
}

NotationOptions::NotationOptions()
    : m_moveFont(AppSettings->value("/GameText/FontBrowserMove").toString(), AppSettings->value("GameText/FontSize").toInt(), QFont::Normal)
    , m_mainFont(m_moveFont)
    , m_textFont(AppSettings->value("/GameText/FontBrowserText").toString(), AppSettings->value("/GameText/FontSize").toInt(), QFont::Normal)
    , m_mainlineColor(AppSettings->value("/GameText/MainLineMoveColor").value<QColor>())
    , m_variationColor(AppSettings->value("/GameText/VariationColor").value<QColor>())
    , m_commentColor(AppSettings->value("/GameText/CommentColor").value<QColor>())
    , m_nagsColor(AppSettings->value("/GameText/NagColor").value<QColor>())
    , m_showDiagrams(AppSettings->value("/GameText/ShowDiagrams").toBool())
    , m_diagramSize()
    , m_inlineDepthThreshold(AppSettings->value("/GameText/VariationIndentLevel").toInt())
{
    auto diagramSize = AppSettings->value("/GameText/DiagramSize").toInt();
    m_diagramSize.setWidth(diagramSize);
    m_diagramSize.setHeight(diagramSize);

    m_mainFont.setBold(true);
}

AnnotatedVariation::VisitingOptions NotationOptions::visitingOptions() const
{
    unsigned options = 0;
    if (showDiagrams())
    {
        options |= AnnotatedVariation::Visit_Diagrams;
    }
    return static_cast<AnnotatedVariation::VisitingOptions>(options);
}

VariationContainerWidget::VariationContainerWidget(int depth, QGraphicsItem *parent)
    : QGraphicsWidget(parent)
    , m_options()
    , m_baseDepth(depth)
    , m_links()
{

}

void VariationContainerWidget::setVariation(const AnnotatedVariation &variation)
{
    setLayout(nullptr);
    m_links.clear();

    prepare();
    variation.visit(*this, m_options.visitingOptions());
    finish();
}

QStringList VariationContainerWidget::cutText(const QString &text)
{
    QStringList chunks;
    QRegularExpression reChunk("\\S+\\s*");

    auto it = reChunk.globalMatch(text);
    while (it.hasNext())
    {
        auto match = it.next();
        if (match.hasMatch())
        {
            chunks << match.captured(0);
        }
    }
    return chunks;
}

NotationTextLink *VariationContainerWidget::createTextItem(const QString &text)
{
    auto *item = new NotationTextLink();
    item->setText(text);
    item->setFont(options().textFont());
    item->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed, QSizePolicy::Label);
    return item;
}

BlockVariationContainer::BlockVariationContainer(int depth, QGraphicsItem *parent)
    : VariationContainerWidget(depth, parent)
    , m_currDepth(depth)
    , m_layout(nullptr)
    , m_currLayout(nullptr)
{

}

void BlockVariationContainer::prepare()
{
    auto *layout = new QGraphicsLinearLayout();
    layout->setOrientation(Qt::Vertical);
    layout->setContentsMargins(0.0, 0.0, 0.0, 0.0);
    layout->setSpacing(0.0);

    m_layout = layout;
    setLayout(m_layout);
}

void BlockVariationContainer::addItem(QGraphicsLayoutItem *item)
{
    if (!m_currLayout)
    {
        m_currLayout = new GraphicsFlowLayout(0.0, 0.0);

        auto *widget = new QGraphicsWidget();
        widget->setLayout(m_currLayout);
        widget->setContentsMargins(20.0 * m_currDepth, 0.0, 0.0, 0.0);

        m_layout->addItem(widget);
    }
    m_currLayout->addItem(item);
}

void BlockVariationContainer::flush()
{
    m_currLayout = nullptr;
}

void BlockVariationContainer::addMove(MoveIterator &begin, const MoveIterator &end)
{
    const auto &board = begin.srcBoard();
    const QString fmt = board.toMove() == White? "%1.": "%1…";
    const QString moveCounter = fmt.arg(board.moveNumber());

    if (begin.srcBoard().toMove() == White)
    {
        // add white move item
        addItem(makeMoveLink(moveCounter, begin));
        ++begin;

        if (begin != end)
        {
            // we have response in the current sequence -> add black move item
            addItem(makeMoveLink("", begin));
            ++begin;
        }
    }
    else if (begin.srcBoard().toMove() == Black)
    {
        addItem(makeMoveLink(moveCounter, begin));
        ++begin;
    }
}

void BlockVariationContainer::visitComment(const QString &text, MoveId moveId, GameX::Position position)
{
    auto href = makeCommentUrl(moveId, position);
    if (m_currDepth == 0)
    {
        flush();
    }

    const auto chunks = cutText(text);
    for (const auto &chunk: chunks)
    {
        auto *item = createTextItem(chunk);
        item->setColor(options().commentColor());
        item->setHref(href);
        connect(item, &NotationLink::linkClicked, this, &VariationContainerWidget::linkClicked);
        addItem(item);
    }

    if (m_currDepth == 0)
    {
        flush();
    }
}

void BlockVariationContainer::visitMoves(const MoveIterator &first, const MoveIterator &last)
{
    for (auto moveIt = first; moveIt != last; )
    {
        addMove(moveIt, last);
    }
}

void BlockVariationContainer::visitDiagram(const BoardX &board)
{
    flush();

    auto *layout = new QGraphicsLinearLayout();
    layout->setOrientation(Qt::Horizontal);
    layout->setContentsMargins(0, 0, 0, 0);

    auto *widget = new QGraphicsWidget();
    widget->setLayout(layout);

    auto *diagram = new DiagramItem(board, options().diagramSize());
    layout->addItem(diagram);
    layout->addStretch();

    m_layout->addItem(widget);
}

void BlockVariationContainer::visitVariation(const AnnotatedVariation &variation)
{
    m_currDepth += 1;

    if (m_currDepth < options().inlineDepthThreshold())
    {
        flush();
        addItem(createTextItem("["));
        variation.visit(*this, options().visitingOptions());
        addItem(createTextItem("]"));
        flush();
    }
    else
    {
        addItem(createTextItem("("));
        variation.visit(*this, options().visitingOptions());
        addItem(createTextItem(")"));
    }

    m_currDepth -= 1;
}

NotationLink *BlockVariationContainer::makeMoveLink(const QString &moveCounter, const MoveIterator &moveIt)
{
    auto moveId = moveIt.dstMoveId();
    const auto moveText = moveIt.srcBoard().moveToSan(moveIt.move());
    const auto nagText = "";

    auto *link = new NotationMoveLink(moveCounter, moveText, nagText);
    link->setHref(makeMoveUrl(moveId));
//    if (m_currDepth == 0)
//    {
//        // main line
//        link->setFont(options().mainFont());
//        link->setColor(options().mainColor());
//    }
//    else
//    {
//        link->setFont(options().moveFont());
//        link->setColor(options().moveColor());
//    }
    link->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed, QSizePolicy::Label);
    link->setAcceptHoverEvents(true);
    link->setAcceptedMouseButtons(Qt::LeftButton);
    m_links[moveId] = link;
    connect(link, &NotationLink::linkClicked, this, &VariationContainerWidget::linkClicked);
    return link;
}

TableVariationContainer::TableVariationContainer(QGraphicsItem *parent)
    : VariationContainerWidget(0, parent)
    , m_layout(nullptr)
{

}

void TableVariationContainer::prepare()
{
    auto *layout = new QGraphicsGridLayout();
    layout->setContentsMargins(0.0, 0.0, 0.0, 0.0);
    layout->setSpacing(0.0);

    m_layout = layout;
    setLayout(m_layout);
}

void TableVariationContainer::visitComment(const QString &text, MoveId moveId, GameX::Position position)
{
    auto href = makeCommentUrl(moveId, position);
    auto *layout = new GraphicsFlowLayout(0.0, 0.0);

    const auto chunks = cutText(text);
    for (const auto &chunk: chunks)
    {
        auto *item = createTextItem(chunk);
        item->setColor(options().commentColor());
        item->setHref(href);
        connect(item, &NotationLink::linkClicked, this, &VariationContainerWidget::linkClicked);
        layout->addItem(item);
    }

    auto *widget = new QGraphicsWidget();
    widget->setLayout(layout);

    m_layout->addItem(widget, m_layout->rowCount(), 0, 1, 4);
}

void TableVariationContainer::visitMoves(const MoveIterator &first, const MoveIterator &last)
{
    auto row = m_layout->rowCount();
    for (auto moveIt = first; moveIt != last; ++row)
    {
        auto *moveIndicator = createGridMoveIndicatorItem(moveIt.moveNumber());
        m_layout->addItem(moveIndicator, row, Column_MoveNumber);

        if (moveIt.srcBoard().toMove() == White)
        {
            // add white move item
            auto *whiteLink = makeMoveLink(moveIt);
            m_layout->addItem(whiteLink, row, Column_White);
            ++moveIt;

            if (moveIt != last)
            {
                // we have response in the current sequence -> add black move item
                auto *blackLink = makeMoveLink(moveIt);
                m_layout->addItem(blackLink, row, Column_Black);
                ++moveIt;
            }
            else if (moveIt)
            {
                // current sequence ended, but the line continues
                auto *blackItem = createGridContinuationItem();
                m_layout->addItem(blackItem, row, Column_Black);
            }
            else
            {
                // line ended
                auto *blackItem = createGridEmptyItem();
                m_layout->addItem(blackItem, row, Column_Black);
            }
        }
        else if (moveIt.srcBoard().toMove() == Black)
        {
            // this should happen at most once, since we usually advance by 2
            auto *whiteItem = createGridContinuationItem();
            m_layout->addItem(whiteItem, row, Column_White);

            auto *blackLink = makeMoveLink(moveIt);
            m_layout->addItem(blackLink, row, Column_Black);
            ++moveIt;
        }
    }
}

void TableVariationContainer::visitDiagram(const BoardX &board)
{
    auto *item = createDiagram(board);
    m_layout->addItem(item, m_layout->rowCount(), Column_White, 1, 2);
}

void TableVariationContainer::visitVariation(const AnnotatedVariation &variation)
{
    auto *widget = new BlockVariationContainer(baseDepth() + 1);
    widget->setVariation(variation);
    connect(widget, &VariationContainerWidget::linkClicked, this, &VariationContainerWidget::linkClicked);
    m_layout->addItem(widget, m_layout->rowCount(), 0, 1, 4);
}

QGraphicsLayoutItem *TableVariationContainer::createGridMoveIndicatorItem(int moveNumber)
{
    auto *item = new NotationCell();
    item->setPreferredSize(100, 25);
    item->setText(QString::number(moveNumber));
    return item;
}

QGraphicsLayoutItem *TableVariationContainer::createGridContinuationItem()
{
    auto *item = new NotationCell();
    item->setPreferredSize(100, 25);
    item->setText("...");
    return item;
}

QGraphicsLayoutItem *TableVariationContainer::createGridEmptyItem()
{
    auto *item = new NotationCell();
    item->setPreferredSize(100, 25);
    item->setText("");
    return item;
}

NotationLink *TableVariationContainer::createGridMoveLink(const MoveIterator &moveIt)
{
    auto moveId = moveIt.dstMoveId();

    auto *link = new NotationCell();
    link->setPreferredSize(100, 25);
    link->setHref(makeMoveUrl(moveId));
    link->setText(moveIt.srcBoard().moveToSan(moveIt.move()));
    return link;
}

QGraphicsLayoutItem *TableVariationContainer::createDiagram(const BoardX &board)
{
    return new DiagramItem(board, options().diagramSize());
}

NotationLink *TableVariationContainer::makeMoveLink(const MoveIterator &moveIt)
{
    auto moveId = moveIt.dstMoveId();
    auto *link = createGridMoveLink(moveIt);
    link->setAcceptHoverEvents(true);
    link->setAcceptedMouseButtons(Qt::LeftButton);
    m_links[moveId] = link;
    return link;
}

void GraphicsControl::setStatesMask(StateFlags mask)
{
    auto change = static_cast<StateFlags>(mask ^ m_statesMask);
    if (change)
    {
        m_statesMask = mask;
        handleStateChanged(change);
    }
}

void GraphicsControl::handleStateChanged(StateFlags changeMask)
{
    emit stateChanged(changeMask);
}

void GraphicsControl::setState(StateFlags mask, bool value)
{
    setStatesMask(static_cast<StateFlags>(value? (m_statesMask | mask): (m_statesMask & ~mask)));
}


void NotationLink::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    SUPER::hoverEnterEvent(event);
    setState(State_Highlighted, true);
}

void NotationLink::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    SUPER::hoverLeaveEvent(event);
    setState(State_Highlighted, false);
}

void NotationLink::mousePressEvent(QGraphicsSceneMouseEvent *event)
{    
    handleClick(event->button());
}

void NotationLink::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);
}

void NotationLink::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);
}

void NotationLink::handleClick(Qt::MouseButton button)
{
    if (!m_href.isEmpty() && (button & Qt::LeftButton))
    {
        emit linkClicked(m_href);
    }
}

NotationScene::NotationScene(QObject *parent)
    : QGraphicsScene(parent)
    , m_moveLinks()
{
}

void NotationScene::reload(const GameX &game, const QSize &viewSize)
{
    clear();
    m_moveLinks.clear();
    selectMove(NO_MOVE);

    NotationOptions options;

    AnnotatedVariation mainLine(game);

    VariationContainerWidget *rootWdgt = nullptr;
    if (AppSettings->value("/GameText/ColumnStyle").toBool())
    {
        rootWdgt = new TableVariationContainer(0);
    }
    else
    {
        rootWdgt = new BlockVariationContainer(0);
    }
    rootWdgt->setVariation(mainLine);
    m_moveLinks = rootWdgt->links();

    connect(rootWdgt, &VariationContainerWidget::linkClicked, this, &NotationScene::linkClicked);

    addItem(rootWdgt);

    const qreal W = viewSize.width();
    const qreal H = rootWdgt->effectiveSizeHint(Qt::PreferredSize, { W, -1 }).height();
    setSceneRect(0, 0, W, H);

    rootWdgt->setGeometry(0, 0, W, H);

    selectMove(game.currentMove());
}

QGraphicsItem* NotationScene::selectMove(MoveId moveId)
{
    if (m_currentMoveId != moveId)
    {
        if (m_currentMoveId != NO_MOVE)
        {
            auto link = m_moveLinks.value(m_currentMoveId, nullptr);
            if (link)
            {
                link->setState(GraphicsControl::State_Selected, false);
            }
        }

        m_currentMoveId = moveId;

        if (m_currentMoveId != NO_MOVE)
        {
            auto link = m_moveLinks.value(m_currentMoveId, nullptr);
            if (link)
            {
                link->setState(GraphicsControl::State_Selected, true);
            }
        }
    }
    return m_moveLinks.value(m_currentMoveId, nullptr);
}

NotationCell::NotationCell(QGraphicsItem *parent)
    : NotationLink(parent)
{
    QPen bgPen(QColorConstants::Black, 0, Qt::NoPen);
    m_backgroundItem = new QGraphicsRectItem(this);
    m_backgroundItem->setPen(bgPen);

    m_textItem = new QGraphicsSimpleTextItem(this);

    updateBackground();
}

void NotationCell::setGeometry(const QRectF &rect)
{
    QGraphicsWidget::setGeometry(rect);

    const auto g = geometry();
    m_backgroundItem->setRect(0, 0, g.width(), g.height());
}

void NotationCell::handleStateChanged(StateFlags changeMask)
{
    SUPER::handleStateChanged(changeMask);
    if (changeMask & (State_Selected | State_Highlighted))
    {
        updateBackground();
    }
}

void NotationCell::updateBackground()
{
    if (hasState(State_Selected))
    {
        m_backgroundItem->setBrush(QColorConstants::DarkBlue);
    }
    else if (hasState(State_Highlighted))
    {
        m_backgroundItem->setBrush(QColorConstants::Blue);
    }
    else
    {
        m_backgroundItem->setBrush(QColorConstants::LightGray);
    }
}


NotationTextLink::NotationTextLink(QGraphicsItem *parent)
    : NotationLink(parent)
{
    QPen bgPen(QColorConstants::Black, 0, Qt::NoPen);
    m_backgroundItem = new QGraphicsRectItem(this);
    m_backgroundItem->setPen(bgPen);

    m_textItem = new QGraphicsSimpleTextItem(this);

    updateBackground();
}

void NotationTextLink::setGeometry(const QRectF &rect)
{
    NotationLink::setGeometry(rect);

    const auto g = geometry();
    m_backgroundItem->setRect(0, 0, g.width(), g.height());
}

QSizeF NotationTextLink::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    Q_UNUSED(constraint)

    QSizeF size;
    QFontMetrics fm(m_textItem->font());

    switch (which)
    {
    case Qt::MinimumSize:
    case Qt::PreferredSize:
    case Qt::MaximumSize:
    {
        const auto text = m_textItem->text();
        size.setWidth(fm.width(text));
        size.setHeight(fm.height());
        break;
    }
    case Qt::MinimumDescent:
        size.setWidth(0);
        size.setHeight(fm.descent());
        break;
    default:
        break;
    }
    return size;
}

void NotationTextLink::handleStateChanged(StateFlags changeMask)
{
    SUPER::handleStateChanged(changeMask);
    if (changeMask & (State_Selected | State_Highlighted))
    {
        updateBackground();
    }
}

void NotationTextLink::updateBackground()
{
    if (hasState(State_Selected))
    {
        m_backgroundItem->setBrush(QColorConstants::DarkBlue);
    }
    else if (hasState(State_Highlighted))
    {
        m_backgroundItem->setBrush(QColorConstants::Blue);
    }
    else
    {
        m_backgroundItem->setBrush(QColorConstants::LightGray);
    }
}

NotationMoveLink::NotationMoveLink(const QString &moveCounter, const QString &moveText, const QString &nagText, QGraphicsItem *parent)
    : NotationLink(parent)
    , m_parts { nullptr, nullptr, nullptr }
{
    setup(kPart_MoveCounter, moveCounter);
    setup(kPart_MoveText, moveText);
    setup(kPart_NagText, nagText);
}

void NotationMoveLink::setup(Part part, const QString &text)
{
    if (text.isEmpty())
        return;
    auto *item = m_parts[part] = new QGraphicsSimpleTextItem(this);
    item->setText(text);
}

void NotationMoveLink::setGeometry(const QRectF &rect)
{
    NotationLink::setGeometry(rect);

    const auto descent = effectiveSizeHint(Qt::MinimumDescent).height();

    const auto g = geometry();
    qreal x = 0.0;
    for (auto *textItem: m_parts)
    {
        if (!textItem)
            continue;
        QFontMetrics fm(textItem->font());
        const auto text = textItem->text();
        const auto w = static_cast<qreal>(fm.width(text));
        const auto h = static_cast<qreal>(fm.height());
        textItem->setPos(x, g.height() - descent + fm.descent() - h);
        x += w;
    }
//    m_backgroundItem->setRect(0, 0, g.width(), g.height());
}

QSizeF NotationMoveLink::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    Q_UNUSED(constraint)

    QSizeF size(0.0, 0.0);

    for (const auto *textItem: m_parts)
    {
        if (!textItem)
            continue;
        QFontMetrics fm(textItem->font());
        switch (which)
        {
        case Qt::MinimumSize:
        case Qt::PreferredSize:
        case Qt::MaximumSize:
        {
            const auto text = textItem->text();
            size.setWidth(fm.width(text) + size.width());
            size.setHeight(qMax(static_cast<qreal>(fm.height()), size.height()));
            break;
        }
        case Qt::MinimumDescent:
            size.setWidth(0);
            size.setHeight(qMax(static_cast<qreal>(fm.descent()), size.height()));
            break;
        default:
            break;
        }
    }
    return size;
}


GameNotationWidget::GameNotationWidget(QWidget* parent)
    : QWidget(parent)
    , m_browser(nullptr)
    , m_graphicsView(nullptr)
    , m_scene(nullptr)
    , m_output(nullptr)
{
    m_browser = new ChessBrowser(nullptr);
    m_graphicsView = new QGraphicsView();
    m_graphicsView->setAlignment(Qt::AlignTop);

    m_scene = new NotationScene(this);
    m_graphicsView->setScene(m_scene);

    // setup layout
    auto layout = new QHBoxLayout();
    layout->addWidget(m_browser);
    layout->addWidget(m_graphicsView);
    layout->setMargin(0);
    setLayout(layout);

    configureFont();

    connect(m_browser, &QTextBrowser::anchorClicked, this, &GameNotationWidget::anchorClicked);
    connect(m_browser, &ChessBrowser::actionRequested, this, &GameNotationWidget::actionRequested);
    connect(m_browser, &ChessBrowser::queryActiveGame, this, &GameNotationWidget::queryActiveGame);
    connect(m_browser, &ChessBrowser::signalMergeGame, this, &GameNotationWidget::signalMergeGame);

    connect(m_scene, &NotationScene::linkClicked, this, &GameNotationWidget::anchorClicked);
}

GameNotationWidget::~GameNotationWidget()
{
    delete m_output;
}

QString GameNotationWidget::getHtml() const
{
    return m_browser->toHtml();
}

QString GameNotationWidget::getText() const
{
    return m_browser->toPlainText();
}

QString GameNotationWidget::generateText(const GameX &game, bool trainingMode)
{
    return m_output->output(&game, trainingMode);
}

void GameNotationWidget::reload(const GameX& game, bool trainingMode)
{
    auto text = m_output->output(&game, trainingMode);
    m_browser->setText(text);
    m_browser->showMove(game.currentMove());

    m_scene->reload(game, m_graphicsView->size());
}

QMap<Nag, QAction*> GameNotationWidget::nagActions() const
{
    QMap<Nag, QAction*> result;
    const auto& actions = m_browser->m_actions;
    for (auto it = actions.cbegin(); it != actions.cend(); ++it)
    {
        auto action = it.key();
        const auto& e = it.value();
        if (e.type() == EditAction::AddNag)
        {
            Nag nag = static_cast<Nag>(e.data().toInt());
            result[nag] = action;
        }
    }
    return result;
}

void GameNotationWidget::saveConfig()
{
    AppSettings->setLayout(this);
}

void GameNotationWidget::slotReconfigure()
{
    AppSettings->layout(this);
    configureFont();

    delete m_output;
    m_output = new Output(Output::NotationWidget, &BoardView::renderImageForBoard);
}

void GameNotationWidget::showMove(int id)
{
    m_browser->showMove(id);
    auto *item = m_scene->selectMove(id);
    if (item)
    {
        m_graphicsView->ensureVisible(item);
    }
}

void GameNotationWidget::configureFont()
{
    QFont f = qApp->font();
    qreal r = AppSettings->getValue("/GameText/FontSize").toInt();
    f.setPointSize(r);
    QString fontFamily = AppSettings->getValue("/GameText/FontBrowserText").toString();
    if (!fontFamily.isEmpty())
    {
        f.setFamily(fontFamily);
    }
    setFont(f);
}
