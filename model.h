#ifndef MODEL_H
#define MODEL_H

#include <QDebug>
#include <QStack>
#include <QFile>
#include <QQueue>
#include <QDateTime>
#include <QList>
#include <QMutex>
#include <QMutexLocker>
#include <QPair>
#include <QString>
#include <QtConcurrent/QtConcurrent>
#include <QFuture>

class ParSearchNode;
class SCCSet;
class Seen;
class StealingQueue;
class Model;

class Graph {
public:
    /** constructor **/
    Graph();
    Graph(int init);

    /** attributes **/
    QList<int> nodesGraph;
    QList< QList<int> > succsGraph;
    int initGraph;  // the initial node

    /** methods **/
    void importGraph(QList< QList<int> > localGraph);

    int numEdges();
    void suffle();
    void fullNonDiv(int n);
    void oneRev(int n, int i, int j);
    void random(int n, float p);
    void rectangularGrid(int m, int n);
    void independentLoops(int k, int n);
    void multipleLoops(int n, int k);

    void showGraphInfo();
};
class Logger {
public:
    /** constructor **/
    Logger();
    Logger(QString filename);

    /** attributes **/
    QFile outputFileLogger;
    QTextStream streamLogger;


    /** methods **/
    void addLog(QString log);
    void breath();
    void init(QString filename);
private:
    QMutex mutexLogger;
};
class Search {
public:
    Search();
    Search(Model* sp, int owner, QString id, ParSearchNode* firstNode, QList<int> firstSuccs);

    /** attributes **/
    Model* spSearch;
    bool abortedSearch;
    QString idSearch;
    int ownerSearch;
    int COMPLETED;
    int INPROGRESS;
    int SUSPENDED;
    int PENDING;

    /** methods **/
    void resume(int w);
    void unblock(ParSearchNode* n);
    ParSearchNode* getBlockingNode(Search* s);
    QPair< QPair< QList<ParSearchNode*>, QList<ParSearchNode*> >, QPair< ParSearchNode*, bool> > getTransferNodes(ParSearchNode* n1);
    QList<Search*> transfer(QList<ParSearchNode*> cs, QList<ParSearchNode*> ts);
    void apply();

private:
    /** attributes */
    int iterSearch;
    QAtomicInteger<int> statusSearch;

    QStack<ParSearchNode*> controlStackSearch;
    QStack<ParSearchNode*> tarjanStackSearch;
    int indexSearch;
    ParSearchNode* waitingForSearch;

    QMutex mutexSearch;

    /** methods **/
    void initChild(ParSearchNode* node, QList<int> succs);
    bool expandEdge(ParSearchNode* parent, ParSearchNode* child);
};
class ParSearchNode
{
public:
    ParSearchNode(Model* sp, int node);

    /** attributes **/
    int nodeParSearchNode;
    Model* spParSearchNode;
    QAtomicInteger<int> indexParSearchNode;
    QAtomicInteger<int> lowlinkParSearchNode;
    Search* searchParSearchNode;
    QList<ParSearchNode*> succsParSearchNode;

    /** methods **/
    bool isNew();
    bool takeOwnership(Search* search);
    bool takeOwnershipEmpty();
    void init(QList<ParSearchNode*> succs, int index);
    ParSearchNode* next();
    void updateLowlink(int ix);
    void setComplete();
    void setComplete0();
    void unblock();
    void noLongerBlocked(Search* s);
    void nowBlocked(Search* s);
    QPair< QList<Search*>, int > transfer(Search* s, int delta);
    QPair< QPair<bool, bool>, QPair<bool, int> > getState(Search* s);

private:
    /** attributes **/
    int flagsParSearchNode;
    int nextIxParSearchNode;
    QList<Search*> blockedParSearchNode;
    QMutex mutexParSearchNode;

    /** methods **/
    bool claimed();
    void setClaimed();
    bool complete();
    void setComp();
    bool inStack(Search* s);

};
class Pending
{
public:
    Pending();

    /** methods **/
    void makePending(QList<Search*> ss);
    void checkDone();
    Search* getPending();

private:
    /** attributes **/
    QQueue<Search*> queuesPending;
    QMutex mutexPending;

};
class SCCSet {
public:
    /** constructor **/
    SCCSet();
    SCCSet(int p);

    /** attributes **/
    int pSCCSet;

    /** methos **/
    void add(QSet<int> scc);
    void showResult();
    int getSize();

private:
    /** attributes **/
    QList< QSet<int> > nonSingletonsSCCSet;  // each set S in nonSingletons represents itself. Each member of nonSingletons has size > 1.
    QMutex mutexSCCSet;
};
class Scheduler {
public:
    /** constructor */
    Scheduler();
    Scheduler(int p, Model* sp);

    /** attributes **/
    int pScheduler;
    Model* spScheduler;

    /** methods **/
    QPair<void*, int> get(int worker);

private:
    int ALLFLAGSSET;
    QAtomicInteger<int> flagsScheduler;
    int MINDELAY;
    int MAXDELAY;
};
class SearchNode {
public:
    /** constructor **/
    SearchNode();
    SearchNode(int node);
    SearchNode(const SearchNode &object);
    SearchNode& operator=(const SearchNode& object);

    /** methods **/
    void init(int index, QList<int> succs);
    int next();
    void updateLowlink(int ix);
    bool notEqual(const SearchNode& object);
    int getNextIx();

