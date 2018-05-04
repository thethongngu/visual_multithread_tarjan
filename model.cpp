#include "model.h"

// ----- GRAPH ----- //

Graph::Graph() {}

/** new graph with root node is [init], no edge **/
Graph::Graph(int init) {
    this->initGraph = init;
}

/** get the number of edges **/
int Graph::numEdges() {
    Q_ASSERT(nodesGraph.size() != 0);
    int res = 0;
    for(int i = 0; i < nodesGraph.size(); i++) {
        res += succsGraph[i].size();
    }
    return res;
}

/** shuffle the order of nodes in graph **/
void Graph::suffle() {
    qDebug() << "Graph::suffle() need implementation";
}

void Graph::importGraph(QList< QList<int> > localGraph) {
    for(int i = 0; i < localGraph.size(); i++)
        nodesGraph.append(i);

    for(int i = 0; i < localGraph.size(); i++) {
        succsGraph.append(localGraph[i]);
    }
}

/** modify graph: (0..n-1) nodes, (i, j) | i < j **/
void Graph::fullNonDiv(int n) {
    nodesGraph.clear();
    for(int i = 0; i < n; i++) nodesGraph.append(i);

    for(int i = 0; i < n; i++) {
        succsGraph.append(QList<int>());
        for(int j = i + 1; j < n; j++) succsGraph[i].append(j);
    }
}

/** add one more edge(i, j) to fullNonDiv graph **/
void Graph::oneRev(int n, int i, int j) {
    fullNonDiv(n);
    succsGraph[i].append(j);
}

/** print graph info to stdout **/
void Graph::showGraphInfo() {
    qDebug() << "GraphInfo: ";
    qDebug() << "   Node size: " << nodesGraph.size();
    qDebug() << "   Edge size: " << numEdges();
    for(int i = 0; i < nodesGraph.size(); i++) {
        qDebug() << "   " << i;
        for(int j = 0; j < succsGraph[i].size(); j++)
            qDebug() << "       " << succsGraph[i][j];
    }
}

// ----- LOGGER ----- //

Logger::Logger() {}

/** create logger with this filename */
Logger::Logger(QString filename) {
    outputFileLogger.setFileName(filename);
    streamLogger.setDevice(&outputFileLogger);
    outputFileLogger.open(QFile::WriteOnly | QFile::Truncate);
    outputFileLogger.close();
}

void Logger::init(QString filename) {
    outputFileLogger.setFileName(filename);
    streamLogger.setDevice(&outputFileLogger);
    outputFileLogger.open(QFile::WriteOnly | QFile::Truncate);
    outputFileLogger.close();
}

/** append new log to logger **/
void Logger::addLog(QString log) {
    QMutexLocker locker(&mutexLogger);
    outputFileLogger.open(QFile::WriteOnly | QFile::Append);
    log += QString(" (time: %1)\n").arg(QDateTime().currentDateTime().toTime_t());
    streamLogger << log;
    outputFileLogger.close();
}

/** append new endline to logger **/
void Logger::breath() {
    outputFileLogger.open(QFile::WriteOnly | QFile::Append);
    QString log = "\n";
    streamLogger << log;
    outputFileLogger.close();
}

// ----- PARSEARCHNODE ----- //

/** create new node with identity: [node] **/
ParSearchNode::ParSearchNode(Model* sp, int node) :
    mutexParSearchNode(QMutex::Recursive)
{
    this->spParSearchNode = sp;
    this->nodeParSearchNode = node;
    indexParSearchNode.store(-1);
    lowlinkParSearchNode.store(-1);
    searchParSearchNode = nullptr;
    succsParSearchNode = QList<ParSearchNode*>();

    flagsParSearchNode = 0;
    blockedParSearchNode = QList<Search*>();
    nextIxParSearchNode = 0;
}

bool ParSearchNode::claimed() {
    return ((flagsParSearchNode & 1) != 0);
}
void ParSearchNode::setClaimed() {
    flagsParSearchNode = (flagsParSearchNode | 1);
}
bool ParSearchNode::complete() {
    return ((flagsParSearchNode & 2) != 0);
}
void ParSearchNode::setComp() {
    flagsParSearchNode = (flagsParSearchNode | 2);
}

/** is this node in stack of seach s **/
bool ParSearchNode::inStack(Search *s) {
    return (s == searchParSearchNode);
}

/** is this node unclaimed by any search **/
bool ParSearchNode::isNew() {
    return !claimed();
}

/** try to take the ownership of this node by search s **/
bool ParSearchNode::takeOwnership(Search* s) {
    QMutexLocker locker(&mutexParSearchNode);
    if (!claimed()) {
        Q_ASSERT(searchParSearchNode == nullptr);
        setClaimed();
        searchParSearchNode = s;
        return true;
    }
    else return false;
}

/** try to take the ownership of this empty node (no child) **/
bool ParSearchNode::takeOwnershipEmpty() {
    QMutexLocker locker(&mutexParSearchNode);
    if (!claimed()) {
        setClaimed();
        setComplete();
        return true;
    }
    else return false;
}

/** try to take ownership of this node, and other related information **/
QPair< QPair<bool, bool>, QPair<bool, int> > ParSearchNode::getState(Search* s) {
     QMutexLocker locker(&mutexParSearchNode);
     if (!claimed()) {
         searchParSearchNode = s;   setClaimed();
         QPair<bool, bool> pair1(true, false);
         QPair<bool, int> pair2(false, -1);
         return QPair< QPair<bool, bool>, QPair<bool, int> >(pair1, pair2);
     }
     else {
         if (!complete() && !inStack(s)) blockedParSearchNode.prepend(s);
         QPair<bool, bool> pair1(false, complete());
         QPair<bool, int> pair2(inStack(s), indexParSearchNode);
         return QPair< QPair<bool, bool>, QPair<bool, int> >(pair1, pair2);
     }
}

/** init this node (the ownership is already taken -> doesn't need synchronized) **/
void ParSearchNode::init(QList<ParSearchNode *> succs, int index) {
    this->succsParSearchNode = succs;   this->indexParSearchNode = index;   lowlinkParSearchNode = index;
}

