#include "pgnwidget.h"

#include "settings.h"

//("/GameText/SymbolicNag", true);
//("/GameText/VariationIndentLevel", 1);
//("/GameText/VariationIndentSize", 3);
//("/GameText/CommentIndent", "OnlyMainline");
//("/GameText/HeaderColor", "blue");
//("/GameText/ShowHeader", false);

//static QUrl makeCommentUrl(MoveId moveId, GameX::Position position)
//{
//    QUrl url(QString("cmt:%1").arg(moveId));
//    if (position == GameX::BeforeMove)
//    {
//        url.setScheme("precmt");
//    }
//    return url;
//}

//static QUrl makeMoveUrl(MoveId moveId)
//{
//    return { QString("move:%1").arg(moveId) };
//}


//NotationOptions::NotationOptions()
//    : m_moveFont(AppSettings->value("/GameText/FontBrowserMove").toString(), AppSettings->value("GameText/FontSize").toInt(), QFont::Normal)
//    , m_mainFont(m_moveFont)
//    , m_textFont(AppSettings->value("/GameText/FontBrowserText").toString(), AppSettings->value("/GameText/FontSize").toInt(), QFont::Normal)
//    , m_mainlineColor(AppSettings->value("/GameText/MainLineMoveColor").value<QColor>())
//    , m_variationColor(AppSettings->value("/GameText/VariationColor").value<QColor>())
//    , m_commentColor(AppSettings->value("/GameText/CommentColor").value<QColor>())
//    , m_nagsColor(AppSettings->value("/GameText/NagColor").value<QColor>())
//    , m_showDiagrams(AppSettings->value("/GameText/ShowDiagrams").toBool())
//    , m_diagramSize()
//    , m_inlineDepthThreshold(AppSettings->value("/GameText/VariationIndentLevel").toInt())
//{
//    auto diagramSize = AppSettings->value("/GameText/DiagramSize").toInt();
//    m_diagramSize.setWidth(diagramSize);
//    m_diagramSize.setHeight(diagramSize);

//    m_mainFont.setBold(true);
//}

//AnnotatedVariation::VisitingOptions NotationOptions::visitingOptions() const
//{
//    unsigned options = 0;
//    if (showDiagrams())
//    {
//        options |= AnnotatedVariation::Visit_Diagrams;
//    }
//    return static_cast<AnnotatedVariation::VisitingOptions>(options);
//}

//NotationTextLink *VariationContainerWidget::createTextItem(const QString &text)
//{
//    auto *item = new NotationTextLink();
//    item->setText(text);
//    item->setFont(options().textFont());
//    item->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed, QSizePolicy::Label);
//    return item;
//}

//void BlockVariationContainer::addMove(MoveIterator &begin, const MoveIterator &end)
//{
//    const auto &board = begin.srcBoard();
//    const QString fmt = board.toMove() == White? "%1.": "%1…";
//    const QString moveCounter = fmt.arg(board.moveNumber());

//    if (begin.srcBoard().toMove() == White)
//    {
//        // add white move item
//        addItem(makeMoveLink(moveCounter, begin));
//        ++begin;

//        if (begin != end)
//        {
//            // we have response in the current sequence -> add black move item
//            addItem(makeMoveLink("", begin));
//            ++begin;
//        }
//    }
//    else if (begin.srcBoard().toMove() == Black)
//    {
//        addItem(makeMoveLink(moveCounter, begin));
//        ++begin;
//    }
//}

//void BlockVariationContainer::visitComment(const QString &text, MoveId moveId, GameX::Position position)
//{
//    auto href = makeCommentUrl(moveId, position);
//    if (m_currDepth == 0)
//    {
//        flush();
//    }

//    const auto chunks = cutText(text);
//    for (const auto &chunk: chunks)
//    {
//        auto *item = createTextItem(chunk);
//        item->setColor(options().commentColor());
//        item->setHref(href);
//        connect(item, &NotationLink::linkClicked, this, &VariationContainerWidget::linkClicked);
//        addItem(item);
//    }

