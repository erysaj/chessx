#ifndef GRAPHICSFLOWLAYOUT_H
#define GRAPHICSFLOWLAYOUT_H

#include <QGraphicsLayout>


class GraphicsFlowLayout : public QGraphicsLayout
{
public:
    explicit GraphicsFlowLayout(QGraphicsLayoutItem *parent, qreal hSpacing = -1, qreal vSpacing = -1);
    explicit GraphicsFlowLayout(qreal hSpacing = -1, qreal vSpacing = -1);
    ~GraphicsFlowLayout();

    int count() const override;
    QGraphicsLayoutItem *itemAt(int index) const override;
    void removeAt(int index) override;

    void addItem(QGraphicsLayoutItem *item);

    qreal horizontalSpacing() const;
    qreal verticalSpacing() const;

    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const override;

    void setGeometry(const QRectF &rect) override;

private:
    qreal performLayout(const QRectF &rect, bool dryRun) const;

    QList<QGraphicsLayoutItem *> m_items;
    qreal m_hSpace;
    qreal m_vSpace;
};


#endif // GRAPHICSFLOWLAYOUT_H