/** return the next successor node **/
ParSearchNode* ParSearchNode::next() {
    int N = succsParSearchNode.length();
    int badStart = N;

    while (nextIxParSearchNode < N) {
        ParSearchNode* res = succsParSearchNode[nextIxParSearchNode];  // pointer of next succ node
        if (res->complete()) {
            nextIxParSearchNode += 1;
        }
        else if (res->searchParSearchNode == nullptr || res->searchParSearchNode == this->searchParSearchNode || nextIxParSearchNode >= badStart) {
            nextIxParSearchNode += 1;   return res;
        }
        else {
            badStart -= 1;
            succsParSearchNode[nextIxParSearchNode] = succsParSearchNode[badStart];
            succsParSearchNode[badStart] = res;
        }
    }
    succsParSearchNode.clear();
    return nullptr;
}

/** update lowlink index (the ownership is already taken) **/
void ParSearchNode::updateLowlink(int ix) {
    if (lowlinkParSearchNode > ix) lowlinkParSearchNode = ix;
}

/** unblock any other searches that this is blocking **/
void ParSearchNode::unblock() {
    QMutexLocker locker(&mutexParSearchNode);
    if (!blockedParSearchNode.isEmpty()) {
        for(int i = 0; i < blockedParSearchNode.size(); i++) {
            Search* s = blockedParSearchNode[i];
            s->unblock(this);
            spParSearchNode->loggerModel.addLog(QString("22_%1_%2. Node: %1 unblock Search: %2").arg(nodeParSearchNode).arg(s->idSearch));
        }
        spParSearchNode->suspendedModel->unsuspend(blockedParSearchNode);
        spParSearchNode->pendingModel->makePending(blockedParSearchNode);
        blockedParSearchNode = QList<Search*>();
    }
}

/** set the node is completed by search, unblock any other search is blocked on this node **/
void ParSearchNode::setComplete() {
    QMutexLocker locker(&mutexParSearchNode);
    setComp();
    unblock();
    Q_ASSERT(blockedParSearchNode.empty());
}

/** set the node is completed, check no search needs to unblocking **/
void ParSearchNode::setComplete0() {
    setComp();
    Q_ASSERT(blockedParSearchNode.isEmpty());
}

/** record that search s is no longer blocked waiting for this search **/
void ParSearchNode::noLongerBlocked(Search *s) {
    QMutexLocker locker(&mutexParSearchNode);
    QList<Search*> newBlocked;
    for(int i = 0; i < blockedParSearchNode.size(); i++) {
        if (s != blockedParSearchNode[i]) newBlocked.append(blockedParSearchNode[i]);
    }
    blockedParSearchNode = newBlocked;
}

/** record that search s is now blocked waiting for this search **/
void ParSearchNode::nowBlocked(Search *s) {
    QMutexLocker locker(&mutexParSearchNode);
    blockedParSearchNode.prepend(s);
}

/** transfer to search s, change index and lowlink by delta **/
QPair< QList<Search*>, int > ParSearchNode::transfer(Search* s, int delta) {
    QMutexLocker locker(&mutexParSearchNode);
    searchParSearchNode = s;
    indexParSearchNode += delta;   lowlinkParSearchNode += delta;
    Q_ASSERT(indexParSearchNode.load() >= 0 && lowlinkParSearchNode.load() >= 0);
    spParSearchNode->loggerModel.addLog(QString("11_%1_%2_%3_%4_Trời đu. Deadlock cmnr. Transfer đỉnh(%1) sang Search(%4) và update index, lowlink._").arg(nodeParSearchNode).arg(indexParSearchNode).arg(lowlinkParSearchNode).arg(s->idSearch));
    return QPair< QList<Search*>, int >(blockedParSearchNode, indexParSearchNode);
}

// ----- PENDING ----- //

Pending::Pending() :
    mutexPending(QMutex::Recursive)
{
    queuesPending = QQueue<Search*>();
}

/** record that searches in ss in now pending **/
void Pending::makePending(QList<Search *> ss) {
    QMutexLocker locker(&mutexPending);
    queuesPending.append(ss);
}

/** checking if there is any search in queue **/
void Pending::checkDone() {
    Q_ASSERT(queuesPending.isEmpty());
}

/** try to get the pending search **/
Search* Pending::getPending() {
    if (!queuesPending.isEmpty()) {
        QMutexLocker locker(&mutexPending);
        if (!queuesPending.isEmpty()) return queuesPending.dequeue();
        else return nullptr;
    }
    else return nullptr;
}

// ----- SCCSET ----- //

SCCSet::SCCSet() {}

/** the number of workers that using this set **/
SCCSet::SCCSet(int p) :
    mutexSCCSet(QMutex::Recursive)
{
    this->pSCCSet = p;
}

/** add new SCC to set **/
void SCCSet::add(QSet<int> scc) {
    QMutexLocker locker(&mutexSCCSet);
    nonSingletonsSCCSet.append(scc);
}

/** print result to stdout **/
void SCCSet::showResult() {
    qDebug() << "SCCs size: " << nonSingletonsSCCSet.size();
    for(int i = 0; i < nonSingletonsSCCSet.size(); i++) {
        QList<int> scc = nonSingletonsSCCSet[i].toList();
        for(int j = 0; j < scc.size(); j++) {
            qDebug() << scc[j] << " ";
        }
        qDebug () << "endline";
    }
}

int SCCSet::getSize() {
    return nonSingletonsSCCSet.size();
}

// ----- SCHEDULER ----- //

/** number of workers use this scheduler **/
Scheduler::Scheduler(int p, Model* sp) {
    this->pScheduler = p;
    this->spScheduler = sp;
    ALLFLAGSSET = (1 << p) - 1;
}

/** try to get the new search **/
QPair<void*, int> Scheduler::get(int worker) {
    int myMask = (1 << worker);   // the flag for this worker
    flagsScheduler = 0;   // clear all flags
    bool needToSet = true;
    int count = 0;
    int delay = MINDELAY;
    bool done = false;

    while (!done) {
        Search* s = spScheduler->pendingModel->getPending();  // try to get a pending search
        if (s != nullptr) {
            return QPair<void*, int>(static_cast<void*>(s), 0);
        }

        ParSearchNode* node = spScheduler->stealingQueueModel->get(worker);
        if(node != nullptr) {
            return QPair<void*, int>(static_cast<void*>(node), 1);
        }

        if (count < 0) {
            spScheduler->loggerModel.addLog("count < 0 !!!!");
        }
        count += 1;

        // set flag gì đó, đéo hiểu làm gì
        if (needToSet) {
            bool flagSet = false;
            do {
                int f = flagsScheduler.load();
                int newf = f | myMask;
                flagSet = (f == newf || flagsScheduler.testAndSetOrdered(f, newf));
            } while (!flagSet);
            needToSet = false;
        }
        else needToSet = ((flagsScheduler.load() & myMask) == 0);
        done = (flagsScheduler.load() == ALLFLAGSSET);

        if (!done) {
            // implement thread sleep (not implemented)
            delay = MINDELAY;
        }
    }
    return QPair<void*, int>(static_cast<void*>(nullptr), 2);
}