//    if (m_currDepth == 0)
//    {
//        flush();
//    }
//}

//void BlockVariationContainer::visitMoves(const MoveIterator &first, const MoveIterator &last)
//{
//    for (auto moveIt = first; moveIt != last; )
//    {
//        addMove(moveIt, last);
//    }
//}

//void BlockVariationContainer::visitDiagram(const BoardX &board)
//{
//    flush();

//    auto *layout = new QGraphicsLinearLayout();
//    layout->setOrientation(Qt::Horizontal);
//    layout->setContentsMargins(0, 0, 0, 0);

//    auto *widget = new QGraphicsWidget();
//    widget->setLayout(layout);

//    auto *diagram = new DiagramItem(board, options().diagramSize());
//    layout->addItem(diagram);
//    layout->addStretch();

//    m_layout->addItem(widget);
//}

//void BlockVariationContainer::visitVariation(const AnnotatedVariation &variation)
//{
//    m_currDepth += 1;

//    if (m_currDepth < options().inlineDepthThreshold())
//    {
//        flush();
//        addItem(createTextItem("["));
//        variation.visit(*this, options().visitingOptions());
//        addItem(createTextItem("]"));
//        flush();
//    }
//    else
//    {
//        addItem(createTextItem("("));
//        variation.visit(*this, options().visitingOptions());
//        addItem(createTextItem(")"));
//    }

//    m_currDepth -= 1;
//}

//NotationLink *BlockVariationContainer::makeMoveLink(const QString &moveCounter, const MoveIterator &moveIt)
//{
//    auto moveId = moveIt.dstMoveId();
//    const auto moveText = moveIt.srcBoard().moveToSan(moveIt.move());
//    const auto nagText = "";

//    auto *link = new NotationMoveLink(moveCounter, moveText, nagText);
//    link->setHref(makeMoveUrl(moveId));
////    if (m_currDepth == 0)
////    {
////        // main line
////        link->setFont(options().mainFont());
////        link->setColor(options().mainColor());
////    }
////    else
////    {
////        link->setFont(options().moveFont());
////        link->setColor(options().moveColor());
////    }
//    link->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed, QSizePolicy::Label);
//    link->setAcceptHoverEvents(true);
//    link->setAcceptedMouseButtons(Qt::LeftButton);
//    m_links[moveId] = link;
//    connect(link, &NotationLink::linkClicked, this, &VariationContainerWidget::linkClicked);
//    return link;
//}

//void TableVariationContainer::visitComment(const QString &text, MoveId moveId, GameX::Position position)
//{
//    auto href = makeCommentUrl(moveId, position);
//    auto *layout = new GraphicsFlowLayout(0.0, 0.0);

//    const auto chunks = cutText(text);
//    for (const auto &chunk: chunks)
//    {
//        auto *item = createTextItem(chunk);
//        item->setColor(options().commentColor());
//        item->setHref(href);
//        connect(item, &NotationLink::linkClicked, this, &VariationContainerWidget::linkClicked);
//        layout->addItem(item);
//    }

//    auto *widget = new QGraphicsWidget();
//    widget->setLayout(layout);

//    m_layout->addItem(widget, m_layout->rowCount(), 0, 1, 4);
//}

//void TableVariationContainer::visitMoves(const MoveIterator &first, const MoveIterator &last)
//{
//    auto row = m_layout->rowCount();
//    for (auto moveIt = first; moveIt != last; ++row)
//    {
//        auto *moveIndicator = createGridMoveIndicatorItem(moveIt.moveNumber());
//        m_layout->addItem(moveIndicator, row, Column_MoveNumber);

//        if (moveIt.srcBoard().toMove() == White)
//        {
//            // add white move item
//            auto *whiteLink = makeMoveLink(moveIt);
//            m_layout->addItem(whiteLink, row, Column_White);
//            ++moveIt;

