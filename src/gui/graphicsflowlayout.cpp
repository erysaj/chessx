#include "graphicsflowlayout.h"

GraphicsFlowLayout::GraphicsFlowLayout(QGraphicsLayoutItem *parent, qreal hSpacing, qreal vSpacing)
    : QGraphicsLayout(parent), m_hSpace(hSpacing), m_vSpace(vSpacing)
{
    setContentsMargins(0.0, 0.0, 0.0, 0.0);

    QSizePolicy policy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    policy.setHeightForWidth(true);

    setSizePolicy(policy);
}

GraphicsFlowLayout::GraphicsFlowLayout(qreal hSpacing, qreal vSpacing)
    : GraphicsFlowLayout(nullptr, hSpacing, vSpacing)
{
}

GraphicsFlowLayout::~GraphicsFlowLayout()
{
    for (auto *item: qAsConst(m_items))
    {
        item->setParentLayoutItem(nullptr);
        if (item->ownedByLayout())
            delete item;
    }
}

int GraphicsFlowLayout::count() const
{
    return m_items.size();
}

QGraphicsLayoutItem *GraphicsFlowLayout::itemAt(int index) const
{
    return m_items.value(index);
}

void GraphicsFlowLayout::removeAt(int index)
{
    if (index >= 0 && index < m_items.size())
    {
        m_items.takeAt(index);
        invalidate();
    }
}

void GraphicsFlowLayout::addItem(QGraphicsLayoutItem *item)
{
    if (!item)
    {
        qWarning("GraphicsFlowLayout::addItem: cannot insert null item");
        return;
    }

    if (item == this)
    {
        qWarning("GraphicsFlowLayout::addItem: cannot insert itself");
        return;
    }

    m_items.append(item);
    addChildLayoutItem(item);
    invalidate();
}

qreal GraphicsFlowLayout::horizontalSpacing() const
{
    return qMax(m_hSpace, 0.0);
}

qreal GraphicsFlowLayout::verticalSpacing() const
{
    return qMax(m_vSpace, 0.0);
}

QSizeF GraphicsFlowLayout::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    QRectF rect({}, constraint);

    switch (which)
    {
    case Qt::MinimumSize:
    case Qt::MaximumSize:
    case Qt::PreferredSize:
        return { constraint.width(), performLayout(rect, true)};
    default:
        return {};
    }
}

qreal GraphicsFlowLayout::performLayout(const QRectF &rect, bool dryRun) const
{
    qreal ml, mt, mr, mb;
    getContentsMargins(&ml, &mt, &mr, &mb);

    auto isWidthConstrained = rect.width() >= 0;

    auto effectiveRect = rect.adjusted(+ml, +mt, -mr, -mb);
    auto x = effectiveRect.x();
    auto y = effectiveRect.y();
    qreal lineHeight = 0;

    QSizeF noConstraint;

    auto spaceX = horizontalSpacing();
    auto spaceY = verticalSpacing();

    for (auto *item : m_items)
    {
        auto itemSize = item->effectiveSizeHint(Qt::PreferredSize, noConstraint);
        auto nextX = x + itemSize.width() + spaceX;
        if (nextX - spaceX > effectiveRect.right() && lineHeight > 0 && isWidthConstrained)
        {
            x = effectiveRect.x();
            y = y + lineHeight + spaceY;
            nextX = x + itemSize.width() + spaceX;
            lineHeight = 0;
        }

        if (!dryRun)
        {
            QRectF itemRect(x, y, itemSize.width(), itemSize.height());
            item->setGeometry(itemRect);
        }

        x = nextX;
        lineHeight = qMax(lineHeight, itemSize.height());
    }
    return y + lineHeight - rect.y() + mb;
}

void GraphicsFlowLayout::setGeometry(const QRectF &rect)
{
    QGraphicsLayout::setGeometry(rect);
    performLayout(rect, false);
}
