#ifndef CGRAPH_H
#define CGRAPH_H

#include <list>
#include <iostream>

template<class DATA, class COST>
class CGraphNode;

template<class DATA, class COST>
class CGraphEdge;

template<class DATA, class COST>
class CGraph {
 public:
  typedef CGraphNode<DATA,COST> Node;
  typedef CGraphEdge<DATA,COST> Edge;

  typedef std::list<Node *> NodeList;
  typedef std::list<Edge *> EdgeList;

 public:
  CGraph() :
   debug_(false) {
  }

 ~CGraph() {
    reset();
  }

  CGraph(const CGraph &graph) {
    typename NodeList::const_iterator pnode1 = graph.nodes_.begin();
    typename NodeList::const_iterator pnode2 = graph.nodes_.end  ();

    for ( ; pnode1 != pnode2; ++pnode1)
      addNode((*pnode1)->getData());

    typename EdgeList::const_iterator pedge1 = graph.edges_.begin();
    typename EdgeList::const_iterator pedge2 = graph.edges_.end  ();

    for ( ; pedge1 != pedge2; ++pedge1)
      addEdge((*pedge1)->getNode1()->getData(),
              (*pedge1)->getNode2()->getData(),
              (*pedge1)->getCost());
  }

  const CGraph &operator=(const CGraph &graph) {
    reset();

    typename NodeList::const_iterator pnode1 = graph.nodes_.begin();
    typename NodeList::const_iterator pnode2 = graph.nodes_.end  ();

    for ( ; pnode1 != pnode2; ++pnode1)
      addNode((*pnode1)->getData());

    typename EdgeList::const_iterator pedge1 = graph.edges_.begin();
    typename EdgeList::const_iterator pedge2 = graph.edges_.end  ();

    for ( ; pedge1 != pedge2; ++pedge1)
      addEdge((*pedge1)->getNode1()->getData(),
              (*pedge1)->getNode2()->getData(),
              (*pedge1)->getCost());

    return *this;
  }

  void reset() {
    typename NodeList::const_iterator pnode1 = nodes_.begin();
    typename NodeList::const_iterator pnode2 = nodes_.end  ();

    for ( ; pnode1 != pnode2; ++pnode1)
      delete *pnode1;

    nodes_.clear();

    typename EdgeList::const_iterator pedge1 = edges_.begin();
    typename EdgeList::const_iterator pedge2 = edges_.end  ();

    for ( ; pedge1 != pedge2; ++pedge1)
      delete *pedge1;

    edges_.clear();
  }

  void setDebug(bool debug=true) { debug_ = debug; }

  const NodeList &getNodes() const { return nodes_; }
  const EdgeList &getEdges() const { return edges_; }

  Node *addNode(DATA data) {
    Node *node = new Node(data);

    nodes_.push_back(node);

    return node;
  }

  Edge *addEdge(DATA data1, DATA data2, COST cost) {
    Node *node1 = getNode(data1);
    Node *node2 = getNode(data2);

    return addEdge(node1, node2, cost);
  }

  Edge *addEdge(Node *node1, Node *node2, COST cost) {
    Edge *edge = new Edge(node1, node2, cost);

    edges_.push_back(edge);

    node1->addNode(node2);
    node2->addNode(node1);

    node1->addEdge(edge);
    node2->addEdge(edge);

    return edge;
  }

  void removeEdge(Edge *edge) {
    edges_.remove(edge);

    Node *node1 = edge->getNode1();
    Node *node2 = edge->getNode2();

    node1->removeNode(node2);
    node2->removeNode(node1);

    node1->removeEdge(edge);
    node2->removeEdge(edge);

    delete edge;
  }

  Node *getNode(DATA data) const {
    typename NodeList::const_iterator pnode1 = nodes_.begin();
    typename NodeList::const_iterator pnode2 = nodes_.end  ();

    for ( ; pnode1 != pnode2; ++pnode1)
      if ((*pnode1)->getData() == data)
        return *pnode1;

    return NULL;
  }

  CGraph minimumSpaningTree() {
    //minimumSpaningTree_Kruskal();

    return minimumSpaningTree_Prim();
  }

