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
#include <map>
#include "settings.h"
#include "types.h"

using namespace std;

//Add edge from one given vertex to another given vertex
Edge* addEdge (Vertex *from, Vertex *to, const double weight, Graph &graph, const Settings& settings);

//Add vertex to a graph
bool addVertex (Vertex *vertex, Graph &graph, const Settings& settings);

//Create vertex and add it to a graph
Vertex* addVertex (const string &name, Graph &graph, const Settings& settings);

//Remove vertex from the graph
//All context information should be previously deleted by calling clearContext()
void removeVertex (Vertex **vertex, Graph &graph);

//find vertex with the given name in the graph
Vertex* findVertex(const string &name, const Graph &graph);

//Makes a clone of the graph
//vertices and edge are cloned but not a context
//graph - graph to make clone from
//returns new graph which is a clone of given
//Graph is allocated with new expression.
Graph& cloneGraph(const Graph &graph);

//Deletes all vertices from graph
//All context information should be previously deleted by calling clearContext()
void clearGraph(Graph &graph);

#endif /* INC_GRAPH_H_ */