// ----- SEARCH ----- //

Search::Search() {}

/** Each object of this class encapsulates a search
  * @param owner the worker that owns this search
  * @param id this node's identity
  * @param firstNode the node from which the search should start
  * @param firstSuccs the successors of firstNode */
Search::Search(Model* sp, int owner, QString id, ParSearchNode* firstNode, QList<int> firstSuccs) :
    mutexSearch(QMutex::Recursive)
{

    COMPLETED = 0;
    INPROGRESS = 1;
    SUSPENDED = 2;
    PENDING = 3;

    abortedSearch = false;
    this->spSearch = sp;
    this->ownerSearch = owner;
    this->idSearch = id;
    this->indexSearch = 0;
    iterSearch = 0;
    statusSearch.store(INPROGRESS);

    controlStackSearch = QStack<ParSearchNode*>();
    tarjanStackSearch = QStack<ParSearchNode*>();

    bool isNewSearch = firstNode->takeOwnership(this);
    if (isNewSearch) {
        initChild(firstNode, firstSuccs);
        iterSearch += 1;
    }
    else {
        abortedSearch = true;
    }

    waitingForSearch = nullptr;

    sp->loggerModel.addLog(QString("00. Worker: %3 created Search: %1 for Node: %2").arg(idSearch).arg(firstNode->nodeParSearchNode).arg(owner));
    apply();
}

/** record this search is not blocked **/
void Search::unblock(ParSearchNode *n) {
    QMutexLocker locker(&mutexSearch);
    Q_ASSERT(n == waitingForSearch && statusSearch == SUSPENDED);
    statusSearch = PENDING;
}

/** initialize node and add it to stack; create a node corresponding to succs and add them to Seen and StealingQueue **/
void Search::initChild(ParSearchNode *node, QList<int> succs) {
    if (spSearch->rootedModel) {
        QPair<QList<ParSearchNode*>, QList<ParSearchNode*> > resSeen = spSearch->seenModel->putAll(ownerSearch, succs);   // kiểm tra list con của node này trong Seen
        QList<ParSearchNode*> nodeSuccs = resSeen.first;
        QList<ParSearchNode*> newNodes = resSeen.second;
        node->init(nodeSuccs, indexSearch);  indexSearch += 1;    // tạo node này có pointer vào những node con đã seen
        spSearch->stealingQueueModel->putAll(newNodes, ownerSearch);   // thêm vào stealingQueue nodes chờ để được search
        if (indexSearch == 1) {
            spSearch->loggerModel.addLog(QString("01_%1_%2_%3_%4_Node: %1 added to Stack of Search: %4_").arg(node->nodeParSearchNode).arg(node->indexParSearchNode).arg(node->lowlinkParSearchNode).arg(idSearch));
        }
    }
    else {   // program doesn't reach here, don't worry!
        QList<ParSearchNode*> nodeSuccs;
        if (succs.empty()) nodeSuccs = QList<ParSearchNode*>();
        else nodeSuccs = spSearch->seenModel->putAllUnrooted(ownerSearch, succs);
        node->init(nodeSuccs, indexSearch);  indexSearch += 1;
        if (indexSearch == 1) {
            spSearch->loggerModel.addLog(QString("01_%1_%2_%3_%4_Node: %1 added to Stack of Search: %4_").arg(node->nodeParSearchNode).arg(node->indexParSearchNode).arg(node->lowlinkParSearchNode).arg(idSearch));
        }
    }
    controlStackSearch.push(node);  tarjanStackSearch.push(node);
}

/** record that this search will be resume by worker w **/
void Search::resume(int w) {
    QMutexLocker locker(&mutexSearch);
    Q_ASSERT(statusSearch == PENDING);
    ownerSearch = w;
    waitingForSearch = nullptr;  statusSearch = INPROGRESS;
}

/** trả về node đã block search này, null nếu s bị blocked trước khi gặp node mới */
ParSearchNode* Search::getBlockingNode(Search* s) {
    QMutexLocker locker(&mutexSearch);
    Q_ASSERT(waitingForSearch != nullptr && statusSearch == SUSPENDED && waitingForSearch->searchParSearchNode == s);
    return waitingForSearch;
}

/** Transferring nodes to break cycles of blocking **/
QPair< QPair< QList<ParSearchNode*>, QList<ParSearchNode*> >, QPair< ParSearchNode*, bool> > Search::getTransferNodes(ParSearchNode* n1) {
    QMutexLocker locker(&mutexSearch);
    Q_ASSERT(statusSearch == SUSPENDED && waitingForSearch != nullptr);
    ParSearchNode* next = tarjanStackSearch.pop();
    int ll = next->lowlinkParSearchNode;
    bool reachedN1 = (next == n1);

    QList<ParSearchNode*> ts;
    ts.prepend(next);
    while(!reachedN1 || ll < next->indexParSearchNode) {
      next = tarjanStackSearch.pop();
      ts.prepend(next);
      if (next->lowlinkParSearchNode < ll) ll = next->lowlinkParSearchNode;
      if (next == n1) reachedN1 = true;
    }
    ParSearchNode* l = next;

    next = controlStackSearch.pop();
    QList<ParSearchNode*> cs;
    cs.prepend(next);
    while(next != l) {
        next = controlStackSearch.pop();
        cs.prepend(next);
    }

    bool isEmpty = tarjanStackSearch.isEmpty();
    ParSearchNode* oldWaitingFor = waitingForSearch;
    waitingForSearch->noLongerBlocked(this);
    waitingForSearch = l;
    if (!isEmpty) l->nowBlocked(this);
    else {
        Q_ASSERT(controlStackSearch.isEmpty());
        statusSearch.store(COMPLETED);
    }

    QPair< QList<ParSearchNode*>, QList<ParSearchNode*> > pair1(cs, ts);
    QPair< ParSearchNode*, bool > pair2(oldWaitingFor, isEmpty);
    return QPair< QPair< QList<ParSearchNode*>, QList<ParSearchNode*> >, QPair< ParSearchNode*, bool> >(pair1, pair2);
}

/** Transfer (ts, cs) to this search.
 * Return tất cả các search bị blocked bởi các node được transfer
 */
