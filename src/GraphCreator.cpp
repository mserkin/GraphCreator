//============================================================================
// Name        : GraphCreator.cpp
// Author      : Mikhail Serkin
// Version     :
// Copyright   : Released under MIT license
// Description : Graph search algorithms
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

const string VERSION = "1.15.0.22";

struct UserContex {
	Settings* SettingsPtr = nullptr;
	Graph* GraphPtr = nullptr;
	Vertex* SourceVertex = nullptr;
	Vertex* TargetVertex = nullptr;
	int Checked = 0;
	int Processed = 0;
	int SourceProcessingTimes = 0;
	UserContex (Settings* settings, Graph* graph, Vertex* source, Vertex* target): SettingsPtr(settings), GraphPtr(graph), SourceVertex(source), TargetVertex(target) {};
};

static

void printGraph(Graph& graph) {
	for (auto& pair: graph) {
		cout << pair.first << "->";
		for (auto& edge: *pair.second->OutcomingEdges) {
			cout << edge->ToVertex->Name << "; ";
		}
		cout << endl;
	}
}

void handleAlgorithmEvent(AlgoEvent event, Vertex* vertex, void* user_context){
	UserContex* alg_context = static_cast<UserContex*>(user_context);
	bool verbose = alg_context->SettingsPtr->Verbose;
	switch(event) {
	case AlgoEvent::VertexDiscovered:
		alg_context->Checked++;
		if (verbose) cout << "\t\tvertex discovered: " << vertex->Name << endl;
		break;
	case AlgoEvent::VertexProcessingStarted:
		alg_context->Processed++;
		if (verbose) cout << "\tprocessing started: " << vertex->Name << endl;
		if (alg_context->SettingsPtr->SearchAlgorithm == Algorithm::BellmanFord && vertex == alg_context->SourceVertex) {
			cout << "Iteration " << ++alg_context->SourceProcessingTimes << " of " << alg_context->GraphPtr->size() + 1 << endl;
		}
		break;
	case AlgoEvent::VertexProcessingFinished:
		if (verbose) cout << "\tprocessing finished: " << vertex->Name << endl;
		break;
	case AlgoEvent::TargetFound:
		cout << "target found: " << vertex->Name << endl;
		break;
	case AlgoEvent::TargetNotFound:
		cout << "target not found. " << endl;
		break;
	case AlgoEvent::AlgorithmFinished:
		if (alg_context->SettingsPtr->SearchAlgorithm != Algorithm::BellmanFord) {
			cout << "Vertices checked: " << alg_context->Checked << ", processed: "
					<< alg_context->Processed << endl;
		}
		break;
	case AlgoEvent::NegativeLoopDetected:
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
	if (settings.SearchAlgorithm == Algorithm::None) return;
	Vertex *source = findVertex(settings.SourceVertex, graph);
	Vertex *target = findVertex(settings.TargetVertex, graph);
	UserContex user_context(&settings, &graph, source, target);
	AlgoResult result;
	BidirectionalDijkstraResult fast_dijkstra_result;

	switch (settings.SearchAlgorithm) {
	case Algorithm::BreadthFirstSearch: {
		cout << "Applying breadth-first search..." << endl;
		bfs(source, target, handleAlgorithmEvent, result, &user_context);
		break;
	}
	case Algorithm::DepthFirstSearch: {
		cout << "Applying depth-first search..." << endl;
		dfs(source, target, handleAlgorithmEvent, result, &user_context);
		break;
	}
	case Algorithm::Dijkstra: {
		cout << "Applying Dijkstra minimal weight path search..." << endl;
		dijkstra(source, target, graph, handleAlgorithmEvent, result, &user_context);
		break;
	}
	case Algorithm::BellmanFord: {
		cout << "Applying Bellman-Ford minimal weight path search..." << endl;
		bellmanFord(source, target, graph, handleAlgorithmEvent, result, &user_context);
		if (result.ResultCode == AlgoResultCode::Found || result.ResultCode == AlgoResultCode::NotFound) {
			printPathsToAllVertices(source, graph);
		}
		break;
	}
	case Algorithm::FastDijkstra:
		cout << "Applying Bidirectional Dijkstra minimal weight path search..." << endl;
		bidirectionalDijkstra(source, target, graph, handleAlgorithmEvent, fast_dijkstra_result, &user_context);
		result = fast_dijkstra_result;
		break;
	case Algorithm::Dijkstra2D: {
		cout << "Applying Dijkstra2D minimal weight path search..." << endl;
		dijkstra2d(static_cast<Vertex2d*>(source), static_cast<Vertex2d*>(target), graph, handleAlgorithmEvent, result, &user_context);
		break;
	}
	default: return;
	}

	switch (result.ResultCode) {
	case AlgoResultCode::NoSourceOrTarget:
		cout << "\tSource or target vertices are not defined." << endl;
		break;
	case AlgoResultCode::SourceIsTarget:
		cout << "\tSource and target are the same vertex." << endl;
		break;
	case AlgoResultCode::NotFound:
		cout << "A path from source to target has not been found." << endl;
		break;
	case AlgoResultCode::NegativeLoopFound:
		cout << "Negative loop was detected and algorithm execution stopped." << endl;
		break;
	case AlgoResultCode::Found: {
		cout << "The path from source to target has been found: " << endl;

		Vertex *v = target;
		if (settings.SearchAlgorithm == Algorithm::FastDijkstra) {
			v = fast_dijkstra_result.ForwardSearchLastVertex;
		}
		stack<Vertex*> path;
		path.push(v);
		while (v != source) {
			switch(settings.SearchAlgorithm) {
			case Algorithm::Dijkstra:
			case Algorithm::Dijkstra2D:
			case Algorithm::BellmanFord:
				v = static_cast<DijkstraContext*>(v->Context)->Parent;
				break;
			case Algorithm::BreadthFirstSearch:
			case Algorithm::DepthFirstSearch:
				v = static_cast<Vertex*>(v->Context);
				break;
			case Algorithm::FastDijkstra:
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

		if (settings.SearchAlgorithm == Algorithm::FastDijkstra) {
			v = fast_dijkstra_result.BackwardSearchLastVertex;
			cout << v->Name << ";";
			while (v != target) {
				v = static_cast<BidirectionalDijkstraContext*>(v->Context)->ParentInBackwardSearch;
				cout << v->Name << ";";
			}
		}

		cout << "\n\tShortest path weight: ";
		switch(settings.SearchAlgorithm) {
		case Algorithm::Dijkstra:
		case Algorithm::BellmanFord:
			cout << static_cast<DijkstraContext*>(target->Context)->Weight;
			break;
		case Algorithm::Dijkstra2D:
		case Algorithm::BreadthFirstSearch:
		case Algorithm::DepthFirstSearch:
			cout << "unknown (used algorithm is not capable to detect path of lowest weight)\n";
			break;
		case Algorithm::FastDijkstra:
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

void printError(int err) {
	switch (err) {
	case FATAL_ERROR_FILE_OPEN_FAILURE:
		cerr << "Cannot load graph from file.\n";
		break;
	case FATAL_ERROR_NO_MEMORY:
		cerr << "Memory allocation error.\n";
		break;
	case FATAL_ERROR_FILE_READ_FAILURE:
		cerr << "Failed to read file. Possible reason: the root element is not array.\n";
		break;
	case FATAL_ERROR_NAME_ELEMENT_NOT_FOUND:
		cerr << "Invalid file format. \"name\" element is expected in object, but not found\n";
		break;
	case FATAL_ERROR_FAILED_TO_ADD_VERTEX:
		cerr << "Failed to add vertex.\n";
		break;
	case FATAL_ERROR_FROM_VERTEX_NOT_FOUND:
		cerr << "Edge to unknown vertex found.\n";
		break;
	case FATAL_ERROR_TO_VERTEX_NOT_FOUND:
		cerr << "Edge from unknown vertex found.\n";
		break;
	case FATAL_ERROR_FAILED_TO_ADD_EDGE:
		cerr << "Cannot add edge.\n";
		break;
	case FATAL_ERROR_2D_FILE_CONTAINS_SHORT_LINE:
		cerr << "Error: not enough characters in line or illegal character found\n";
		break;
	default:
		cerr << "Unknown error\n";
		break;
	}
}

int main(int argc, char **argv) {
	srand(997);   //time(0));

	Settings settings;

	int error;
	if (NO_ERROR != (error = settings.parse(argc, argv))) {
		exit(error);
	}

	Graph graph;

	cout << " GraphCreator [OPTIONS]\n";
	if (settings.Verbose) settings.print();

	if (settings.LoadFromFile)	{
		if (settings.FilePath.rfind(".2d") != string::npos) {
			int err = load2dGraph(graph, settings);
			if (err < 0) {
				cout << "Error: Failed loading 2d graph\n";
				printError(err);
				exit(err);
			}
		}
		else if (settings.FilePath.rfind(".json") != string::npos) {
			int err = loadGraph(graph, settings);
			if (err != NO_ERROR) {
				cerr << "Error: Failed loading graph\n";
				printError(err);
				exit(err);
			}
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
		if (settings.FilePath.rfind(".2d") != string::npos) {
			save2dGraph(graph, settings);
		}
		else if (settings.FilePath.rfind(".json") != string::npos) {
			saveGraph(graph, settings);
		}
		else {
			cout << "Error: Unknown file type!\n";
			exit(FATAL_ERROR_UNKNOWN_FILE_TYPE);
		}
	}
	cout << endl;
	return 0;
}