//            if (moveIt != last)
//            {
//                // we have response in the current sequence -> add black move item
//                auto *blackLink = makeMoveLink(moveIt);
//                m_layout->addItem(blackLink, row, Column_Black);
//                ++moveIt;
//            }
//            else if (moveIt)
//            {
//                // current sequence ended, but the line continues
//                auto *blackItem = createGridContinuationItem();
//                m_layout->addItem(blackItem, row, Column_Black);
//            }
//            else
//            {
//                // line ended
//                auto *blackItem = createGridEmptyItem();
//                m_layout->addItem(blackItem, row, Column_Black);
//            }
//        }
//        else if (moveIt.srcBoard().toMove() == Black)
//        {
//            // this should happen at most once, since we usually advance by 2
//            auto *whiteItem = createGridContinuationItem();
//            m_layout->addItem(whiteItem, row, Column_White);

//            auto *blackLink = makeMoveLink(moveIt);
//            m_layout->addItem(blackLink, row, Column_Black);
//            ++moveIt;
//        }
//    }
//}

//void TableVariationContainer::visitDiagram(const BoardX &board)
//{
//    auto *item = createDiagram(board);
//    m_layout->addItem(item, m_layout->rowCount(), Column_White, 1, 2);
//}

//void TableVariationContainer::visitVariation(const AnnotatedVariation &variation)
//{
//    auto *widget = new BlockVariationContainer(baseDepth() + 1);
//    widget->setVariation(variation);
//    connect(widget, &VariationContainerWidget::linkClicked, this, &VariationContainerWidget::linkClicked);
//    m_layout->addItem(widget, m_layout->rowCount(), 0, 1, 4);
//}

//QGraphicsLayoutItem *TableVariationContainer::createGridMoveIndicatorItem(int moveNumber)
//{
//    auto *item = new NotationCell();
//    item->setPreferredSize(100, 25);
//    item->setText(QString::number(moveNumber));
//    return item;
//}

//QGraphicsLayoutItem *TableVariationContainer::createGridContinuationItem()
//{
//    auto *item = new NotationCell();
//    item->setPreferredSize(100, 25);
//    item->setText("...");
//    return item;
//}

//QGraphicsLayoutItem *TableVariationContainer::createGridEmptyItem()
//{
//    auto *item = new NotationCell();
//    item->setPreferredSize(100, 25);
//    item->setText("");
//    return item;
//}

//NotationLink *TableVariationContainer::createGridMoveLink(const MoveIterator &moveIt)
//{
//    auto moveId = moveIt.dstMoveId();

//    auto *link = new NotationCell();
//    link->setPreferredSize(100, 25);
//    link->setHref(makeMoveUrl(moveId));
//    link->setText(moveIt.srcBoard().moveToSan(moveIt.move()));
//    return link;
//}

//QGraphicsLayoutItem *TableVariationContainer::createDiagram(const BoardX &board)
//{
//    return new DiagramItem(board, options().diagramSize());
//}

//NotationLink *TableVariationContainer::makeMoveLink(const MoveIterator &moveIt)
//{
//    auto moveId = moveIt.dstMoveId();
//    auto *link = createGridMoveLink(moveIt);
//    link->setAcceptHoverEvents(true);
//    link->setAcceptedMouseButtons(Qt::LeftButton);
//    m_links[moveId] = link;
//    return link;
//}

//void NotationScene::reload(const GameX &game, const QSize &viewSize)
//{
//    clear();
//    m_moveLinks.clear();
//    selectMove(NO_MOVE);

//    NotationOptions options;

//    AnnotatedVariation mainLine(game);

//    VariationContainerWidget *rootWdgt = nullptr;
//    if (AppSettings->value("/GameText/ColumnStyle").toBool())
//    {
//        rootWdgt = new TableVariationContainer(0);
//    }
//    else
//    {
//        rootWdgt = new BlockVariationContainer(0);
//    }
//    rootWdgt->setVariation(mainLine);
//    m_moveLinks = rootWdgt->links();

//    connect(rootWdgt, &VariationContainerWidget::linkClicked, this, &NotationScene::linkClicked);

//    addItem(rootWdgt);

//    const qreal W = viewSize.width();
//    const qreal H = rootWdgt->effectiveSizeHint(Qt::PreferredSize, { W, -1 }).height();
//    setSceneRect(0, 0, W, H);

