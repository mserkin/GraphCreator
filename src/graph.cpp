/*
 * graph.cpp
 *
 *  Created on: 3 џэт. 2020 у.
 *      Author: Serkin
 */
#include <vector>
#include "graph.h"


Edge* addEdge (Vertex *from, Vertex *to, const double weight, Graph &graph, const Settings& settings) {
	if (!from || !to) return nullptr;
	if (indexOfVertex(from, graph) < 0 || indexOfVertex(to, graph) < 0) return nullptr;
	if (from == to && !settings.SelfLoop)  return nullptr;
	if (!settings.BiDirectional) {
		for (auto it = to->Edges->begin(); it != to->Edges->end(); it++) {
			if ((*it)->ToVertex == from) return nullptr;
		}
	}
	if ((settings.MinEdgeWeight != 0.0 || settings.MaxEdgeWeight != 0.0)
			&& (weight < settings.MinEdgeWeight || weight > settings.MaxEdgeWeight)) {
		return nullptr;
	}

	Edge *edge = new Edge(from, to, weight);
	from->Edges->push_back(edge);
	return edge;
}

bool addVertex (Vertex *vertex, Graph &graph, const Settings& settings) {
	if (findVertex(vertex->Name, graph)) {
		return false;
	}
	for (auto ite = vertex->Edges->begin(); ite != vertex->Edges->end(); ite++) {
		if (indexOfVertex((*ite)->ToVertex, graph) < 0) return false;
		if ((*ite)->ToVertex == (*ite)->FromVertex && !settings.SelfLoop) return false;
		if ((*ite)->Weight > settings.MaxEdgeWeight || (*ite)->Weight < settings.MinEdgeWeight) return false;
	}
	graph.push_back(vertex);
	return true;
}

Vertex* addVertex (const string &name, Graph &graph, const Settings& settings) {
	Vertex *vertex = new Vertex (name, new EdgeList());
	if (addVertex(vertex, graph, settings)) {
		return vertex;
	}
	else {
		return nullptr;
	}
}

Vertex* findVertex(const string &name, const Graph &graph) {
	for (auto it = graph.begin(); it != graph.end(); it++) {
		if (!(*it)->Name.compare(name)) {
			return (*it);
		}
	}
	return nullptr;
}

int indexOfVertex(const Vertex *vertex, const Graph &graph) {
	size_t i = 0;
	for (auto it = graph.begin(); it != graph.end(); it++) {
		if (*it == vertex) return i;
		i++;
	}
	return -1;
}