  CGraph minimumSpaningTree_Kruskal() {
    CGraph new_tree;

    int num_nodes = nodes_.size();
    int num_edges = edges_.size();

    if (num_nodes == 0 || num_edges == 0)
      return new_tree;

    std::list<Edge *> in_edges;

    typename EdgeList::const_iterator pedge1 = edges_.begin();
    typename EdgeList::const_iterator pedge2 = edges_.end  ();

    for ( ; pedge1 != pedge2; ++pedge1)
      in_edges.push_back(*pedge1);

    int num_in_edges = in_edges.size();

    while (num_in_edges > 0) {
      Edge *min_edge = NULL;
      COST  min_cost = 0;

      typename std::list<Edge *>::iterator pedge1 = in_edges.begin();
      typename std::list<Edge *>::iterator pedge2 = in_edges.end  ();

      for ( ; pedge1 != pedge2; ++pedge1) {
        COST cost = (*pedge1)->getCost();

        if (min_edge == NULL || cost < min_cost) {
          min_edge = *pedge1;
          min_cost = cost;
        }
      }

      if (min_edge == NULL)
        break;

      in_edges.remove(min_edge);

      --num_in_edges;

      Node *min_node1 = min_edge->getNode1();
      Node *min_node2 = min_edge->getNode2();

      DATA data1 = min_node1->getData();
      DATA data2 = min_node2->getData();

      min_node1 = new_tree.getNode(data1);
      min_node2 = new_tree.getNode(data2);

      if (min_node1 == NULL)
        min_node1 = new_tree.addNode(data1);

      if (min_node2 == NULL)
        min_node2 = new_tree.addNode(data2);

      min_edge = new_tree.addEdge(data1, data2, min_cost);

      if (new_tree.isCycle(min_node1) || new_tree.isCycle(min_node2)) {
        new_tree.removeEdge(min_edge);
      }
    }

    if (debug_) {
      std::cout << "Kruskal" << std::endl;

      std::cout << new_tree << std::endl;
    }
  }

  CGraph minimumSpaningTree_Prim() {
    CGraph new_tree;

    int num_nodes = nodes_.size();
    int num_edges = edges_.size();

    if (num_nodes == 0 || num_edges == 0)
      return new_tree;

    typename NodeList::iterator pnode1 = nodes_.begin();
    typename NodeList::iterator pnode2 = nodes_.end  ();

    for ( ; pnode1 != pnode2; ++pnode1)
      (*pnode1)->setProcessed(false);

    typename EdgeList::iterator pedge1 = edges_.begin();
    typename EdgeList::iterator pedge2 = edges_.end  ();

    for ( ; pedge1 != pedge2; ++pedge1)
      (*pedge1)->setProcessed(false);

    NodeList processed_nodes;

    pnode1 = nodes_.begin();

    processed_nodes.push_back(*pnode1);

    (*pnode1)->setProcessed(true);

    int num_processed_edges = 0;

    while (num_processed_edges < num_edges - 1) {
      Edge *min_edge = NULL;
      Node *min_node = NULL;
      COST  min_cost = 0;

      typename EdgeList::iterator pedge1 = edges_.begin();
      typename EdgeList::iterator pedge2 = edges_.end  ();

      for ( ; pedge1 != pedge2; ++pedge1) {
        if ((*pedge1)->getProcessed())
          continue;

        Node *node1 = (*pedge1)->getNode1();
        Node *node2 = (*pedge1)->getNode2();

        bool processed1 = node1->getProcessed();
        bool processed2 = node2->getProcessed();

        Node *node;

        if      (  processed1 && ! processed2)
          node = node2;
        else if (! processed1 &&   processed2)
          node = node1;
        else
          continue;

        COST cost = (*pedge1)->getCost();

        if (min_edge == NULL || cost < min_cost) {
          min_edge = *pedge1;
          min_node = node;
          min_cost = cost;
        }
      }

      if (min_edge == NULL)
        break;

      processed_nodes.push_back(min_node);

      min_node->setProcessed(true);

      Node *min_node1 = min_edge->getNode1();
      Node *min_node2 = min_edge->getNode2();

      DATA data1 = min_node1->getData();
      DATA data2 = min_node2->getData();

      min_node1 = new_tree.getNode(data1);
      min_node2 = new_tree.getNode(data2);

      if (min_node1 == NULL)
        min_node1 = new_tree.addNode(data1);

      if (min_node2 == NULL)
        min_node2 = new_tree.addNode(data2);

      min_edge = new_tree.addEdge(data1, data2, min_cost);

      min_edge->setProcessed(true);

      ++num_processed_edges;
    }

    if (debug_) {
      std::cout << "Prim" << std::endl;

      std::cout << new_tree << std::endl;
    }

    return new_tree;
  }

