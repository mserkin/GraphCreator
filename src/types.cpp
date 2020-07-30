/*
 * types.h
 *
 *  Created on: 22 èþë. 2020 ã.
 *      Author: Serkin
 */
#include "types.h"


Vertex::Vertex(std::string _Name): Name(_Name) {
	IncomingEdges = new EdgeList();
	OutcomingEdges = new EdgeList();
};

Vertex::~Vertex() {
	PEdge pedge;
	while (!this->OutcomingEdges->empty()) {
		//take last
		auto it1 = this->OutcomingEdges->end();
		pedge = *(--it1);
		//remove last
		this->OutcomingEdges->pop_back();
		//find and remove edge from incomings
		for (it1 = pedge->ToVertex->IncomingEdges->begin(); it1 !=pedge->ToVertex->IncomingEdges->end(); it1++) {
			if ((*it1)->FromVertex == this) {
				pedge->ToVertex->IncomingEdges->erase(it1);
				break;
			}
		}
		delete pedge;
	}
	while (!this->IncomingEdges->empty()) {
		//take last
		auto it2 = this->IncomingEdges->end();
		pedge = *(--it2);
		//remove last
		this->IncomingEdges->pop_back();
		//find edge in outcomings
		for (it2 = pedge->FromVertex->OutcomingEdges->begin(); it2 !=pedge->FromVertex->OutcomingEdges->end(); it2++) {
			if ((*it2)->ToVertex == this) {
				//Erase it :)
				pedge->FromVertex->OutcomingEdges->erase(it2);
				break;
			}
		}
		delete pedge;
	}

	delete OutcomingEdges;
	delete IncomingEdges;
}

Vertex *Vertex::edgelessClone() {
	Vertex *clone = new Vertex(this->Name);
	return clone;
};

