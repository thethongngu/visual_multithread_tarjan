#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    // setup UI
    ui->setupUi(this);
    ui->pauseButton->setEnabled(false);
    setMouseTracking(true);
    setWindowTitle("Concurrent Tarjan Algorithm Visualization | thethong.ngu");

    QList<QString> codeSample;
    codeSample.append("addNode(startNode) status = inProgress");                        // 01_%node_%index_%lowlink_%search.
    codeSample.append("while(!controlStack.isEmpty()):");                               // 02_%search_
    codeSample.append("    parent = controlStack.pop()");                               // 03_%node_%search_
    codeSample.append("    if (parent has child to explore): ");                        // 04_%node_%search_
    codeSample.append("        if (child == UNSEEN): ");                                // 05_%node_%search_
    codeSample.append("            addNode(child)");                                    // 06_%node_%index_%lowlink_%search_
    codeSample.append("        elif (child in stack of this search): ");                // 07_%node_%search_
    codeSample.append("            parent.updateLowlink(child.index)");                 // 08_%parent_%child_%parentIndex_%parentLowlink_%search_
    codeSample.append("        elif (child in stack of other search):");                // 09_%node_%search_
    codeSample.append("            updateInfomation() suspendSearch()");                // 10_%node_%search_
    codeSample.append("            if (blockingCycle()): solveDeadlock()");             // 11_%node_%index_%lowlink_%search_
    codeSample.append("    else:");                                                     // 12_%node_%search_
    codeSample.append("        controlStack.pop();");                                   // 13_%node_%search_
    codeSample.append("        if (!controlStack.isEmpty()):");                         // 14_%search_
    codeSample.append("            controlStack.top.updateLowlink(parent.lowlink)");    // 15_%parent_%child_%parentIndex_%parentLowlink_%search_
    codeSample.append("        if (parent.index == parent.lowlink:");                   // 16_%node_%search_
    codeSample.append("            SCC_found() updateInformation()");                   // 17_%node_%search_
    codeSample.append("searchComplete()");                                              // 18_%search_

    ui->searchWidget->setRowCount(18);
    ui->searchWidget->setColumnCount(1);
    ui->searchWidget->horizontalHeader()->hide();
    ui->searchWidget->setShowGrid(false);
    ui->searchWidget->verticalHeader()->setFixedWidth(20);
    ui->searchWidget->setColumnWidth(0, 500);
    ui->searchWidget->setFont(QFont("Courier New", 11));

    ui->statusWidget->setRowCount(1);
    ui->statusWidget->setColumnCount(1);
    ui->statusWidget->horizontalHeader()->hide();
    ui->statusWidget->verticalHeader()->hide();
    ui->statusWidget->setRowHeight(0, 50);
    ui->statusWidget->setShowGrid(false);
    ui->statusWidget->setColumnWidth(0, 331);
    ui->statusWidget->setFont(QFont("Courier New", 13));
    QTableWidgetItem *newItem1 = new QTableWidgetItem();
    ui->statusWidget->setItem(0, 0, newItem1);

    for(int i = 0; i < codeSample.size(); i++) {
        QTableWidgetItem *newItem1 = new QTableWidgetItem(codeSample[i]);
        ui->searchWidget->setRowHeight(i, 20);
        ui->searchWidget->setItem(i, 0, newItem1);
    }

    QString filename = "log001.txt";
    QFile file(filename);
    if (!file.exists()) {
        file.open(QIODevice::ReadWrite | QIODevice::Text);
        file.close();
    }
}
MainWindow::~MainWindow()
{
    delete ui;
}

/** GUI functions */
void MainWindow::mousePressEvent(QMouseEvent *event) {
    int x = event->x();
    int y = event->y();

    int maybeID = ui->drawPanel->isPressedNode(x, y);
    if (maybeID == -1) {  // not a node -> add new node

        NodeView newNode(ui->drawPanel->nodeList.size(), x, y);
        ui->drawPanel->addNodeToGraph(newNode);
    }
    if (maybeID >= 0) {   // pressed other node -> maybe new edge
        ui->drawPanel->cachePressed(maybeID);
    }

    ui->drawPanel->repaint();
}
void MainWindow::mouseReleaseEvent(QMouseEvent *event) {
    int x = event->x();
    int y = event->y();

    int endNodeID = ui->drawPanel->isPressedNode(x, y);
    int startNodeID = ui->drawPanel->getCachePressed();

    if (endNodeID >= 0 && startNodeID >= 0 && endNodeID != startNodeID) {  // no loop edge
        ui->drawPanel->nodeList[startNodeID].succs.push_back(endNodeID);  // and child

        bool found = false;
        for(int i = 0; i < ui->drawPanel->edgeList.size(); i++) {
            if (ui->drawPanel->edgeList[i].idNode1 == startNodeID && ui->drawPanel->edgeList[i].idNode2 == endNodeID) {
                found = true;
                break;
            }
        }

        if (!found) {
            EdgeView newEdgeView(startNodeID, endNodeID);
            ui->drawPanel->addEdgeToGraph(newEdgeView);
        }
    }
    ui->drawPanel->repaint();

}

