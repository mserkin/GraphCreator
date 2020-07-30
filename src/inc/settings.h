#ifndef __SETTINGS_H_
#define __SETTINGS_H_

#include <string>
#include "types.h"

using namespace std;

struct Settings {
	Algorithm SearchAlgorithm = Algorithm::None;
	int VertexCount = 6;
	int MaxEdgeCount = 8;
	bool SelfLoop = true;
	bool BiDirectional = true;
	bool Verbose = false;
	int MinEdgeWeight = 0;
	int MaxEdgeWeight = 0;
	int GraphHeight = 0;
	int GraphWidth = 0;
	string FilePath = "./graph.json";
	string SourceVertex = "";
	string TargetVertex = "";
	bool LoadFromFile = false;
	bool SaveToFile = false;
	GraphType TypeOfGraph = GraphType::GraphNonpositional;

	int parse (int argc, char **argv);
	void print ();
};

#endif
