//============================================================================
// Name        : GraphCreator.cpp
// Author      : Mikhail Serkin
// Version     :
// Copyright   : Released under MIT license
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <ctime>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <stack>
#include <cstdlib>
#include <stdlib.h>
#include <stdio.h>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/rapidjson.h"
#include "settings.h"
#include "graph.h"
#include "algo.h"

using namespace std;

const string VERSION = "1.7.0.11";

const int FATAL_ERROR_FILE_OPEN_FAILURE = -1;
const int FATAL_ERROR_NO_MEMORY = -2;
const int FATAL_ERROR_FILE_READ_FAILURE = -3;
const int FATAL_ERROR_ROOT_IS_NOT_ARRAY = -4;
const int FATAL_ERROR_NAME_ELEMENT_NOT_FOUND = -5;
const int FATAL_ERROR_FROM_VERTEX_NOT_FOUND = -6;
const int FATAL_ERROR_TO_VERTEX_NOT_FOUND = -7;
const int FATAL_ERROR_FAILED_TO_ADD_VERTEX = -8;
const int FATAL_ERROR_FAILED_TO_ADD_EDGE = -9;
const int FATAL_ERROR_UNKNOWN_FILE_TYPE = -10;
const int FATAL_ERROR_SELF_LOOPS_ARE_PROHIBITED = -101;
const int FATAL_ERROR_BIDIRECTION_EDGES_ARE_PROHIBITED = -102;
const int FATAL_ERROR_EDGE_ALREADY_EXISTS = -103;
const int FATAL_ERROR_NET_FILE_CONTAINS_SHORT_LINE = -201;

struct UserContex {
	Settings* SettingsPtr = nullptr;
	Graph* GraphPtr = nullptr;
	Vertex* SourceVertex = nullptr;
	Vertex* TargetVertex = nullptr;

	UserContex (Settings* settings, Graph* graph, Vertex* source, Vertex* target): SettingsPtr(settings), GraphPtr(graph), SourceVertex(source), TargetVertex(target) {};
};

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