//    rootWdgt->setGeometry(0, 0, W, H);

//    selectMove(game.currentMove());
//}

//NotationCell::NotationCell(QGraphicsItem *parent)
//    : NotationLink(parent)
//{
//    QPen bgPen(QColor("black"), 0, Qt::NoPen);
//    m_backgroundItem = new QGraphicsRectItem(this);
//    m_backgroundItem->setPen(bgPen);

//    m_textItem = new QGraphicsSimpleTextItem(this);

//    updateBackground();
//}

//void NotationCell::setGeometry(const QRectF &rect)
//{
//    QGraphicsWidget::setGeometry(rect);

//    const auto g = geometry();
//    m_backgroundItem->setRect(0, 0, g.width(), g.height());
//}

//void NotationCell::handleStateChanged(StateFlags changeMask)
//{
//    SUPER::handleStateChanged(changeMask);
//    if (changeMask & (State_Selected | State_Highlighted))
//    {
//        updateBackground();
//    }
//}

//void NotationCell::updateBackground()
//{
//    if (hasState(State_Selected))
//    {
//        m_backgroundItem->setBrush(QColor("darkblue"));
//    }
//    else if (hasState(State_Highlighted))
//    {
//        m_backgroundItem->setBrush(QColor("blue"));
//    }
//    else
//    {
//        m_backgroundItem->setBrush(QColor("lightgray"));
//    }
//}


//NotationTextLink::NotationTextLink(QGraphicsItem *parent)
//    : NotationLink(parent)
//{
//    QPen bgPen(QColor("black"), 0, Qt::NoPen);
//    m_backgroundItem = new QGraphicsRectItem(this);
//    m_backgroundItem->setPen(bgPen);

//    m_textItem = new QGraphicsSimpleTextItem(this);

//    updateBackground();
//}

//void NotationTextLink::setGeometry(const QRectF &rect)
//{
//    NotationLink::setGeometry(rect);

//    const auto g = geometry();
//    m_backgroundItem->setRect(0, 0, g.width(), g.height());
//}

//QSizeF NotationTextLink::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
//{
//    Q_UNUSED(constraint)

//    QSizeF size;
//    QFontMetrics fm(m_textItem->font());

//    switch (which)
//    {
//    case Qt::MinimumSize:
//    case Qt::PreferredSize:
//    case Qt::MaximumSize:
//    {
//        const auto text = m_textItem->text();
//        size.setWidth(fm.width(text));
//        size.setHeight(fm.height());
//        break;
//    }
//    case Qt::MinimumDescent:
//        size.setWidth(0);
//        size.setHeight(fm.descent());
//        break;
//    default:
//        break;
//    }
//    return size;
//}

//void NotationTextLink::handleStateChanged(StateFlags changeMask)
//{
//    SUPER::handleStateChanged(changeMask);
//    if (changeMask & (State_Selected | State_Highlighted))
//    {
//        updateBackground();
//    }
//}

//void NotationTextLink::updateBackground()
//{
//    if (hasState(State_Selected))
//    {
//        m_backgroundItem->setBrush(QColor("darkblue"));
//    }
//    else if (hasState(State_Highlighted))
//    {
//        m_backgroundItem->setBrush(QColor("blue"));
//    }
//    else
//    {
//        m_backgroundItem->setBrush(QColor("lightgray"));
//    }
//}

PgnWidget::PgnWidget(QWidget* parent)
    : QWidget(parent)
    , m_quickView(nullptr)
{
    m_quickView = new QQuickWidget();
    m_quickView->setSource(QUrl(QStringLiteral("qrc:/qml/notation.qml")));

    // setup layout
    auto layout = new QHBoxLayout();
    layout->addWidget(m_quickView);
    layout->setMargin(0);
    setLayout(layout);
}

PgnWidget::~PgnWidget() = default;

void PgnWidget::reload(const GameX& game, bool trainingMode) {
    Q_UNUSED(trainingMode);
}

void PgnWidget::showMove(int id) {
    Q_UNUSED(id);
}