QString getLog(QString log, int index) {
    int start = 2;
    int end = log.indexOf('_', 3);
    for(int i = 0; i < index - 1; i++) {
        start = end;
        end = log.indexOf('_', start + 1);
    }
    qDebug() << log.mid(start + 1, end - start - 1);
    return log.mid(start + 1, end - start - 1);
}

void MainWindow::updateCodeColor(int linePos, QString logSearchid) {
    if (logSearchid[0] == '0') {
        if (oldLine0 >= 0) {
            if (oldLine1 != oldLine0) ui->searchWidget->item(oldLine0, 0)->setBackground(QBrush(Qt::white));
            else ui->searchWidget->item(oldLine0, 0)->setBackground(QBrush(QColor(191, 241, 255)));
        }
        if (oldLine1 == linePos - 1) ui->searchWidget->item(linePos - 1, 0)->setBackground(QBrush(Qt::green));
        else ui->searchWidget->item(linePos - 1, 0)->setBackground(QBrush(Qt::yellow));
        oldLine0 = linePos - 1;
    }
    else {
        if (oldLine1 >= 0) {
            if (oldLine0 != oldLine1) ui->searchWidget->item(oldLine1, 0)->setBackground(QBrush(Qt::white));
            else ui->searchWidget->item(oldLine1, 0)->setBackground(QBrush(Qt::yellow));
        }
        if (oldLine0 == linePos - 1) ui->searchWidget->item(linePos - 1, 0)->setBackground(QBrush(Qt::green));
        else ui->searchWidget->item(linePos - 1, 0)->setBackground(QBrush(QColor(191, 241, 255)));
        oldLine1 = linePos - 1;
    }
}

