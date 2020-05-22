/*
 * graph.h
 *
 *  Created on: 3 џэт. 2020 у.
 *      Author: Serkin
 */

#ifndef INC_GRAPH_H_
#define INC_GRAPH_H_

#include <string>
#include <vector>
#include "settings.h"

using namespace std;

struct Edge;

typedef vector<Edge*> EdgeList;

struct Vertex {
	string Name;
	EdgeList *OutcomingEdges;
	EdgeList *IncomingEdges;
	void* Context = nullptr;
	Vertex(string _Name, EdgeList *_FromEdges, EdgeList *_ToEdges):
		Name(_Name), OutcomingEdges(_FromEdges), IncomingEdges(_ToEdges) {};
};
typedef Vertex* PVertex;

struct Edge {
	Vertex *FromVertex;
	Vertex *ToVertex;
	double Weight;
	Edge(Vertex* _FromVertex, Vertex* _ToVertex, double _Weight):
		FromVertex(_FromVertex), ToVertex(_ToVertex), Weight(_Weight) {};
};
typedef Edge* PEdge;
typedef vector<Vertex*> Graph;

Edge* addEdge (Vertex *from, Vertex *to, const double weight, Graph &graph, const Settings& settings);
bool addVertex (const Vertex *vertex, Graph &graph, const Settings& settings);
Vertex* addVertex (const string &name, Graph &graph, const Settings& settings);
void removeVertex (Vertex **vertex, Graph &graph);
Vertex* findVertex(const string &name, const Graph &graph);
int indexOfVertex(const Vertex *vertex, const Graph &graph);



#endif /* INC_GRAPH_H_ */