  bool isCycle(Node *node) const {
    typename EdgeList::const_iterator pedge1 = edges_.begin();
    typename EdgeList::const_iterator pedge2 = edges_.end  ();

    for ( ; pedge1 != pedge2; ++pedge1)
      (*pedge1)->setVisited(false);

    return isCycle(node, node);
  }

  bool isCycle(Node *start_node, Node *node) const {
    std::list<Edge *> edges = start_node->getEdges();

    typename std::list<Edge *>::iterator pedge1 = edges.begin();
    typename std::list<Edge *>::iterator pedge2 = edges.end  ();

    for ( ; pedge1 != pedge2; ++pedge1) {
      if ((*pedge1)->getVisited())
        continue;

      (*pedge1)->setVisited(true);

      Node *node1 = (*pedge1)->getNode1();
      Node *node2 = (*pedge1)->getNode2();

      if (node1 == start_node) {
        if (node == node2)
          return true;

        if (isCycle(node, node2))
          return true;
      }
      else {
        if (node == node1)
          return true;

        if (isCycle(node, node1))
          return true;
      }
    }

    return false;
  }

  void print(std::ostream &os) const {
    os << "Nodes" << std::endl;

    typename NodeList::const_iterator pnode1 = nodes_.begin();
    typename NodeList::const_iterator pnode2 = nodes_.end  ();

    for ( ; pnode1 != pnode2; ++pnode1)
      os << " " << **pnode1 << std::endl;

    os << "Edges" << std::endl;

    typename EdgeList::const_iterator pedge1 = edges_.begin();
    typename EdgeList::const_iterator pedge2 = edges_.end  ();

    for ( ; pedge1 != pedge2; ++pedge1)
      os << " " << **pedge1 << std::endl;
  }

  friend std::ostream &operator<<(std::ostream &os, const CGraph &tree) {
    tree.print(os);

    return os;
  }

 private:
  NodeList nodes_;
  EdgeList edges_;
  bool     debug_;
};

template<class DATA, class COST>
class CGraphNode {
  typedef CGraphNode<DATA,COST> Node;
  typedef CGraphEdge<DATA,COST> Edge;

  typedef std::list<Node *> NodeList;
  typedef std::list<Edge *> EdgeList;

 public:
  CGraphNode(DATA data) : data_(data) { }

  DATA getData() const { return data_; }

  const NodeList &getNodes() const { return nodes_; }
  const EdgeList &getEdges() const { return edges_; }

  void clearNodes() { nodes_.clear(); }

  void addNode(Node *node) {
    nodes_.push_back(node);
  }

  void removeNode(Node *node) {
    nodes_.remove(node);
  }

  void addEdge(Edge *edge) {
    edges_.push_back(edge);
  }

  void removeEdge(Edge *edge) {
    edges_.remove(edge);
  }

  void setProcessed(bool processed) { processed_ = processed; }
  bool getProcessed() { return processed_; }

  void setVisited(bool visited) { visited_ = visited; }
  bool getVisited() { return visited_; }

  void print(std::ostream &os) const {
    os << data_;
  }

  friend std::ostream &operator<<(std::ostream &os, const Node &node) {
    node.print(os);

    return os;
  }

 private:
  DATA     data_;
  NodeList nodes_;
  EdgeList edges_;
  bool     processed_;
  bool     visited_;
};

template<class DATA, class COST>
class CGraphEdge {
  typedef CGraphNode<DATA,COST> Node;
  typedef CGraphEdge<DATA,COST> Edge;

 public:
  CGraphEdge(Node *node1, Node *node2, COST cost) :
   node1_(node1), node2_(node2), cost_(cost) { }

  Node *getNode1() const { return node1_; }
  Node *getNode2() const { return node2_; }

  COST getCost() const { return cost_; }

  void setProcessed(bool processed) { processed_ = processed; }
  bool getProcessed() { return processed_; }

  void setVisited(bool visited) { visited_ = visited; }
  bool getVisited() { return visited_; }

  Node *getOtherNode(Node *node) const {
    assert(node == node1_ || node == node2_);

    if (node1_ == node)
      return node2_;
    else
      return node1_;
  }

  void print(std::ostream &os) const {
    os << *node1_ << "->" << *node2_;
  }

  friend std::ostream &operator<<(std::ostream &os, const Edge &edge) {
    edge.print(os);

    return os;
  }

 private:
  Node *node1_;
  Node *node2_;
  COST  cost_;
  bool  processed_;
  bool  visited_;
};

#endif