QList<Search*> Search::transfer(QList<ParSearchNode*> cs, QList<ParSearchNode*> ts) {
    QMutexLocker locker(&mutexSearch);
    int delta = indexSearch - cs.front()->indexParSearchNode;
    QList<Search*> allBlocked;
    for(int i = 0; i < ts.size(); i++) {
        ParSearchNode* n = ts[i];
        QPair< QList<Search*>, int > res = n->transfer(this, delta);
        QList<Search*> blocked = res.first;
        int newIndex = res.second;

        allBlocked = blocked + allBlocked;
        if (newIndex > indexSearch) indexSearch = newIndex;
        tarjanStackSearch.push(n);
    }
    for(int i = 0; i < cs.size(); i++) controlStackSearch.push(cs[i]);
    indexSearch = indexSearch + 1;   // index is now greater than all in the stack
    return allBlocked;
}

/** Hàm tách để control việc search node khác dễ hơn
 * Trả về false nếu không expand được.
 */
bool Search::expandEdge(ParSearchNode* parent, ParSearchNode* child) {
    QPair< QPair<bool, bool>, QPair<bool, int> > res = child->getState(this);
    bool isNew = res.first.first;
    bool complete = res.first.second;
    bool inStack = res.second.first;
    int cIndex = res.second.second;

    spSearch->loggerModel.addLog(QString("05_%1_%2_Kiểm tra đỉnh(%1) của Search(%2) đã explored chưa?._").arg(child->nodeParSearchNode).arg(idSearch));
    if(isNew) {
        QList<int> succs = spSearch->gModel->succsGraph[child->nodeParSearchNode];
        initChild(child, succs);
        spSearch->loggerModel.addLog(QString("06_%1_%2_%3_%4_Chưa được explored nên thêm đỉnh (%1) vào stack của Search(%4)._").arg(child->nodeParSearchNode).arg(child->indexParSearchNode).arg(child->lowlinkParSearchNode).arg(idSearch));
        return true;
    }
    else if (complete) {
        spSearch->loggerModel.addLog(QString("07_%1_%2_Đỉnh(%1) đã được explored rồi. Kiểm tra xem của nằm trong stack của Search(%2) không?_").arg(child->nodeParSearchNode).arg(idSearch));
        spSearch->loggerModel.addLog(QString("09_%1_%2_Đỉnh(%1) không nằm trong stack chả Search(%2). Kiểm tra xem của nằm trong stack của Search khác không?_").arg(child->nodeParSearchNode).arg(idSearch));
        return true;
    }
    else if(inStack) {
        parent->updateLowlink(cIndex);
        spSearch->loggerModel.addLog(QString("07_%1_%2_Đỉnh(%1) đã được explored rồi. Kiểm tra xem của nằm trong stack của Search(%2) không?_").arg(child->nodeParSearchNode).arg(idSearch));
        spSearch->loggerModel.addLog(QString("08_%1_%2_%3_%4_%5_Đỉnh(%1) nằm trong stack của Search(%5). Update lowlink của Đỉnh(%1)._").arg(parent->nodeParSearchNode).arg(child->nodeParSearchNode).arg(parent->lowlinkParSearchNode).arg(child->indexParSearchNode).arg(idSearch));
        return true;
    }
    else {   // this node is owned by other search -> this search is blocked

        waitingForSearch = child; statusSearch = SUSPENDED;
        QPair<bool, ParSearchNode*> res = spSearch->suspendedModel->suspend(this, child);
        bool suspended = res.first;
        ParSearchNode* loopNode = res.second;

        spSearch->loggerModel.addLog(QString("07_%1_%2_Đỉnh(%1) đã được explored rồi. Kiểm tra xem của nằm trong stack của Search(%2) không?_").arg(child->nodeParSearchNode).arg(idSearch));
        spSearch->loggerModel.addLog(QString("09_%1_%2_Đỉnh(%1) không nằm trong stack của Search(%2). Kiểm tra xem của nằm trong stack của Search khác không?_").arg(child->nodeParSearchNode).arg(idSearch));
        spSearch->loggerModel.addLog(QString("10_%1_%2_Đỉnh(%1) nằm trong stack của Search khác. Pause Search(%2) lại._").arg(child->nodeParSearchNode).arg(idSearch));

        if (suspended) return false;
        else {   // solve deadlock
            Q_ASSERT(statusSearch == SUSPENDED); // volatile read
            waitingForSearch = nullptr; statusSearch = INPROGRESS;
            child->noLongerBlocked(this); // undo the blocking

            if(loopNode != nullptr) {
                ParSearchNode* pp = controlStackSearch.top();
                Q_ASSERT(loopNode->searchParSearchNode == this);
                pp->updateLowlink(loopNode->indexParSearchNode);
            }
            else{
                // not implemented
            }
        return true;
        } // end of last else (unsuspended)
    } // end of middle else (suspending case)
}

