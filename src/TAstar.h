#ifndef TASTAR_H
#define TASTAR_H

#include <boost/graph/astar_search.hpp>
#include <boost/graph/use_mpi.hpp>
#include <boost/graph/distributed/adjacency_list.hpp>
#include <boost/graph/distributed/mpi_process_group.hpp>
#include <boost/graph/random.hpp>
#include <boost/random.hpp>
#include <boost/graph/graphviz.hpp>
//#include <sys/time.h>
#include <vector>
#include <list>
#include <iostream>
#include <fstream>
#include <math.h>    // for sqrt
#include <QDebug>

using namespace boost;
using namespace std;


// auxiliary types
struct location
{
  float y, x, z; // lat, long
  int id, area;
};
typedef float cost;

// euclidean distance heuristic
template <class Graph, class CostType, class LocMap>
class distance_heuristic : public astar_heuristic<Graph, CostType>
{
public:
  typedef typename graph_traits<Graph>::vertex_descriptor Vertex;
  distance_heuristic(LocMap l, Vertex goal)
    : m_location(l), m_goal(goal) {}
  CostType operator()(Vertex u)
  {
      return 1;
      if (m_location[m_goal].area != m_location[u].area)
          return 1;
      CostType dx = m_location[m_goal].x - m_location[u].x;
      CostType dy = m_location[m_goal].y - m_location[u].y;
      //qDebug() << "dx" << dx << "dy" << dy << "\n";
      CostType dz = m_location[m_goal].z - m_location[u].z;
      return ::sqrt(dx * dx + dy * dy + dz * dz);
  }
private:
  LocMap m_location;
  Vertex m_goal;
};


struct found_goal {}; // exception for termination

// visitor that terminates when we find the goal
template <class Vertex>
class astar_goal_visitor : public boost::default_astar_visitor
{
public:
  astar_goal_visitor(Vertex goal) : m_goal(goal) {}
  template <class Graph>
  void examine_vertex(Vertex u, Graph& g) {
    if(u == m_goal)
      throw found_goal();
  }
private:
  Vertex m_goal;
};

#endif // TASTAR_H
