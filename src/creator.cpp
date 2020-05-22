/*
 * creator.cpp
 *
 *  Created on: 23 мая 2020 г.
 *      Author: Serkin
 */

#include <iostream>
#include "graph.h"
#include "settings.h"
#include "error.h"

int chooseToVertex(Vertex *vertex_from, const Settings &settings, Graph &graph,
		int &error_count) {
	int vertex_ind = rand() % settings.VertexCount;
	Vertex *vertex_to = graph[vertex_ind];
	if (!settings.SelfLoop && vertex_to == vertex_from) {
		error_count++;
		return FATAL_ERROR_SELF_LOOPS_ARE_PROHIBITED;
	}
	if (!settings.BiDirectional) {
		for (auto itb = vertex_to->OutcomingEdges->begin(); itb != vertex_to->OutcomingEdges->end(); itb++) {
			if ((*itb)->ToVertex == vertex_from) {
				error_count++;
				return FATAL_ERROR_BIDIRECTION_EDGES_ARE_PROHIBITED;
			}
		}
	}
	for (auto ite = vertex_from->OutcomingEdges->begin(); ite != vertex_from->OutcomingEdges->end(); ite++) {
		if ((*ite)->ToVertex == vertex_to) {
			error_count++;
			return FATAL_ERROR_EDGE_ALREADY_EXISTS;
		}
	}
	return vertex_ind;
}


void createGraph(Graph& graph, const Settings& settings) {
	int error_count = 0;
	int last_error = 0;
	int total_edge_num = settings.MaxEdgeCount;
	for (int i = 0; i < settings.VertexCount; i++) {
		char *name = new char[4]; name[0] = 'V';
		sprintf(name+1, "%X", i);
		//itoa(i, name+1, 36);
		Vertex *vertex = addVertex(name, graph, settings);
		if (vertex == nullptr) {
			cerr << "Cannot add vertex to graph.";
			exit(FATAL_ERROR_UNKNOWN_FILE_TYPE);
		}
	}

	while (total_edge_num > 0 && error_count < 1000) {
		for (auto it = graph.begin(); it != graph.end(); it++) {
			int ver_edges = rand() % (settings.MaxEdgeCount / settings.VertexCount) + 1;
			Vertex* pvert = *it;
			cout << endl << "\tvertex: " << pvert->Name << "\n\tto create edges:" << ver_edges << endl;
			while (ver_edges > 0 && total_edge_num > 0 && error_count < 1000) {
				int vertex_ind = chooseToVertex(*it, settings, graph, error_count);
				if (vertex_ind < 0) {
					last_error = vertex_ind;
					cout << "Error creating edge for vertex " << static_cast<Vertex*>(*it)->Name << ":";
					switch(vertex_ind) {
					case FATAL_ERROR_SELF_LOOPS_ARE_PROHIBITED:
						cout << " self-loops are not allowed by settings" << endl;
						break;
					case FATAL_ERROR_BIDIRECTION_EDGES_ARE_PROHIBITED:
						cout << " bidirectional edges are not allowed by settings" << endl;
						break;
					case FATAL_ERROR_EDGE_ALREADY_EXISTS:
						cout << " such edge already exists" << endl;
						break;
					}
					error_count++;
					continue;
				}
				double weight = rand() % (settings.MaxEdgeWeight - settings.MinEdgeWeight + 1) - settings.MinEdgeWeight;
				Edge* edge = addEdge(*it, graph[vertex_ind], weight, graph, settings);
				if (!edge) {
					last_error = FATAL_ERROR_FAILED_TO_ADD_EDGE;
					error_count++;
					continue;
				}
				ver_edges--;
				total_edge_num--;
			}
		}
	}
	cout << "Error count: " << error_count << "\nLast error: " << last_error << endl;
}