/** main function of search **/
void Search::apply() {
    QMutexLocker locker(&mutexSearch);

    bool done = false;
    bool suspending = false;
    Q_ASSERT(statusSearch.load() == INPROGRESS);

    while(!done && !suspending) {
        if(!controlStackSearch.isEmpty()){
            ParSearchNode* node = controlStackSearch.top();
            ParSearchNode* child = node->next();

            spSearch->loggerModel.addLog(QString("02_%1_Kiểm tra controlStack của Search(%1) còn đỉnh không._").arg(idSearch));
            spSearch->loggerModel.addLog(QString("03_%1_%2_Lấy dỉnh(%1) ở đầu controlStack của Search(%2)_").arg(node->nodeParSearchNode).arg(idSearch));

            if(child != nullptr) {
                spSearch->loggerModel.addLog(QString("04_%1_%2_Đỉnh(%1) của Search(%2) có cạnh chưa đi qua._").arg(node->nodeParSearchNode).arg(idSearch));
                bool ok = expandEdge(node, child);
                if (!ok) {
                    suspending = true;
                }
            }
            else { // Backtrack
                spSearch->loggerModel.addLog(QString("12_%1_%2_Đỉnh(%1) của Search(%2) không còn con nào chưa duyệt_.").arg(node->nodeParSearchNode).arg(idSearch));
                spSearch->loggerModel.addLog(QString("13_%1_%2_Pop đỉnh(%1) trong controlStack của Search(%2)._").arg(controlStackSearch.top()->nodeParSearchNode).arg(idSearch));
                spSearch->loggerModel.addLog(QString("14_%1_Nếu controlStack của Search(%1) chưa rỗng._").arg(idSearch));
                controlStackSearch.pop();
                if(!controlStackSearch.isEmpty()) {
                    controlStackSearch.top()->updateLowlink(node->lowlinkParSearchNode);
                    spSearch->loggerModel.addLog(QString("15_%1_%2_%3_%4_%5_Update lowlink của Đỉnh(%1) của Search(%5)._").arg(controlStackSearch.top()->nodeParSearchNode).arg(node->nodeParSearchNode).arg(controlStackSearch.top()->indexParSearchNode).arg(controlStackSearch.top()->lowlinkParSearchNode).arg(idSearch));
                }
                spSearch->loggerModel.addLog(QString("16_%1_%2_Kiểm tra nếu đỉnh(%1) của Search(%2) có index == lowlink._").arg(node->nodeParSearchNode).arg(idSearch));
                if(node->lowlinkParSearchNode == node->indexParSearchNode) {
                    spSearch->loggerModel.addLog(QString("17_%1_%2_%3_Đỉnh(%1) thuộc SCC mới tìm được._").arg(node->nodeParSearchNode).arg(spSearch->sccsModel->getSize() + 1).arg(idSearch));
                    QList<int> scc = QList<int>();
                    ParSearchNode* w = nullptr;
                    do {
                        w = tarjanStackSearch.pop();
                        scc.prepend(w->nodeParSearchNode);
                        w->setComplete();
                        spSearch->loggerModel.addLog(QString("17_%1_%2_%3_Đỉnh(%1) thuộc SCC mới tìm được._").arg(w->nodeParSearchNode).arg(spSearch->sccsModel->getSize() + 1).arg(idSearch));
                    } while(w != node);
                    QSet<int> sccSet = scc.toSet();
                    spSearch->sccsModel->add(sccSet);
                }
            }
        }
        else {
            done = true;
        }
    } // end of while


    if (done) {
        Q_ASSERT(controlStackSearch.isEmpty() && tarjanStackSearch.isEmpty());
        statusSearch = COMPLETED;
        spSearch->loggerModel.addLog(QString("18_%1_Search(%1) hoàn thành_").arg(idSearch));
    }
    else {
        Q_ASSERT(suspending); // Updates were done earlier
        spSearch->loggerModel.addLog(QString("18_%1_Search(%1) hoàn thành_").arg(idSearch));
    }
}

// ----- SEARCHNODE ----- //

SearchNode::SearchNode() {}

SearchNode::SearchNode(int node) {
    this->nodeSearchNode = node;
    indexSearchNode = 0;
    lowlinkSearchNode = indexSearchNode;
    isNewSearchNode = true;
    inStackSearchNode = true;
    divergentSearchNode = false;

    succsSearchNode.clear();
    NSearchNode = 0;
    nextIxSearchNode = 0;
}

SearchNode::SearchNode(const SearchNode& object) {
    this->nodeSearchNode = object.nodeSearchNode;
    this->indexSearchNode = object.indexSearchNode;
    this->lowlinkSearchNode = object.lowlinkSearchNode;
    this->isNewSearchNode = object.isNewSearchNode;
    this->inStackSearchNode = object.inStackSearchNode;
    this->divergentSearchNode = object.divergentSearchNode;

    this->succsSearchNode = object.succsSearchNode;
    this->NSearchNode = object.NSearchNode;
    this->nextIxSearchNode = object.nextIxSearchNode;
}

SearchNode& SearchNode::operator=(const SearchNode& object) {
    this->nodeSearchNode = object.nodeSearchNode;
    this->indexSearchNode = object.indexSearchNode;
    this->lowlinkSearchNode = object.lowlinkSearchNode;
    this->isNewSearchNode = object.isNewSearchNode;
    this->inStackSearchNode = object.inStackSearchNode;
    this->divergentSearchNode = object.divergentSearchNode;

    this->succsSearchNode = object.succsSearchNode;
    this->NSearchNode = object.NSearchNode;
    this->nextIxSearchNode = object.nextIxSearchNode;
    return *this;
}

/** Initialize the node
 * @param index the node's index in the search
 * @param succs the successor of the node */
void SearchNode::init(int index, QList<int> succs) {
    Q_ASSERT(isNewSearchNode);  isNewSearchNode = false;
    this->indexSearchNode = index;  lowlinkSearchNode = indexSearchNode;
    this->succsSearchNode = succs;  this->NSearchNode = succs.size();
}

int SearchNode::next() {
    if (nextIxSearchNode == NSearchNode) return -1;
    else {
        nextIxSearchNode += 1;
        return succsSearchNode[nextIxSearchNode - 1];
    }
}

int SearchNode::getNextIx() {
    return nextIxSearchNode;
}

void SearchNode::updateLowlink(int ix) {
    if (ix < lowlinkSearchNode) lowlinkSearchNode = ix;
}

bool SearchNode::notEqual(const SearchNode &object) {
    return object.nodeSearchNode != this->nodeSearchNode;
}

// ----- SEEN -----

Seen::Seen(Model* sp) :
    mutexSeen(QMutex::Recursive)
{
    this->spSeen = sp;
}

void Seen::clear() {
    seenSeen.clear();
}

ParSearchNode* Seen::getOrInit(int n) {
    QMutexLocker locker(&mutexSeen);
    if (seenSeen.find(n) == seenSeen.end()) {
        ParSearchNode* node = new ParSearchNode(spSeen, n);
        seenSeen.insert(n, node);
    }
    return seenSeen[n];
}

QPair< QList<ParSearchNode*>, QList<ParSearchNode*> > Seen::putAll(int me, QList<int> ns) {
//    spSeen->loggerModel.addLog(QString("            Putting node to Seen of Worker %1").arg(me));
    QMutexLocker locker(&mutexSeen);
    QList<ParSearchNode*> nodes;
    QList<ParSearchNode*> newNode;
    int size = ns.size();
    for(int i = 0; i < size; i++) {
        if (seenSeen.find(ns[i]) == seenSeen.end()) {  // if n is inserted
            ParSearchNode* n = new ParSearchNode(spSeen, ns[i]);
            seenSeen.insert(ns[i], n);
            nodes.append(n);
            if (i != 0) newNode.prepend(n);
        }
        else nodes.append(seenSeen[ns[i]]);
    }
    return QPair< QList<ParSearchNode*>, QList<ParSearchNode*> >(nodes, newNode);
}

