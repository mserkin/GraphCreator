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

//Add edge from one given vertex to another given vertex
Edge* addEdge (Vertex *from, Vertex *to, const double weight, Graph &graph, const Settings& settings);

//Add vertex to a graph
bool addVertex (Vertex *vertex, Graph &graph, const Settings& settings);

//Create vertex and add it to a graph
Vertex* addVertex (const string &name, Graph &graph, const Settings& settings);

//Remove vertex from the graph
void removeVertex (Vertex **vertex, Graph &graph);

//find vertex with the given name in the graph
Vertex* findVertex(const string &name, const Graph &graph);

//find the given vertex in the graph and return its index
int indexOfVertex(const Vertex *vertex, const Graph &graph);



#endif /* INC_GRAPH_H_ */