void MainWindow::visualizeTarjan(QString filename) {
    ui->replayButton->setEnabled(false);
    ui->drawPanel->setEnabled(false);
    ui->startButton->setEnabled(false);
    ui->resetButton->setEnabled(false);
    ui->pauseButton->setEnabled(true);


    QFile logFile(filename);
    QFileInfo check_file(filename);
    if (!(check_file.exists() && check_file.isFile())) {

    }


    logFile.open(QFile::ReadOnly | QFile::Truncate);

    QTextStream reader(&logFile);
    QString logNodeid, logNodeIndex, logNodeLowlink, logSearchid, logSCC;
    QString logParentid, logChildid, logParentLowlink, logChildIndex;
    QString logStatus;
    int ttime = 100;
    int cntLog = 0;

    while (!reader.atEnd()) {
        if (isPaused) break;

        QString log;
        while (cntLog <= oldLogLine) {
            log = reader.readLine();
            cntLog += 1;
        }
        oldLogLine = cntLog;

        // get command and data
        int linePos = log.mid(0, 2).toInt();
        ttime = ui->horizontalSlider->value() * 400;
        switch (linePos) {
            case 1:
                logNodeid = getLog(log, 1);  logNodeIndex = getLog(log, 2);  logNodeLowlink = getLog(log, 3);  logSearchid = getLog(log, 4);  logStatus = getLog(log, 5);
                ui->drawPanel->nodeList[logNodeid.toInt()].updateState(Qt::green, logNodeIndex, logNodeLowlink, logSearchid, ui->drawPanel, "0");
                updateCodeColor(linePos, logSearchid);
                ui->statusWidget->item(0, 0)->setText(logStatus);
                ui->drawPanel->repaint();
                QTest::qWait(ttime);
                break;
            case 2:
                logSearchid = getLog(log, 1);  logStatus = getLog(log, 2);
                updateCodeColor(linePos, logSearchid);
                ui->statusWidget->item(0, 0)->setText(logStatus);
                ui->drawPanel->repaint();
                QTest::qWait(ttime);
                break;
            case 3:
                logNodeid = getLog(log, 1);  logSearchid = getLog(log, 2);  logStatus = getLog(log, 3);
                updateCodeColor(linePos, logSearchid);
                ui->statusWidget->item(0, 0)->setText(logStatus);
                ui->drawPanel->repaint();
                QTest::qWait(ttime);
                break;
            case 4:
                logNodeid = getLog(log, 1);  logSearchid = getLog(log, 2);  logStatus = getLog(log, 3);
                updateCodeColor(linePos, logSearchid);
                ui->statusWidget->item(0, 0)->setText(logStatus);
                ui->drawPanel->repaint();
                QTest::qWait(ttime);
                break;
            case 5:
                logNodeid = getLog(log, 1);  logSearchid = getLog(log, 2);  logStatus = getLog(log, 3);
                updateCodeColor(linePos, logSearchid);
                ui->statusWidget->item(0, 0)->setText(logStatus);
                ui->drawPanel->repaint();
                QTest::qWait(ttime);
                break;
            case 6:
                logNodeid = getLog(log, 1);  logNodeIndex = getLog(log, 2);  logNodeLowlink = getLog(log, 3);  logSearchid = getLog(log, 4);  logStatus = getLog(log, 5);
                ui->drawPanel->nodeList[logNodeid.toInt()].updateState(Qt::green, logNodeIndex, logNodeLowlink, logSearchid, ui->drawPanel, "0");
                updateCodeColor(linePos, logSearchid);
                ui->statusWidget->item(0, 0)->setText(logStatus);
                ui->drawPanel->repaint();
                QTest::qWait(ttime);
                break;
            case 7:
                logNodeid = getLog(log, 1);  logSearchid = getLog(log, 2);  logStatus = getLog(log, 3);
                updateCodeColor(linePos, logSearchid);
                ui->statusWidget->item(0, 0)->setText(logStatus);
                ui->drawPanel->repaint();
                QTest::qWait(ttime);
                break;
            case 8:
                logParentid = getLog(log, 1);  logChildid = getLog(log, 2);  logParentLowlink = getLog(log, 3);  logChildIndex = getLog(log, 4);  logSearchid = getLog(log, 5);  logStatus = getLog(log, 6);
                ui->drawPanel->nodeList[logParentid.toInt()].updateState(Qt::green, "-1", logParentLowlink, logSearchid, ui->drawPanel, "0");
                updateCodeColor(linePos, logSearchid);
                ui->statusWidget->item(0, 0)->setText(logStatus);
                ui->drawPanel->repaint();
                QTest::qWait(ttime);
                break;
            case 9:
                logNodeid = getLog(log, 1);  logSearchid = getLog(log, 2);  logStatus = getLog(log, 3);
                updateCodeColor(linePos, logSearchid);
                ui->statusWidget->item(0, 0)->setText(logStatus);
                ui->drawPanel->repaint();
                QTest::qWait(ttime);
                break;
            case 10:
                logStatus = getLog(log, 1);
                updateCodeColor(linePos, logSearchid);
                ui->statusWidget->item(0, 0)->setText(logStatus);
                ui->drawPanel->repaint();
                QTest::qWait(ttime);
                break;
            case 11:
                logNodeid = getLog(log, 1);  logNodeIndex = getLog(log, 2);  logNodeLowlink = getLog(log, 3);  logSearchid = getLog(log, 4); logStatus = getLog(log, 5);
                ui->drawPanel->nodeList[logParentid.toInt()].updateState(Qt::cyan, logNodeIndex, logNodeLowlink, logSearchid, ui->drawPanel, "0");
                updateCodeColor(linePos, logSearchid);
                ui->statusWidget->item(0, 0)->setText(logStatus);
                ui->drawPanel->repaint();
                QTest::qWait(ttime);
                break;
            case 12:
                logNodeid = getLog(log, 1);  logSearchid = getLog(log, 2);  logStatus = getLog(log, 3);
                updateCodeColor(linePos, logSearchid);
                ui->statusWidget->item(0, 0)->setText(logStatus);
                ui->drawPanel->repaint();
                QTest::qWait(ttime);
                break;
            case 13:
                logNodeid = getLog(log, 1);  logSearchid = getLog(log, 2);  logStatus = getLog(log, 3);
                ui->drawPanel->nodeList[logNodeid.toInt()].updateState(Qt::red, "-1", "-1", logSearchid, ui->drawPanel, "0");
                updateCodeColor(linePos, logSearchid);
                ui->statusWidget->item(0, 0)->setText(logStatus);
                ui->drawPanel->repaint();
                QTest::qWait(ttime);
                break;
            case 14:
                logSearchid = getLog(log, 1);  logStatus = getLog(log, 2);
                updateCodeColor(linePos, logSearchid);
                ui->statusWidget->item(0, 0)->setText(logStatus);
                ui->drawPanel->repaint();
                QTest::qWait(ttime);
                break;
            case 15:
                logParentid = getLog(log, 1);  logChildid = getLog(log, 2);  logParentLowlink = getLog(log, 3);  logChildIndex = getLog(log, 4);  logSearchid = getLog(log, 5);  logStatus = getLog(log, 6);
                ui->drawPanel->nodeList[logParentid.toInt()].updateState(Qt::green, "-1", logChildIndex, logSearchid, ui->drawPanel, "0");
                updateCodeColor(linePos, logSearchid);
                ui->statusWidget->item(0, 0)->setText(logStatus);
                ui->drawPanel->repaint();
                QTest::qWait(ttime);
                break;
            case 16:
                logNodeid = getLog(log, 1);  logSearchid = getLog(log, 2);  logStatus = getLog(log, 3);
                ui->drawPanel->nodeList[logNodeid.toInt()].updateState(Qt::red, "-1", "-1", logSearchid, ui->drawPanel, "0");
                updateCodeColor(linePos, logSearchid);
                ui->statusWidget->item(0, 0)->setText(logStatus);
                ui->drawPanel->repaint();
                QTest::qWait(ttime);
                break;
            case 17:
                logNodeid = getLog(log, 1);  logSCC = getLog(log, 2);  logSearchid = getLog(log, 3);  logStatus = getLog(log, 4);
                ui->drawPanel->nodeList[logNodeid.toInt()].updateState(Qt::magenta, "-1", "-1", logSearchid, ui->drawPanel, logSCC);
                updateCodeColor(linePos, logSearchid);
                ui->statusWidget->item(0, 0)->setText(logStatus);
                ui->drawPanel->repaint();
                QTest::qWait(ttime);
                break;
            case 18:
                logSearchid = getLog(log, 1);  logStatus = getLog(log, 2);
                updateCodeColor(linePos, logSearchid);
                ui->statusWidget->item(0, 0)->setText(logStatus);
                ui->drawPanel->repaint();
                QTest::qWait(ttime);
                break;
            case 21:
                logNodeid = getLog(log, 1);  logSCC = getLog(log, 2);  logStatus = getLog(log, 3);
                ui->drawPanel->nodeList[logNodeid.toInt()].updateState(Qt::magenta, "-1", "-1", "-1", ui->drawPanel, logSCC);
                ui->statusWidget->item(0, 0)->setText(logStatus);
                ui->drawPanel->repaint();
                QTest::qWait(ttime);
                break;
        }
    }

    ui->searchWidget->item(17, 0)->setBackground(QBrush(Qt::white));
    logFile.close();

    if (isPaused) {
        ui->pauseButton->setEnabled(true);
    }
    else {
        ui->drawPanel->setEnabled(true);
        ui->startButton->setEnabled(true);
        ui->resetButton->setEnabled(true);
        ui->pauseButton->setEnabled(false);
        ui->replayButton->setEnabled(true);
    }
}