void printGraph(Graph& graph) {
	for (auto itv = graph.begin(); itv != graph.end(); itv++) {
		cout << (*itv)->Name << "->";
		for (auto ite = (*itv)->OutcomingEdges->begin(); ite != (*itv)->OutcomingEdges->end(); ite++) {
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

void loadGraph(Graph& graph, const Settings& settings) {
	FILE * fd = fopen(settings.FilePath.c_str(), "rb");

	if (!fd){
		cerr << "Cannot load graph from file: " << settings.FilePath;
		exit(FATAL_ERROR_FILE_OPEN_FAILURE);
	}

	fseek(fd, 0, SEEK_END);                          // устанавливаем позицию в конец файла
	long lSize = ftell(fd);                            // получаем размер в байтах
	rewind (fd);                                       // устанавливаем указатель в конец файла

	char * buffer = (char*) malloc(sizeof(char) * lSize + 1); // выделить память для хранения содержимого файла
	if (buffer == NULL)
	{
	  fputs("Memory allocation error.", stderr);
	  exit(FATAL_ERROR_NO_MEMORY);
	}

	char *begin = buffer;
	int chunksize = 256;
	size_t result;
	do {
		result = fread(begin, 1, chunksize, fd);       // считываем файл в буфер
		begin += result;
	} while (result > 0);

	if (begin - buffer != lSize)
	{
	  fputs("Failed to read file", stderr);
		exit(FATAL_ERROR_FILE_READ_FAILURE);
	}

	buffer[lSize] = 0;

	//содержимое файла теперь находится в буфере
	if (settings.Verbose) puts(buffer);

	// завершение работы
	fclose (fd);

	rapidjson::Document doc;
	doc.ParseInsitu(buffer);
	if (!doc.IsArray()) {
		fputs("Invalid file format. The root element is expected to be array.", stderr);
		exit(FATAL_ERROR_FILE_READ_FAILURE);
	}

	//creating all vertices
	for (rapidjson::Value::ConstValueIterator itr = doc.Begin(); itr != doc.End(); ++itr) {
	    auto vertex_obj1 = itr->GetObject();
		if (!vertex_obj1.HasMember("name")) {
			fputs("Invalid file format. \"name\" element is expected in object, but not found", stderr);
			exit(FATAL_ERROR_NAME_ELEMENT_NOT_FOUND);
		}
		string *str1 = new string (vertex_obj1["name"].GetString());

		Vertex *vertex = addVertex(*str1, graph, settings);
		if (vertex == nullptr) {
			cerr << "Failed to add vertex.";
			exit(FATAL_ERROR_FAILED_TO_ADD_VERTEX);
		}
	}

	//creating edges
	for (rapidjson::Value::ConstValueIterator itv = doc.Begin(); itv != doc.End(); ++itv) {
	    auto vertex_obj2 = itv->GetObject();
	    string *str_from = new string (vertex_obj2["name"].GetString());
	    Vertex* from_vertex = findVertex(*str_from, graph);

	    if (!from_vertex) {
	    	cerr << "Vertex not found: " << *str_from;
			exit(FATAL_ERROR_FROM_VERTEX_NOT_FOUND);
	    }

	    if (vertex_obj2.HasMember("edges")) {
			auto edges = vertex_obj2["edges"].GetArray();
			for (rapidjson::Value::ConstValueIterator ite = edges.Begin(); ite != edges.End(); ++ite) {
				auto edge_obj = ite->GetObject();
				string *str_to = new string(edge_obj["to_vertex"].GetString());
				Vertex* to_vertex = findVertex(*str_to, graph);
			    if (!to_vertex) {
			    	cerr << "Vertex not found: " << *str_to;
					exit(FATAL_ERROR_TO_VERTEX_NOT_FOUND);
			    }
				double weight = edge_obj["weight"].GetDouble();
				if (!addEdge(from_vertex, to_vertex, weight, graph, settings)) {
					cerr << "Cannot add edge: " << *str_from << "->" << *str_to << endl;
					exit(FATAL_ERROR_FAILED_TO_ADD_EDGE);
				}
			}
		}
	}
	free (buffer);
}

int loadNetGraph(Graph& graph, const Settings& settings) {
	Vertex *vnet[9][9];
	for (unsigned char y1 = 0; y1 < 9; y1++) {
		for (unsigned char x1 = 0; x1 < 9; x1++) {
			string name {"0:0"};
			name[0] = 0x30 + x1;
			name[2] = 0x30 + y1;
			vnet[y1][x1] = addVertex(name, graph, settings);
		}
	}
	for (int y2 = 0; y2 < 9; y2++) {
		for (int x2 = 0; x2 < 9; x2++) {
			if (y2 > 0) {
				if (!addEdge(vnet[y2][x2], vnet[y2-1][x2], 1, graph, settings)){
					return FATAL_ERROR_FAILED_TO_ADD_EDGE;
				}
			}
			if (y2 < 8) {
				if (!addEdge(vnet[y2][x2], vnet[y2+1][x2], 1, graph, settings)) {
					return FATAL_ERROR_FAILED_TO_ADD_EDGE;
				}
			}
			if (x2 > 0) {
				if(!addEdge(vnet[y2][x2], vnet[y2][x2-1], 1, graph, settings)) {
					return FATAL_ERROR_FAILED_TO_ADD_EDGE;
				}
			}
			if (x2 < 8) {
				if (!addEdge(vnet[y2][x2], vnet[y2][x2+1], 1, graph, settings)) {
					return FATAL_ERROR_FAILED_TO_ADD_EDGE;
				}
			}
		}
	}

	ifstream file(settings.FilePath.c_str());
	string line;
	int y3 = 0;
	while(y3 < 9 && std::getline(file, line)){
		if (line.length() < 9) {
			cout << "Error: line is shorter than 9 charecters\n";
			exit(FATAL_ERROR_NET_FILE_CONTAINS_SHORT_LINE);
		}
		for (int x3 = 0; x3 < 9; x3++) {
			switch (line[x3]) {
			case '+':
				removeVertex(&vnet[y3][x3], graph);
				break;
			case '-':
				break;
			case 0x13:
			case 0x10:
				cout << "Error: line " << y3 << " is shorter than 9 characters\n";
				exit(FATAL_ERROR_NET_FILE_CONTAINS_SHORT_LINE);
			default:
				cout << "Error: line "<< y3 << " contains illegal character " << line[x3] << "(" << int(line[x3]) << ")\n";
				exit(FATAL_ERROR_NET_FILE_CONTAINS_SHORT_LINE);
			}
		}
		y3++;
	}
	return 0;
}

void saveGraph(Graph& graph, const Settings& settings) {
	rapidjson::Document doc;
	doc.SetArray();
	for (auto itv = graph.begin(); itv != graph.end(); itv++) {
		rapidjson::Value vert(rapidjson::kObjectType);
		rapidjson::Value name(rapidjson::kStringType);
		rapidjson::Value edges(rapidjson::kArrayType);
		for (auto ite = (*itv)->OutcomingEdges->begin(); ite != (*itv)->OutcomingEdges->end(); ite++)
		{
			rapidjson::Value edge(rapidjson::kObjectType);
			rapidjson::Value to_vertex(rapidjson::kStringType);
			to_vertex.SetString((*ite)->ToVertex->Name.c_str(), (*ite)->ToVertex->Name.length(), doc.GetAllocator());
			rapidjson::Value weight((*ite)->Weight);
			edge.AddMember ("to_vertex", to_vertex, doc.GetAllocator());
			edge.AddMember ("weight", weight, doc.GetAllocator());
			edges.PushBack(edge, doc.GetAllocator());
		}
		name.SetString((*itv)->Name.c_str(), (*itv)->Name.length(), doc.GetAllocator());
		vert.AddMember("name", name, doc.GetAllocator());
		vert.AddMember("edges", edges, doc.GetAllocator());
		doc.PushBack(vert, doc.GetAllocator());
	}
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer, rapidjson::Document::EncodingType, rapidjson::ASCII<> > writer(buffer);
	doc.Accept(writer);

	FILE *fd = fopen(settings.FilePath.c_str(), "w");
	fputs(buffer.GetString(), fd);
	fclose(fd);
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
		break;
	}
	case FastDijkstra:
		cout << "Applying Bidirectional Dijkstra minimal weight path search..." << endl;
		bidirectionalDijkstra(source, target, graph, handleAlgorithmEvent, fast_dijkstra_result, &user_context);
		result = fast_dijkstra_result;
		break;

	default: return;
	}

	switch (result.ResultCode) {
	case NoSourceOrTarget:
		cout << "\tSource or target vertices are not defined." << endl;
		break;
	case NotFound:
		cout << "A path from source to target has not been found." << endl;
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
			cout << path.top()->Name << ";";
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
		if (settings.FilePath.rfind(".net") != string::npos) {
			int err = loadNetGraph(graph, settings);
			if (err < 0) {
				cout << "Error: Failed loading net graph\n";
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
