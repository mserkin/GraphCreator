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
		for (auto it = to->OutcomingEdges->begin(); it != to->OutcomingEdges->end(); it++) {
			if ((*it)->ToVertex == from) return nullptr;
		}
	}
	if ((settings.MinEdgeWeight != 0.0 || settings.MaxEdgeWeight != 0.0)
			&& (weight < settings.MinEdgeWeight || weight > settings.MaxEdgeWeight)) {
		return nullptr;
	}

	Edge *edge = new Edge(from, to, weight);
	from->OutcomingEdges->push_back(edge);
	to->IncomingEdges->push_back(edge);
	return edge;
}

bool addVertex (Vertex *vertex, Graph &graph, const Settings& settings) {
	if (findVertex(vertex->Name, graph)) {
		return false;
	}
	for (auto ite = vertex->OutcomingEdges->begin(); ite != vertex->OutcomingEdges->end(); ite++) {
		if (indexOfVertex((*ite)->ToVertex, graph) < 0) return false;
		if ((*ite)->ToVertex == (*ite)->FromVertex && !settings.SelfLoop) return false;
		if ((*ite)->Weight > settings.MaxEdgeWeight || (*ite)->Weight < settings.MinEdgeWeight) return false;
	}
	graph.push_back(vertex);
	return true;
}

Vertex* addVertex (const string &name, Graph &graph, const Settings& settings) {
	Vertex *vertex = new Vertex (name);
	bool success = addVertex(vertex, graph, settings);
	if (success) {
		return vertex;
	}
	else {
		return nullptr;
	}
}

void removeVertex (Vertex **vertex, Graph &graph) {
	PVertex pvertex = (*vertex);
	PEdge pedge;
	while (!pvertex->OutcomingEdges->empty()) {
		//take last
		auto it1 = pvertex->OutcomingEdges->end();
		pedge = *(--it1);
		//remove last
		pvertex->OutcomingEdges->pop_back();
		//find and remove edge from incomings
		for (it1 = pedge->ToVertex->IncomingEdges->begin(); it1 !=pedge->ToVertex->IncomingEdges->end(); it1++) {
			if ((*it1)->FromVertex == pvertex) {
				pedge->ToVertex->IncomingEdges->erase(it1);
				break;
			}
		}
		delete pedge;
	}
	while (!pvertex->IncomingEdges->empty()) {
		//take last
		auto it2 = pvertex->IncomingEdges->end();
		pedge = *(--it2);
		//remove last
		pvertex->IncomingEdges->pop_back();
		//find edge in outcomings
		for (it2 = pedge->FromVertex->OutcomingEdges->begin(); it2 !=pedge->FromVertex->OutcomingEdges->end(); it2++) {
			if ((*it2)->ToVertex == pvertex) {
				//Erase it :)
				pedge->FromVertex->OutcomingEdges->erase(it2);
				break;
			}
		}
		delete pedge;
	}

	for (auto it3 = graph.begin(); it3 != graph.end(); it3++) {
		if (!(*it3)->Name.compare(pvertex->Name)) {
			graph.erase(it3);
		}
	}

	delete pvertex;
	vertex = nullptr;
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