void MainWindow::on_startButton_clicked()  {

    int numThread = ui->threadNumSpinBox->value();
    if (ui->drawPanel->nodeList.size() > 0) {
        md.initTarjan(numThread);
        md.gModel->importGraph(ui->drawPanel->exportGraph());

        // initialize for unrooted case
        md.seenModel->putAllUnrooted(0, md.gModel->nodesGraph);
        md.stealingQueueModel->ids = md.gModel->nodesGraph;

        md.gModel->showGraphInfo();
        md.solveTarjan();
        md.release();
        if (numThread <= 2 && numThread > 0) {
            ui->drawPanel->resetPanel();   ui->drawPanel->repaint();
            oldLine0 = -1;
            oldLine1 = -1;
            oldLogLine = 0;
            visualizeTarjan("log001.txt");
        }
    }
}
void MainWindow::on_pauseButton_clicked()
{
    if (ui->pauseButton->text() == "Pause") {
        isPaused = true;
        ui->pauseButton->setText("Resume");
        ui->pauseButton->setEnabled(true);
    }
    else {
        isPaused = false;
        ui->pauseButton->setText("Pause");
        visualizeTarjan("log001.txt");
    }
}

void MainWindow::on_resetButton_clicked()
{
    ui->drawPanel->reset();
}

void MainWindow::on_replayButton_clicked() {

    if (ui->threadNumSpinBox->value() <= 2) {
        oldLine0 = -1;
        oldLine1 = -1;
        ui->drawPanel->resetPanel();
        ui->drawPanel->repaint();
        oldLogLine = 0;
        visualizeTarjan("log001.txt");
    }
}

void MainWindow::on_horizontalSlider_actionTriggered(int action)
{
    qDebug() << ui->horizontalSlider->value();
}
