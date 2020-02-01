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

const string VERSION = "1.6.0.9";

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
		return -1;
	}
	if (!settings.BiDirectional) {
		for (auto itb = vertex_to->Edges->begin(); itb != vertex_to->Edges->end(); itb++) {
			if ((*itb)->ToVertex == vertex_from) {
				error_count++;
				return -1;
			}
		}
	}
	for (auto ite = vertex_from->Edges->begin(); ite != vertex_from->Edges->end(); ite++) {
		if ((*ite)->ToVertex == vertex_to) {
			error_count++;
			return -1;
		}
	}
	return vertex_ind;
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
		if (vertex == alg_context->SourceVertex) {
			cout << "Iteration " << ++source_processing_times << " of " << alg_context->GraphPtr->size() + 1 << endl;
		}
		break;
	case VertexProcessingFinished:
		if (verbose) cout << "\tprocessing finished: " << vertex->Name << endl;
		break;
	case TargetFound:
		if (alg_context->SettingsPtr->Algorithm != BellmanFord) cout << "target found: " << vertex->Name << endl;
		break;
	case TargetNotFound:
		if (alg_context->SettingsPtr->Algorithm != BellmanFord) cout << "target not found. " << endl;
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
	int total_edge_num = settings.MaxEdgeCount;
	for (int i = 0; i < settings.VertexCount; i++) {
		char *name = new char[4]; name[0] = 'V';
		sprintf(name+1, "%X", i);
		//itoa(i, name+1, 36);
		Vertex *vertex = addVertex(name, graph, settings);
		if (vertex == nullptr) {
			cerr << "Cannot add vertex to graph.";
			exit (-10);
		}
	}
	while (total_edge_num > 0 && error_count < 1000) {
		for (auto it = graph.begin(); it != graph.end(); it++) {
			int ver_edges = rand() % (settings.MaxEdgeCount / settings.VertexCount) + 1;
			while (ver_edges > 0 && total_edge_num > 0 && error_count < 1000) {
				int vertex_ind = chooseToVertex(*it, settings, graph, error_count);
				if (vertex_ind < 0) {
					cout << "Error creating edge for vertex " << static_cast<Vertex*>(*it)->Name << endl;
					ver_edges--;
					continue;
				}
				double weight = rand() % (settings.MaxEdgeWeight - settings.MinEdgeWeight + 1) - settings.MinEdgeWeight;
				addEdge(*it, graph[vertex_ind], weight, graph, settings);
				ver_edges--;
				total_edge_num--;
			}
		}
	}
	cout << "Error count: " << error_count << endl;
}

void loadGraph(Graph& graph, const Settings& settings) {
	FILE * fd = fopen(settings.FilePath.c_str(), "rb");

	if (!fd){
		cerr << "Cannot load graph from file: " << settings.FilePath;
		exit(-1);
	}

	fseek(fd, 0, SEEK_END);                          // устанавливаем позицию в конец файла
	long lSize = ftell(fd);                            // получаем размер в байтах
	rewind (fd);                                       // устанавливаем указатель в конец файла

	char * buffer = (char*) malloc(sizeof(char) * lSize + 1); // выделить память для хранения содержимого файла
	if (buffer == NULL)
	{
	  fputs("Memory allocation error.", stderr);
	  exit(-2);
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
	  exit (-3);
	}

	buffer[lSize] = 0;

	//содержимое файла теперь находится в буфере
	puts(buffer);

	// завершение работы
	fclose (fd);

	rapidjson::Document doc;
	doc.ParseInsitu(buffer);
	if (!doc.IsArray()) {
		fputs("Invalid file format. The root element is expected to be array.", stderr);
		exit(-4);
	}

	//creating all vertices
	for (rapidjson::Value::ConstValueIterator itr = doc.Begin(); itr != doc.End(); ++itr) {
	    auto vertex_obj1 = itr->GetObject();
		if (!vertex_obj1.HasMember("name")) {
			fputs("Invalid file format. \"name\" element is expected in object, but not found", stderr);
			exit(-5);
		}
		string *str1 = new string (vertex_obj1["name"].GetString());

		Vertex *vertex = addVertex(*str1, graph, settings);
		if (vertex == nullptr) {
			cerr << "Failed to add vertex.";
			exit(-8);
		}
	}

	//creating edges
	for (rapidjson::Value::ConstValueIterator itv = doc.Begin(); itv != doc.End(); ++itv) {
	    auto vertex_obj2 = itv->GetObject();
	    string *str_from = new string (vertex_obj2["name"].GetString());
	    Vertex* from_vertex = findVertex(*str_from, graph);

	    if (!from_vertex) {
	    	cerr << "Vertex not found: " << *str_from;
	    	exit(-6);
	    }

	    if (vertex_obj2.HasMember("edges")) {
			auto edges = vertex_obj2["edges"].GetArray();
			for (rapidjson::Value::ConstValueIterator ite = edges.Begin(); ite != edges.End(); ++ite) {
				auto edge_obj = ite->GetObject();
				string *str_to = new string(edge_obj["to_vertex"].GetString());
				Vertex* to_vertex = findVertex(*str_to, graph);
			    if (!to_vertex) {
			    	cerr << "Vertex not found: " << *str_to;
			    	exit(-7);
			    }
				double weight = edge_obj["weight"].GetDouble();
				if (!addEdge(from_vertex, to_vertex, weight, graph, settings)) {
					cerr << "Cannot add edge: " << *str_from << "->" << *str_to;
					exit(-8);
				}
			}
		}
	}
	free (buffer);
}