QList<ParSearchNode*> Seen::putAllUnrooted(int me, QList<int> ns) {
//    spSeen->loggerModel.addLog(QString("            Putting node to Seen of Worker %1").arg(me));
    QMutexLocker locker(&mutexSeen);
    int size = ns.size();
    QList<ParSearchNode*> nodes;
    int i = 0;
    while(i < size){
        int n = ns[i]; nodes.append(getOrInit(n));
        i += 1;
    }
    return nodes;
}

// ----- SEQOPENHASHMMAP -----//

SeqOpenHashMap::~SeqOpenHashMap() {
    for(QHash<int, SearchNode*>::iterator it = entriesSeqOpenHashMap.begin(); it != entriesSeqOpenHashMap.end(); it++)
        delete(*it);
}
SeqOpenHashMap::SeqOpenHashMap(int initSize) {
    entriesSeqOpenHashMap.reserve(initSize);
}
SearchNode* SeqOpenHashMap::getOrInit(int n) {
    if (entriesSeqOpenHashMap.find(n) == entriesSeqOpenHashMap.end()) {
        entriesSeqOpenHashMap.insert(n, new SearchNode(n));
    }
    return entriesSeqOpenHashMap[n];
}
void SeqOpenHashMap::clear() {
    entriesSeqOpenHashMap.clear();
}
QPair<SearchNode*, bool> SeqOpenHashMap::getOrInitIsNew(int n) {
    if (entriesSeqOpenHashMap.find(n)  == entriesSeqOpenHashMap.end()) {
        entriesSeqOpenHashMap.insert(n, new SearchNode(n));
        return QPair<SearchNode*, bool>(entriesSeqOpenHashMap[n], true);
    }
    return QPair<SearchNode*, bool>(entriesSeqOpenHashMap[n], true);
}

// ----- SEQSOLVER ----- //

SeqSolver::~SeqSolver() {
    outputFileSeqSolver.close();
}
SeqSolver::SeqSolver() :
    gSeqSolver(0), loggerSeqSolver(), seenSeqSolver(256), sccsSeqSolver(1)
{
    indexSeqSolver = 0;
}

SeqSolver::SeqSolver(Graph g, bool rooted, bool loops, bool lassos, QString filename) :
    gSeqSolver(g), rootedSeqSolver(rooted), loopsSeqSolver(loops), lassosSeqSolver(lassos),
    loggerSeqSolver(filename), seenSeqSolver(256), sccsSeqSolver(1)
{
    indexSeqSolver = 0;
    loggerSeqSolver.addLog("Start new Solver");
    gSeqSolver.showGraphInfo();
}

// initialize new SearchNode and add it to both stacks
void SeqSolver::addNode(int n, SearchNode* sn) {
    loggerSeqSolver.addLog(QString("    Added sn.node: %1 ").arg(sn->nodeSearchNode));
    loggerSeqSolver.addLog(QString("         index: %1").arg(indexSeqSolver));
    loggerSeqSolver.addLog(QString("         n: %1").arg(n));
    loggerSeqSolver.addLog(QString("         g.succs[n].size(): %1 ").arg(gSeqSolver.succsGraph[n].size()));

    sn->init(indexSeqSolver, gSeqSolver.succsGraph[n]);  indexSeqSolver += 1;
    controlStackSeqSolver.push(sn);
    tarjanStackSeqSolver.push(sn);
}

// ~~~ NEED TO ADD LOG ~~~
void SeqSolver::backtrack(SearchNode* parent) {
    controlStackSeqSolver.pop();
    loggerSeqSolver.addLog(QString("     Pop node from controlStack: %1").arg(parent->nodeSearchNode));
    if (!controlStackSeqSolver.isEmpty()) {
        //logger.addLog(QString("     parent: %1 parent.lowlink: %1 nextN.node %3 nextN.index: %4").arg(parent.node).arg(parent.lowlink).arg(nextN.node).arg(nextN.index));
        controlStackSeqSolver.top()->updateLowlink(parent->lowlinkSearchNode);
    }
    loggerSeqSolver.addLog(QString("     parent.lowlink: %1 == parent.index: %2").arg(parent->lowlinkSearchNode).arg(parent->indexSearchNode));
    if (parent->lowlinkSearchNode == parent->indexSearchNode) {
        if (!loopsSeqSolver) {
            SearchNode* w = tarjanStackSeqSolver.pop();
            w->inStackSearchNode = false;
            QList<int> scsg;   scsg.append(w->nodeSearchNode);
            while (w->notEqual(*parent)) {
                w = tarjanStackSeqSolver.pop();
                w->inStackSearchNode = false;
                scsg.prepend(w->nodeSearchNode);
            }
            loggerSeqSolver.addLog(QString("     New SCC found"));
            loggerSeqSolver.breath();
            sccsSeqSolver.add(scsg.toSet());
        }
        else if (lassosSeqSolver) {
            // not implemented
        }
        else {   // loops but not lassos
            // not implemented
        }
    }
}

/** Run Tarjan's algorithm, starting from SearchNode sn with index start */
void SeqSolver::strongConnect(int start, SearchNode* sn) {
    addNode(start, sn);
    while(!controlStackSeqSolver.empty()) {
        SearchNode* parent = controlStackSeqSolver.top();
        int next = parent->next();

        loggerSeqSolver.addLog(QString("     Top node of controlStack: %1").arg(parent->nodeSearchNode));
        loggerSeqSolver.addLog(QString("     Child node of parent: %1").arg(next));
        loggerSeqSolver.breath();

        if (next == -1) {  // we have explored all successors of parent
            backtrack(parent);
        }
        else {  // explore next edge
            SearchNode* nextN = seenSeqSolver.getOrInit(next);
            if (!nextN->isNewSearchNode) {
                if (nextN->inStackSearchNode) {
                    if (lassosSeqSolver) {
                        // not implemented
                    }
                    else if(loopsSeqSolver && parent->notEqual(*nextN)) {
                        // not implemented
                    }
                    else {
                        parent->updateLowlink(nextN->indexSearchNode);
                    }
                }
                else {
                    // not implemented
                }
            }
            else {
                addNode(next, nextN);
            }
        }
    }
}

void SeqSolver::solve() {
    if (rootedSeqSolver) {
        SearchNode* sn = seenSeqSolver.getOrInit(gSeqSolver.initGraph);    Q_ASSERT(sn->isNewSearchNode);
        strongConnect(gSeqSolver.initGraph, sn);
        sccsSeqSolver.showResult();
    }
    else {
        auto node = gSeqSolver.nodesGraph;
        for(int i = 0; i < node.size(); i++) {
            SearchNode* sn = seenSeqSolver.getOrInit(i);
            if (sn->isNewSearchNode) strongConnect(i, sn);
        }
    }
}

