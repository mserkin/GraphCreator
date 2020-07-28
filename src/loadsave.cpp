/*
 * loadsave.cpp
 *
 *  Created on: 23 мая 2020 г.
 *      Author: Serkin
 */

#include <iostream>
#include <sstream>
#include <fstream>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/rapidjson.h"
#include "error.h"
#include "graph.h"
#include "settings.h"
#include "informed.h"

int loadGraph(Graph& graph, const Settings& settings) {
	using namespace std;

	FILE * fd = fopen(settings.FilePath.c_str(), "rb");

	if (!fd){
		return FATAL_ERROR_FILE_OPEN_FAILURE;
	}

	fseek(fd, 0, SEEK_END);                          // устанавливаем позицию в конец файла
	long lSize = ftell(fd);                            // получаем размер в байтах
	rewind (fd);                                       // устанавливаем указатель в конец файла

	char * buffer = (char*) malloc(sizeof(char) * lSize + 1); // выделить память для хранения содержимого файла
	if (buffer == NULL)
	{
		fclose(fd);
	    return FATAL_ERROR_NO_MEMORY;
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
		free (buffer);
		fclose (fd);
		return FATAL_ERROR_FILE_READ_FAILURE;
	}

	buffer[lSize] = 0;

	if (settings.Verbose) puts(buffer);

	fclose (fd);

	rapidjson::Document doc;
	doc.ParseInsitu(buffer);
	if (!doc.IsArray()) {
		free (buffer);
		return FATAL_ERROR_FILE_READ_FAILURE;
	}

	//creating all vertices
	for (rapidjson::Value::ConstValueIterator itr = doc.Begin(); itr != doc.End(); ++itr) {
	    auto vertex_obj1 = itr->GetObject();
		if (!vertex_obj1.HasMember("name")) {
			free (buffer);
			return FATAL_ERROR_NAME_ELEMENT_NOT_FOUND;
		}
		string *str1 = new string (vertex_obj1["name"].GetString());

		Vertex *vertex = addVertex(*str1, graph, settings);
		if (vertex == nullptr) {
			free (buffer);
			return FATAL_ERROR_FAILED_TO_ADD_VERTEX;
		}
	}

	//creating edges
	for (rapidjson::Value::ConstValueIterator itv = doc.Begin(); itv != doc.End(); ++itv) {
	    auto vertex_obj2 = itv->GetObject();
	    string *str_from = new string (vertex_obj2["name"].GetString());
	    Vertex* from_vertex = findVertex(*str_from, graph);

	    if (!from_vertex) {
	    	free (buffer);
			return FATAL_ERROR_FROM_VERTEX_NOT_FOUND;
	    }

	    if (vertex_obj2.HasMember("edges")) {
			auto edges = vertex_obj2["edges"].GetArray();
			for (rapidjson::Value::ConstValueIterator ite = edges.Begin(); ite != edges.End(); ++ite) {
				auto edge_obj = ite->GetObject();
				string *str_to = new string(edge_obj["to_vertex"].GetString());
				Vertex* to_vertex = findVertex(*str_to, graph);
			    if (!to_vertex) {
			    	free (buffer);
			    	return FATAL_ERROR_TO_VERTEX_NOT_FOUND;
			    }
				double weight = edge_obj["weight"].GetDouble();
				if (!addEdge(from_vertex, to_vertex, weight, graph, settings)) {
					free (buffer);
					return FATAL_ERROR_FAILED_TO_ADD_EDGE;
				}
			}
		}
	}
	free (buffer);
	return NO_ERROR;
}

int load2dGraph(Graph& graph, const Settings& settings) {
	unsigned long long width = 0, height = 0;

	ifstream file(settings.FilePath.c_str());
	if (file.fail()) {
		return FATAL_ERROR_FILE_OPEN_FAILURE;
	}
	string line;
	while(std::getline(file, line)){
		if (width < line.length()) width = line.length();
		height++;
	}
	file.clear();
	file.seekg(ios_base::beg);


	vector<vector<Vertex*>*> v2d;
	for (unsigned long long y1 = 0; y1 < height; y1++) {
		vector<Vertex*>* vector_line = new vector<Vertex*>;
		v2d.push_back(vector_line);
		for (unsigned long long x1 = 0; x1 < width; x1++) {
			std::ostringstream ss_name;
			ss_name << x1 << ":" << y1;
			Vertex *vertex = new Vertex2d(ss_name.str(), x1, y1);
			if (!addVertex(vertex, graph, settings)) return FATAL_ERROR_FAILED_TO_ADD_VERTEX;
			vector_line->push_back(vertex);
		}
	}
	for (unsigned long long y2 = 0; y2 < height; y2++) {
		for (unsigned long long x2 = 0; x2 < width; x2++) {
			if (y2 > 0) {
				if (!addEdge((*v2d[y2])[x2], (*v2d[y2-1])[x2], 1, graph, settings)){
					return FATAL_ERROR_FAILED_TO_ADD_EDGE;
				}
			}
			if (y2 < height - 1) {
				if (!addEdge((*v2d[y2])[x2], (*v2d[y2+1])[x2], 1, graph, settings)) {
					return FATAL_ERROR_FAILED_TO_ADD_EDGE;
				}
			}
			if (x2 > 0) {
				if(!addEdge((*v2d[y2])[x2], (*v2d[y2])[x2-1], 1, graph, settings)) {
					return FATAL_ERROR_FAILED_TO_ADD_EDGE;
				}
			}
			if (x2 < width - 1) {
				if (!addEdge((*v2d[y2])[x2], (*v2d[y2])[x2+1], 1, graph, settings)) {
					return FATAL_ERROR_FAILED_TO_ADD_EDGE;
				}
			}
		}
	}

	int y4 = 0;
	while(std::getline(file, line)) {
		for (unsigned long long x4 = 0; x4 < line.length(); x4++) {
			switch (line[x4]) {
			case '+':
				removeVertex(&(*v2d[y4])[x4], graph);
				break;
			case '-':
				break;
			case 0x13:
			case 0x10:
				break;
			default:
				//cerr << "Error: line "<< y4 << " contains illegal character " << line[x4] << "(" << int(line[x4]) << ")\n";
				return FATAL_ERROR_2D_FILE_CONTAINS_SHORT_LINE;
			}
		}
		y4++;
	}
	return 0;
}

void saveGraph(Graph& graph, const Settings& settings) {
	rapidjson::Document doc;
	doc.SetArray();
	for (auto &pair: graph) {
		rapidjson::Value vert(rapidjson::kObjectType);
		rapidjson::Value name(rapidjson::kStringType);
		rapidjson::Value edges(rapidjson::kArrayType);
		for (auto &edge: *pair.second->OutcomingEdges)
		{
			rapidjson::Value edge_value(rapidjson::kObjectType);
			rapidjson::Value to_vertex_value(rapidjson::kStringType);
			to_vertex_value.SetString(edge->ToVertex->Name.c_str(), edge->ToVertex->Name.length(), doc.GetAllocator());
			rapidjson::Value weight_value(edge->Weight);
			edge_value.AddMember ("to_vertex_value", to_vertex_value, doc.GetAllocator());
			edge_value.AddMember ("weight_value", weight_value, doc.GetAllocator());
			edges.PushBack(edge_value, doc.GetAllocator());
		}
		name.SetString(pair.second->Name.c_str(), pair.second->Name.length(), doc.GetAllocator());
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