    /** attributes **/
    int nodeSearchNode;
    bool divergentSearchNode;  // is this node in tau-loop
    int indexSearchNode;
    bool inStackSearchNode;
    bool isNewSearchNode;
    int lowlinkSearchNode;

private:
    /** attributes **/
    int NSearchNode;  // number of successors
    int nextIxSearchNode;  // index of next successor
    QList<int> succsSearchNode;  // node's successor (~~~ NEED TO REVIEW FOR PERFORMANCE ON QLIST ~~~)
};
class Seen {
public:
    Seen(Model* sp);

    /** attributes **/
    QMutex mutexSeen;
    Model* spSeen;

    /** methods **/
    void clear();
    ParSearchNode* getOrInit(int n);
    QPair< QList<ParSearchNode*>, QList<ParSearchNode*> > putAll(int me, QList<int> ns);
    QList<ParSearchNode*> putAllUnrooted(int me, QList<int> ns);

private:
    QHash<int, ParSearchNode*> seenSeen;
};
class SeqOpenHashMap {
public:
    /** constructor **/
    SeqOpenHashMap(int initSize);
    ~SeqOpenHashMap();

    /** attributes **/


    /** methods **/
    SearchNode* getOrInit(int n);  // If n is not in the map, initialize it to a new ParSearchNode; otherwise return Node
    QPair<SearchNode*, bool> getOrInitIsNew(int n);  // like getOrInit() but return boolean say whether node is new or note
    bool contain(int n);
    void clear();

private:
    /** attributes **/
    int maxLoadFactorSeqOpenHashMap;
    QHash<int, SearchNode*> entriesSeqOpenHashMap;
    int tableSizeSeqOpenHashMap;
    int numEntriesSeqOpenHashMap;
    int threadholdSeqOpenHashMap;
};
class SeqSolver {
public:
    /** contrustor **/
    SeqSolver();
    SeqSolver(Graph g, bool rooted, bool loops, bool lassos, QString filename);

    /** destructor **/
    ~SeqSolver();

    /** attributes **/
    Graph gSeqSolver;
    bool rootedSeqSolver;
    bool loopsSeqSolver;
    bool lassosSeqSolver;
    Logger loggerSeqSolver;
    SeqOpenHashMap seenSeqSolver;
    QFile outputFileSeqSolver;

    // ~~~ seen = OpenHashMap[int, int] ~~~
    QStack<SearchNode*> controlStackSeqSolver;
    QStack<SearchNode*> tarjanStackSeqSolver;
    int indexSeqSolver;

    SCCSet sccsSeqSolver;  // SCCs found so far
    QList<int> divsSeqSolver;  // the nodes on loops found so far

    /** methods **/
    void solve();


private:
    /** methods **/
    void log();
    void addNode(int n, SearchNode* sn);
    void backtrack(SearchNode* parent);
    void strongConnect(int start, SearchNode* sn);


};
class StealingQueue {
public:
    StealingQueue(int p);
    ~StealingQueue();

    /** attributes **/
    int pStealingQueue;
    QList< QQueue<ParSearchNode*> > queuesStealingQueue;

    /** methods **/
    ParSearchNode* get(int w);
    bool isEmpty();
    void checkDone();
    void putAll(QList<ParSearchNode*> nodes, int owner);
    void init(QQueue<ParSearchNode*> queue, int owner);
private:
    QMutex* mutexesStealingQueue;
};
class UnrootedStealingQueue {
public:
    UnrootedStealingQueue(QList<int> _ids, Seen* _seen);
    ~UnrootedStealingQueue();

    Seen* seenUnrootedStealingQueue;
    QList<int> ids;

    ParSearchNode* get(int w);
    void putAll(QList<ParSearchNode*> nodes, int owner);
    bool isEmpty();
    void clear();
private:
    QAtomicInteger<int> next;
    int size;
};
class Suspended {
public:
    Suspended(Model* sp);

    /** attributes **/
    Model* spSuspended;

    /** methods **/
    QPair<bool, ParSearchNode*> suspend(Search* s, ParSearchNode* n);
    void unsuspend(QList<Search*> ss);
    void checkDone();
    int size();

private:
    QMap<Search*, Search*> suspendedSuspended;
    QPair<bool, ParSearchNode*> transfer(Search* s, QList<Search*> path);
    QList<Search*> findPath(Search* start, Search* target);
    QMutex mutexSuspended;
};
class Worker {
public:
    /** constructor **/
    Worker(int me, Model *sp);
    ~Worker();

    /** attributes **/
    int meWorker;

    /** methods **/
    void apply();
private:

    /** attributes **/
    int iterWorker;
    Model* spWorkerWorker;
    Search* searchWorker;
    int searchNoWorker;

    /** methods **/
    QString getSearchId();
};

class Model
{
public:
    Model();
    ~Model();

    /** attributes **/
    int pModel;
    bool rootedModel;
    bool unrootedModel;
    Graph* gModel;
    Seen* seenModel;
//    StealingQueue* stealingQueueModel;
    UnrootedStealingQueue* stealingQueueModel;
    Pending* pendingModel;
    Suspended* suspendedModel;
    Scheduler* schedulerModel;
    SCCSet* sccsModel;
    Logger loggerModel;

    /** methods **/
    void initTarjan(int _p);
    void release();
    void solveTarjan();
};

#endif // MODEL_H