void saveGraph(Graph& graph, const Settings& settings) {
	rapidjson::Document doc;
	doc.SetArray();
	for (auto itv = graph.begin(); itv != graph.end(); itv++) {
		rapidjson::Value vert(rapidjson::kObjectType);
		rapidjson::Value name(rapidjson::kStringType);
		rapidjson::Value edges(rapidjson::kArrayType);
		for (auto ite = (*itv)->Edges->begin(); ite != (*itv)->Edges->end(); ite++)
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

void printGraph(Graph& graph) {
	for (auto itv = graph.begin(); itv != graph.end(); itv++) {
		cout << (*itv)->Name << "->";
		for (auto ite = (*itv)->Edges->begin(); ite != (*itv)->Edges->end(); ite++) {
			cout << (*ite)->ToVertex->Name << "; ";
		}
		cout << endl;
	}
}

void applyAlgo(Graph& graph, Settings &settings) {
	if (settings.Algorithm == None) return;
	Vertex *source = findVertex(settings.SourceVertex, graph);
	Vertex *target = findVertex(settings.TargetVertex, graph);
	UserContex user_context(&settings, &graph, source, target);
	AlgoResult result;

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
		bellman_ford(source, target, graph, handleAlgorithmEvent, result, &user_context);
		break;
	}

	default: return;
	}

	switch (result.ResultCode) {
	case NoSourceOrTarget:
		cerr << "\tSource or target vertices are not defined." << endl;
		break;
	case NotFound:
		cout << "A path from source to target has not been found." << endl;
		break;
	case Found: {
		cout << "The path from source to target has been found: " << endl;
		Vertex *v = target;
		stack<Vertex*> path;
		path.push(v);
		while (v != source) {
			if (settings.Algorithm == Dijkstra || settings.Algorithm == BellmanFord) {
				v = static_cast<DijkstraContext*>(v->Context)->Parent;
			}
			else {
				v = static_cast<Vertex*>(v->Context);
			}
			path.push(v);
		}
		cout << "\t";
		while (!path.empty()) {
			cout << path.top()->Name << ";";
			path.pop();
		}
		cout << endl;
		if (settings.Algorithm == Dijkstra || settings.Algorithm == BellmanFord) {
			cout << "\tShortest path weight: " << static_cast<DijkstraContext*>(target->Context)->Weight;
		}
		break;
	}
	default: break;
	}
}

int main(int argc, char **argv) {
	srand(time(0));

	Settings settings;

	settings.parse(argc, argv);

	Graph graph;

	cout << " GraphCreator [OPTIONS]\n";
	if (settings.Verbose) settings.print();

	if (settings.LoadFromFile)	{
		loadGraph(graph, settings);
	}
	else {
		createGraph(graph, settings);
	}

	if (settings.Verbose) printGraph(graph);

	applyAlgo(graph, settings);

	if (settings.SaveToFile) {
		saveGraph(graph, settings);
	}
	return 0;
}
