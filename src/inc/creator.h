/*
 * creator.h
 *
 *  Created on: 23 мая 2020 г.
 *      Author: Serkin
 */

#ifndef INC_CREATOR_H_
#define INC_CREATOR_H_

#include "graph.h"
#include "settings.h"

void createGraph(Graph& graph, const Settings& settings);

int chooseToVertex(Vertex *vertex_from, const Settings &settings, Graph &graph,
		int &error_count);

#endif /* INC_CREATOR_H_ */
