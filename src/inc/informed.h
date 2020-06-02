/*
 * specalgo.h
 *
 *  Created on: 23 мая 2020 г.
 *      Author: Serkin
 */

#ifndef INC_INFORMED_H_
#define INC_INFORMED_H_

#include <string>
#include "graph.h"
#include "algo.h"

struct Vertex2d : public Vertex {
	int X;
	int Y;
	Vertex2d(string _Name, int _X,  int _Y): Vertex(_Name), X(_X), Y(_Y) {};
};

void dijkstra2d(Vertex2d* source, Vertex2d* target, Graph& graph, Callback callback,  AlgoResult& result, void* user_context = nullptr, double coefficient = 0.5);


#endif /* INC_INFORMED_H_ */
