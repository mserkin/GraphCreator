//============================================================================
// Name        : GraphCreator.cpp
// Author      : Mikhail Serkin
// Version     :
// Copyright   : Released under MIT license
// Description : Hello World in C++, Ansi-style
//============================================================================


#include <ctime>
#include <iostream>
#include <vector>
#include <stack>
#include <cstdlib>
#include <stdlib.h>
#include <stdio.h>
#include "error.h"
#include "settings.h"
#include "graph.h"
#include "algo.h"
#include "informed.h"
#include "loadsave.h"
#include "creator.h"

using namespace std;

const string VERSION = "1.11.0.17";

struct UserContex {
	Settings* SettingsPtr = nullptr;
	Graph* GraphPtr = nullptr;
	Vertex* SourceVertex = nullptr;
	Vertex* TargetVertex = nullptr;

	UserContex (Settings* settings, Graph* graph, Vertex* source, Vertex* target): SettingsPtr(settings), GraphPtr(graph), SourceVertex(source), TargetVertex(target) {};
};

void printGraph(Graph& graph) {
	for (auto itv = graph.begin(); itv != graph.end(); itv++) {
		cout << (*itv).first << "->";
		for (auto ite = itv->second->OutcomingEdges->begin(); ite != itv->second->OutcomingEdges->end(); ite++) {
			cout << (*ite)->ToVertex->Name << "; ";
		}
		cout << endl;
	}
}

void handleAlgorithmEvent(AlgoEvent event, Vertex* vertex, void* user_context){
	static int checked = 0, processed = 0, source_processing_times = 0;
	UserContex* alg_context = static_cast<UserContex*>(user_context);
	bool verbose = alg_context->SettingsPtr->Verbose;
	switch(event) {
	case VertexDiscovered:
		checked++;
		if (verbose) cout << "\t\tvertex discovered: " << vertex->Name << endl;
		break;
	case VertexProcessingStarted:
		processed++;
		if (verbose) cout << "\tprocessing started: " << vertex->Name << endl;
		if (alg_context->SettingsPtr->Algorithm == BellmanFord && vertex == alg_context->SourceVertex) {
			cout << "Iteration " << ++source_processing_times << " of " << alg_context->GraphPtr->size() + 1 << endl;
		}
		break;
	case VertexProcessingFinished:
		if (verbose) cout << "\tprocessing finished: " << vertex->Name << endl;
		break;
	case TargetFound:
		cout << "target found: " << vertex->Name << endl;
		break;
	case TargetNotFound:
		cout << "target not found. " << endl;
		break;
	case AlgorithmFinished:
		if (alg_context->SettingsPtr->Algorithm != BellmanFord) cout << "Vertices checked: " << checked << ", processed: " << processed << endl;
		break;
	case NegativeLoopDetected:
		cout << "Negative loop detected at " << vertex->Name << endl;
	}
}

void printPathsToAllVertices(Vertex* source, Graph& graph) {
	stack<Vertex*> st;
	for (auto &pair: graph) {
		Vertex* current_vertex = pair.second;
		DijkstraContext* context = static_cast<DijkstraContext*>(current_vertex->Context);
		if (context->Weight == INFINITE_WEIGHT) {
			cout << current_vertex->Name << ": no path from source to this vertex\n";
			continue;
		}
		Vertex *vertex = current_vertex;
		while (vertex != source) {
			st.push(vertex);
			vertex = static_cast<DijkstraContext*>(vertex->Context)->Parent;
		}
		cout << source->Name;
		while (!st.empty()) {
			cout << "->" << st.top()->Name;
			st.pop();
		}
		cout << " weight: " << static_cast<DijkstraContext*>(current_vertex->Context)->Weight << "\n";
	}
}