// ----- STEALINGQUEUE ----- //

StealingQueue::~StealingQueue() {
//    delete(mutexes);    // destructor only called one time, so it's will be okay
    // I don't know why mutexes is free before ... :)
}

StealingQueue::StealingQueue(int p) {
    this->pStealingQueue = p;
    mutexesStealingQueue = new QMutex[p];   // don't need Recursive call on this array of mutex
//    mutexesStealingQueue = (QMutex*) calloc(p, sizeof(QMutex(QMutex::Recursive)));
    for(int i = 0; i < p; i++) queuesStealingQueue.append(QQueue<ParSearchNode*>());
}

/** Get the next node to consider.  Returns null if there is no such.
    * @param w the worker doing the get */
ParSearchNode* StealingQueue::get(int w) {
    int iters = 0;
    ParSearchNode* result = nullptr;
    while (iters < pStealingQueue && result == nullptr) {
        int i = (w + iters) % pStealingQueue;
        if (!queuesStealingQueue[i].isEmpty()) {
            QMutexLocker locker(&mutexesStealingQueue[i]);
            if (!queuesStealingQueue[i].isEmpty()) result = queuesStealingQueue[i].dequeue();
        }
        iters += 1;
    }
    return result;
}

void StealingQueue::putAll(QList<ParSearchNode*> nodes, int owner) {
    QMutexLocker locker(&mutexesStealingQueue[owner]);
    queuesStealingQueue[owner].append(nodes);
}

void StealingQueue::init(QQueue<ParSearchNode*> queue, int owner) {
    QMutexLocker locker(&mutexesStealingQueue[owner]);
    queuesStealingQueue[owner] = queue;
}

bool StealingQueue::isEmpty() {
    for(int i = 0; i < queuesStealingQueue.size(); i++)
        if (!queuesStealingQueue[i].isEmpty()) return false;
    return true;
}

void StealingQueue::checkDone() {
    Q_ASSERT(isEmpty());
}

// ----- UNROOTEDSTEALINGQUEUE ----- //
UnrootedStealingQueue::UnrootedStealingQueue(QList<int> _ids, Seen* _seen) {
    ids = _ids;
    seenUnrootedStealingQueue = _seen;
    next.store(0);
    size = ids.size();
}

ParSearchNode* UnrootedStealingQueue::get(int w) {
    ParSearchNode* res = nullptr;
    int theNext = next++;
    while (theNext < ids.size() && res == nullptr) {
        int id = ids[theNext];
        ParSearchNode* node = seenUnrootedStealingQueue->getOrInit(id);
        if (node->isNew()) res = node;
        else theNext = next++;
    }
    return res;
}

bool UnrootedStealingQueue::isEmpty() {
    return (next.load() >= size);
}

void UnrootedStealingQueue::clear() {
    next.store(0);
}

void UnrootedStealingQueue::putAll(QList<ParSearchNode*> nodes, int owner) {

}

UnrootedStealingQueue::~UnrootedStealingQueue() {

}

// ----- SUSPENDED ----- //

Suspended::Suspended(Model* sp) :
    mutexSuspended(QMutex::Recursive)
{
    spSuspended = sp;
}

QPair<bool, ParSearchNode*> Suspended::suspend(Search *s, ParSearchNode *n) {
    QList<Search*> path = QList<Search*>();
    QMutexLocker locker(&mutexSuspended);
    Search* blocker = n->searchParSearchNode;
    path = findPath(blocker, s);
    if (path.size() == 0) {
        suspendedSuspended.insert(s, blocker);
    }
    if (path.size() == 0) return QPair<bool, ParSearchNode*>(true, nullptr);
    else {
        Q_ASSERT(path.length());
        return transfer(s, path);
    }
}

QPair<bool, ParSearchNode*> Suspended::transfer(Search *s, QList<Search *> path) {
    QList< QPair< QList<Search*>, Search* > > blockedPairs;
    QList<Search*> empties;
    QList<Search*> nonEmpties;

    ParSearchNode* n1 = s->getBlockingNode(path[0]);

    for(int i = 0; i < path.size(); i++) {
        Search* s1 = path[i];
        Q_ASSERT(n1 != nullptr);
        QPair< QPair< QList<ParSearchNode*>, QList<ParSearchNode*> >, QPair< ParSearchNode*, bool> > res = s1->getTransferNodes(n1);
        n1 = res.second.first;
        QList<Search*> bs = s->transfer(res.first.first, res.first.second);
        QList<Search*> bs1;
        for(int i = 0; i < bs.size(); i++) {
            if (bs[i] != s && !path.contains(bs[i])) bs1.append(bs[i]);
        }
        if (!bs1.isEmpty()) {
            blockedPairs.prepend(QPair< QList<Search*>, Search* >(bs1, s1));
        }

        if (res.second.second) {
            empties.prepend(s1);
        }
        else {
            nonEmpties.prepend(s1);
        }
    }

    QMutexLocker locker(&mutexSuspended);
    for(int i = 0; i < blockedPairs.size(); i++) {
        QList<Search*> bs = blockedPairs[i].first;
        while (!bs.isEmpty()) {
            Search* b = bs.first();
            bs.erase(bs.begin());
            if (suspendedSuspended.contains(b)) {
                if (suspendedSuspended[b] == blockedPairs[i].second) suspendedSuspended[b] = s;
                else {
                    Q_ASSERT(suspendedSuspended[b] == s);
                }
            }
        }
    }

    for(int i = 0; i < empties.size(); i++) suspendedSuspended.remove(empties[i]);
    for(int i = 0; i < nonEmpties.size(); i++) suspendedSuspended[nonEmpties[i]] = s;

    return QPair<bool, ParSearchNode*>(false, n1);
}

QList<Search*> Suspended::findPath(Search *start, Search *target) {
    Q_ASSERT(target != start);
    Search* current = start;
    QList<Search*> path;     path.append(current);
    while (suspendedSuspended.contains(current) && current != target) {
        current = suspendedSuspended[current];    Q_ASSERT(!path.contains(current));
        if (current != target) path.prepend(current);
    }
    if (current != target) return QList<Search*>();
    else {
        QList<Search*> res;
        for(int i = path.size() - 1; i >= 0; i--) res.append(path[i]);
        return res;
    }
}

