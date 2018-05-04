#ifndef VIEW_H
#define VIEW_H

#include <QWidget>
#include <QPainter>
#include <QObject>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QTest>
#include <QList>

class View;

class NodeView {
public:
    NodeView();

    int id, x, y;
    QString index, lowlink, search, inSCC;
    QColor color;
    QList<int> succs;
    NodeView(int id, int x, int y);
    void updateState(QColor color, QString index, QString lowlink, QString searchId, View* vi, QString scc);
};

class EdgeView {
public:
    int idNode1, idNode2;
    QColor color;
    EdgeView(int idNode1, int idNode2);
    void udpateColorState();
    void transfer(int id1, int id2);
};

class CodeView {
public:
    void updateColorLine();
};

class View : public QWidget
{
    Q_OBJECT
public:
    QPainter painter;
    explicit View(QWidget *parent = 0);

    void paintEvent(QPaintEvent *ev);

    /** attributes **/
    int maybeID;
    int timeSleep;

    /** methods **/
    int isPressedNode(int x, int y);
    void addNodeToGraph(NodeView newNode);
    void addEdgeToGraph(EdgeView newEdge);
    void cachePressed(int _maybeID);
    int getCachePressed();
    void startTarjan(QString filename);

    void reset();
    void drawVertex(QPainter& painter);
    void drawEdge(QPainter& painter);
    void drawDirectedEdge(QPainter& painter);
    void drawArrow(QPainter& painter);
    void resetPanel();

    QList< QList<int> > exportGraph();

    QList<NodeView> nodeList;
    QList<EdgeView> edgeList;
    NodeView startArrow, endArrow;

    QColor myColor[3] = {Qt::red, Qt::green, Qt::blue};
signals:

public slots:
};

#endif // VIEW_H