void applyAlgo(Graph& graph, Settings &settings) {
	if (settings.Algorithm == None) return;
	Vertex *source = findVertex(settings.SourceVertex, graph);
	Vertex *target = findVertex(settings.TargetVertex, graph);
	UserContex user_context(&settings, &graph, source, target);
	AlgoResult result;
	BidirectionalDijkstraResult fast_dijkstra_result;

	switch (settings.Algorithm) {
	case BreadthFirstSearch: {
		cout << "Applying breadth-first search..." << endl;
		bfs(source, target, handleAlgorithmEvent, result, &user_context);
		break;
	}
	case DepthFirstSearch: {
		cout << "Applying depth-first search..." << endl;
		dfs(source, target, handleAlgorithmEvent, result, &user_context);
		break;
	}
	case Dijkstra: {
		cout << "Applying Dijkstra minimal weight path search..." << endl;
		dijkstra(source, target, graph, handleAlgorithmEvent, result, &user_context);
		break;
	}
	case BellmanFord: {
		cout << "Applying Bellman-Ford minimal weight path search..." << endl;
		bellmanFord(source, target, graph, handleAlgorithmEvent, result, &user_context);
		if (result.ResultCode == Found || result.ResultCode == NotFound) {
			printPathsToAllVertices(source, graph);
		}
		break;
	}
	case FastDijkstra:
		cout << "Applying Bidirectional Dijkstra minimal weight path search..." << endl;
		bidirectionalDijkstra(source, target, graph, handleAlgorithmEvent, fast_dijkstra_result, &user_context);
		result = fast_dijkstra_result;
		break;
	case Dijkstra2D: {
		cout << "Applying Dijkstra2D minimal weight path search..." << endl;
		dijkstra2d(static_cast<Vertex2d*>(source), static_cast<Vertex2d*>(target), graph, handleAlgorithmEvent, result, &user_context);
		break;
	}
	default: return;
	}

	switch (result.ResultCode) {
	case NoSourceOrTarget:
		cout << "\tSource or target vertices are not defined." << endl;
		break;
	case SourceIsTarget:
		cout << "\tSource and target are the same vertex." << endl;
		break;
	case NotFound:
		cout << "A path from source to target has not been found." << endl;
		break;
	case NegativeLoopFound:
		cout << "Negative loop was detected and algorithm execution stopped." << endl;
		break;
	case Found: {
		cout << "The path from source to target has been found: " << endl;

		Vertex *v = target;
		if (settings.Algorithm == FastDijkstra) v = fast_dijkstra_result.ForwardSearchLastVertex;
		stack<Vertex*> path;
		path.push(v);
		while (v != source) {
			switch(settings.Algorithm) {
			case Dijkstra:
			case Dijkstra2D:
			case BellmanFord:
				v = static_cast<DijkstraContext*>(v->Context)->Parent;
				break;
			case BreadthFirstSearch:
			case DepthFirstSearch:
				v = static_cast<Vertex*>(v->Context);
				break;
			case FastDijkstra:
				v = static_cast<BidirectionalDijkstraContext*>(v->Context)->ParentInForwardSearch;
				break;
			default:
				break;
			}
			path.push(v);
		}

		cout << "\t";
		while (!path.empty()) {
			cout << path.top()->Name << "; ";
			path.pop();
		}

		if (settings.Algorithm == FastDijkstra) {
			v = fast_dijkstra_result.BackwardSearchLastVertex;
			cout << v->Name << ";";
			while (v != target) {
				v = static_cast<BidirectionalDijkstraContext*>(v->Context)->ParentInBackwardSearch;
				cout << v->Name << ";";
			}
		}

		cout << "\n\tShortest path weight: ";
		switch(settings.Algorithm) {
		case Dijkstra:
		case Dijkstra2D:
		case BellmanFord:
			cout << static_cast<DijkstraContext*>(target->Context)->Weight;
			break;
		case BreadthFirstSearch:
		case DepthFirstSearch:
			cout << "unknown (used algorithm is not capable to detect path of lowest weight)\n";
			break;
		case FastDijkstra:
			cout << static_cast<BidirectionalDijkstraContext*>(fast_dijkstra_result.ForwardSearchLastVertex->Context)->WeightInForwardSearch +
				static_cast<BidirectionalDijkstraContext*>(fast_dijkstra_result.BackwardSearchLastVertex->Context)->WeightInBackwardSearch +
				fast_dijkstra_result.ConnectingEdgeWeight << "\n";
			break;
		default:
			break;
		}
		break;
	}
	default: break;
	}
}

int main(int argc, char **argv) {
	srand(997);   //time(0));

	Settings settings;

	settings.parse(argc, argv);

	Graph graph;

	cout << " GraphCreator [OPTIONS]\n";
	if (settings.Verbose) settings.print();

	if (settings.LoadFromFile)	{
		if (settings.FilePath.rfind(".2d") != string::npos) {
			int err = load2dGraph(graph, settings);
			if (err < 0) {
				cout << "Error: Failed loading 2d graph\n";
				exit(err);
			}
		}
		else if (settings.FilePath.rfind(".json") != string::npos) {
			loadGraph(graph, settings);
		}
		else {
			cout << "Error: Unknown file type!\n";
			exit(FATAL_ERROR_UNKNOWN_FILE_TYPE);
		}
	}
	else {
		createGraph(graph, settings);
	}

	if (settings.Verbose) printGraph(graph);

	applyAlgo(graph, settings);

	if (settings.SaveToFile) {
		saveGraph(graph, settings);
	}
	cout << endl;
	return 0;
}