void Suspended::unsuspend(QList<Search *> ss) {
    QMutexLocker locker(&mutexSuspended);
    // ~~~ ADD assertion here
    for(int i = 0; i < ss.size(); i++) suspendedSuspended.remove(ss[i]);
}

void Suspended::checkDone() {
    Q_ASSERT(suspendedSuspended.isEmpty());
}

int Suspended::size() {
    QMutexLocker locker(&mutexSuspended);
    return suspendedSuspended.size();
}

// ----- WORKER ----- //

Worker::~Worker() {

}

QString Worker::getSearchId() {
    QString res = QString("%1%2").arg(meWorker).arg(searchNoWorker);
    searchNoWorker += 1;
    return res;
}
Worker::Worker(int me, Model *_sp) {
    this->spWorkerWorker = _sp;
    this->meWorker = me;

    searchWorker = nullptr;
    searchNoWorker = 0;  // number of the current search
    spWorkerWorker->loggerModel.addLog(QString("00. Worker %1: initialized").arg(me));
}

void Worker::apply() {

    if(!spWorkerWorker->unrootedModel && meWorker == 0){
        int initNodeId = spWorkerWorker->gModel->initGraph;  // Add start node to search of worker 0
        QList<int> initNode;   initNode.append(initNodeId);
        QPair< QList<ParSearchNode*>, QList<ParSearchNode*> > newN = spWorkerWorker->seenModel->putAll(meWorker, initNode);
        QList<ParSearchNode*> nodes = newN.first;
        QList<ParSearchNode*> newNodes = newN.second;
        Q_ASSERT(nodes.size() == 1 && newNodes.size() == 0);   // đang khởi tạo node mới

        ParSearchNode* firstNode = nodes.front();   // firstNode là con trỏ vào nút vừa tạo
        QList<int> firstSuccs = spWorkerWorker->gModel->succsGraph[firstNode->nodeParSearchNode];  // lấy các nút con của firstNode
        searchWorker = new Search(spWorkerWorker, meWorker, getSearchId(), firstNode, firstSuccs);   // khởi tạo search mới từ node này
    }

    bool done = false;
    while (!done) {
        // do we need to clean up searchWorker here ??? ~~~ //

        QPair<void*, int> resSched = spWorkerWorker->schedulerModel->get(meWorker);
        if (resSched.second == 0) {
            searchWorker = static_cast<Search*>(resSched.first);
            searchWorker->resume(meWorker);
            searchWorker->apply();
            spWorkerWorker->loggerModel.addLog(QString("    Scheduler have resumed pending Search: %1").arg(searchWorker->idSearch));
        }
        if (resSched.second == 1) {
            ParSearchNode* firstNode = static_cast<ParSearchNode*>(resSched.first);
            spWorkerWorker->loggerModel.addLog(QString("00. Scheduler found Node: %1 for Worker: %2").arg(firstNode->nodeParSearchNode).arg(meWorker));
            if (firstNode->isNew()) {

                QList<int> firstSuccs = spWorkerWorker->gModel->succsGraph[firstNode->nodeParSearchNode];
                if (firstSuccs.isEmpty()) {
                    spWorkerWorker->loggerModel.addLog(QString("00. Node: %1 of Worker: %2 have no child").arg(firstNode->nodeParSearchNode).arg(meWorker));
                    searchWorker = nullptr;
                    if (firstNode->takeOwnershipEmpty()) {
                        QSet<int> scc = QSet<int>();   scc.insert(firstNode->nodeParSearchNode);
                        spWorkerWorker->sccsModel->add(scc);
                        spWorkerWorker->loggerModel.addLog(QString("21_%1_%2_Đỉnh(%1) không có con, thuộc SCC(%2)_").arg(firstNode->nodeParSearchNode).arg(spWorkerWorker->sccsModel->getSize()));
                    }
                }
                else {
                    searchWorker = new Search(spWorkerWorker, meWorker, getSearchId(), firstNode, firstSuccs);
                    if (searchWorker->abortedSearch) {
                        spWorkerWorker->loggerModel.addLog(QString("00. Node: %1 is running by other -> must stop Search: %2").arg(firstNode->nodeParSearchNode).arg(searchWorker->idSearch));
                        searchWorker = nullptr;
                    }
                }
            }
            else {
                searchWorker = nullptr;
                spWorkerWorker->loggerModel.addLog(QString("00. Node: %1 is COMPLETED by other").arg(firstNode->nodeParSearchNode));
            }

        }
        if (resSched.second == 2) {
            spWorkerWorker->loggerModel.addLog(QString("00. Worker: %1 is done!").arg(meWorker));
            done = true;
        }
        iterWorker += 1;
    }
}

// ----- MODEL ----- //
Model::Model() :
    loggerModel("log001.txt")
{
    pModel = 1;
//    rootedModel = true;
    rootedModel = false;
    unrootedModel = !rootedModel;
    gModel = nullptr;
    seenModel = nullptr;
    stealingQueueModel = nullptr;
    pendingModel = nullptr;
    suspendedModel = nullptr;
    schedulerModel = nullptr;
    sccsModel = nullptr;    
}

Model::~Model() {
}

void Model::initTarjan(int _p) {
    loggerModel.init("log001.txt");
    pModel = _p;
//    rootedModel = true;
    rootedModel = false;
    unrootedModel = !rootedModel;
    gModel = new Graph(0);
    seenModel = new Seen(this);
//    stealingQueueModel = new StealingQueue(pModel);
    stealingQueueModel = new UnrootedStealingQueue(gModel->nodesGraph, seenModel);
    pendingModel = new Pending();
    suspendedModel = new Suspended(this);
    schedulerModel = new Scheduler(pModel, this);
    sccsModel = new SCCSet(pModel);
}

void Model::release() {
    delete(gModel);
    delete(stealingQueueModel);
    delete(pendingModel);
    delete(suspendedModel);
    delete(schedulerModel);
    delete(seenModel);
    delete(sccsModel);
}

void Model::solveTarjan() {

    // Initialize p Worker data
    QList<Worker*> runner;
    for(int i = 0; i < pModel; i++) {
        Worker* ww = new Worker(i, this);
        runner.append(ww);
    }

    QFuture<void> future;
    for(int i = 0; i < pModel; i++) {
        future = QtConcurrent::run(runner[i], &Worker::apply);
    }
    future.waitForFinished();

    for(int i = 0; i < pModel; i++) delete(runner[i]);
    loggerModel.addLog(QString("Complete Tarjan"));
}
