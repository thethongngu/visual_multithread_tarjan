#include "view.h"

#include <QPainter>
#include <QPen>
#include <cmath>

// ----- NODEVIEW ----- //
NodeView::NodeView() {

}

NodeView::NodeView(int id, int x, int y) {
    this->id = id;
    this->x = x;
    this->y = y;
    this->index = "-1";
    this->lowlink = "-1";
    this->search = "-1";
    this->color = Qt::black;
    inSCC = "-1";
}

void NodeView::updateState(QColor color, QString index, QString lowlink, QString searchId, View* vi, QString scc) {

    this->color = color;
    if (index != "-1") this->index = index;
    if (lowlink!= "-1") this->lowlink = lowlink;
    if (searchId != "-1") this->search = searchId;
    if (color == Qt::magenta) {
        inSCC = scc;
    }

    // update arrow
    vi->startArrow.x = this->x - 50;   vi->startArrow.y = this->y - 50;
    vi->endArrow.x = this->x - 35;     vi->endArrow.y = this->y - 35;
}

// ----- EDGEVIEW ----- //
EdgeView::EdgeView(int idNode1, int idNode2) {
    this->idNode1 = idNode1;
    this->idNode2 = idNode2;
}

View::View(QWidget *parent) : QWidget(parent)
{
    timeSleep = 500;
}

void View::reset() {
    nodeList.clear();
    edgeList.clear();
    startArrow.x = 0;  startArrow.y = 0;
    endArrow.x = 0;    endArrow.y = 0;
    repaint();
}

void View::addNodeToGraph(NodeView newNode) {
    nodeList.push_back(newNode);
    maybeID = -1;
}

void View::addEdgeToGraph(EdgeView newEdge) {
    edgeList.push_back(newEdge);
}

void View::cachePressed(int _maybeID) {
    maybeID = _maybeID;
}

int View::getCachePressed() {
    return maybeID;
}

int View::isPressedNode(int x, int y) {
    if (x < 0 || x > 900 || y < 0 || y > 700) return false;

    for(size_t i = 0; i < nodeList.size(); i++) {
        NodeView tmpNode = nodeList[i];

        if (abs(tmpNode.x - x) <= 30 && abs(tmpNode.y - y) <= 30) return tmpNode.id;
        if (abs(tmpNode.x - x) <= 80 && abs(tmpNode.y - y) <= 80) return -2;
    }

    return -1;
}

void View::paintEvent(QPaintEvent *ev) {

    // setup painter
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255,255,255));
    painter.drawRect(0,0,1265,851);

    drawEdge(painter);
    drawDirectedEdge(painter);
    drawVertex(painter);
    if (nodeList.size() > 0) drawArrow(painter);
}

void View::drawArrow(QPainter& painter) {
    QPen pen(Qt::magenta, 5, Qt::SolidLine);
    painter.setPen(pen);

    NodeView u(-1, startArrow.x, startArrow.y);
    NodeView v(-1, endArrow.x, endArrow.y);
    painter.drawLine(u.x, u.y, v.x, v.y);

    int barb = 15;
    int r = 0;
    double phi = 3.141592653589793238462643383279502884/6;
    double theta = atan2(v.y - u.y, v.x - u.x);
    double xx = v.x - r * cos(theta);
    double yy = v.y - r * sin(theta);
    double x = xx - barb * cos(theta + phi);
    double y = yy - barb * sin(theta + phi);
    painter.drawLine(xx, yy, x, y);
    x = xx - barb * cos(theta - phi);
    y = yy - barb * sin(theta - phi);
    painter.drawLine(xx, yy, x, y);
}

void View::drawVertex(QPainter& painter) {   // redraw all vertices

    for(int i = 0; i < nodeList.size(); i++) {
        QPen penData(nodeList[i].color, 2, Qt::SolidLine);
        QPen penId(nodeList[i].color, 5, Qt::SolidLine);
        painter.setPen(penData);

//        painter.drawEllipse(QPoint(nodeList[i].x, nodeList[i].y), 20, 20);
        painter.drawRect(nodeList[i].x - 30, nodeList[i].y - 30, 60, 60);
        painter.drawRect(nodeList[i].x - 30, nodeList[i].y - 30, 20, 20);
        painter.drawRect(nodeList[i].x - 10, nodeList[i].y - 30, 20, 20);
        painter.drawRect(nodeList[i].x + 10, nodeList[i].y - 30, 20, 20);
        painter.drawRect(nodeList[i].x - 30, nodeList[i].y - 10, 30, 40);

//        painter.setFont(QFont("Courier New", 10));
        if (i < 10) {
            painter.drawText(nodeList[i].x + 10, nodeList[i].y + 10, QString("%1").arg(i));
        }
        else {
            painter.drawText(nodeList[i].x + 7, nodeList[i].y + 10, QString("%1").arg(i));
        }

//        painter.setFont(QFont("Courier New", 10));
        painter.drawText(nodeList[i].x - 27, nodeList[i].y - 15, QString("%1").arg(nodeList[i].index));
        painter.drawText(nodeList[i].x - 05, nodeList[i].y - 15, QString("%1").arg(nodeList[i].lowlink));
        painter.drawText(nodeList[i].x + 12, nodeList[i].y - 15, nodeList[i].search);
        painter.drawText(nodeList[i].x - 20, nodeList[i].y + 10, QString("%1").arg(nodeList[i].inSCC));
    }
}

void View::drawEdge(QPainter &painter) {

    for(int i = 0; i < edgeList.size(); i++) {
        EdgeView tmpEdge = edgeList[i];
        QPen pen(tmpEdge.color, 1, Qt::SolidLine);
        painter.setPen(pen);

        NodeView u = nodeList[tmpEdge.idNode1];
        NodeView v = nodeList[tmpEdge.idNode2];

        painter.drawLine(u.x, u.y, v.x, v.y);    // u -> v is one edge
    }
}

void View::drawDirectedEdge(QPainter &painter) {

    int barb = 15;
    int r = 30;
    double phi = 3.141592653589793238462643383279502884/6;

    for(int i = 0; i < edgeList.size(); i++) {
        EdgeView tmpEdge = edgeList[i];
        NodeView u = nodeList[tmpEdge.idNode1];
        NodeView v = nodeList[tmpEdge.idNode2];

        QPen pen(edgeList[i].color, 1, Qt::SolidLine);
        painter.setPen(pen);

        double theta = atan2(v.y - u.y, v.x - u.x);
        double xx = v.x - r * cos(theta);
        double yy = v.y - r * sin(theta);
        double x = xx - barb * cos(theta + phi);
        double y = yy - barb * sin(theta + phi);
        painter.drawLine(xx, yy, x, y);
        x = xx - barb * cos(theta - phi);
        y = yy - barb * sin(theta - phi);
        painter.drawLine(xx, yy, x, y);
    }
}

void View::resetPanel() {
    for(int i = 0; i < nodeList.size(); i++) {
        nodeList[i].index = "-1";
        nodeList[i].lowlink = "-1";
        nodeList[i].search = "-1";
        nodeList[i].color = Qt::black;
        nodeList[i].inSCC = "-1";
    }
}

QList< QList<int> > View::exportGraph() {
    QList< QList<int> > res;
    for(int i = 0; i < nodeList.size(); i++) {
        QList<int> tmp;
        for(int j = 0; j < nodeList[i].succs.size(); j++) {
            tmp.append(nodeList[i].succs[j]);
        }
        res.append(tmp);
    }
    return res;
}

