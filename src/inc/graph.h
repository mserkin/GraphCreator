/*
 * graph.h
 *
 *  Created on: 3 ���. 2020 �.
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
	EdgeList *OutcomingEdges = nullptr;
	EdgeList *IncomingEdges = nullptr;
	void* Context = nullptr;
	Vertex(string _Name): Name(_Name) {
		IncomingEdges = new EdgeList();
		OutcomingEdges = new EdgeList();
	};
	virtual ~Vertex() {
		delete OutcomingEdges;
		delete IncomingEdges;
	}
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
bool addVertex (Vertex *vertex, Graph &graph, const Settings& settings);
Vertex* addVertex (const string &name, Graph &graph, const Settings& settings);
void removeVertex (Vertex **vertex, Graph &graph);
Vertex* findVertex(const string &name, const Graph &graph);
int indexOfVertex(const Vertex *vertex, const Graph &graph);



#endif /* INC_GRAPH_H_ */
