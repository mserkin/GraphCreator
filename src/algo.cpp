/*
 * algo.cpp
 *
 *  Created on: 5 џэт. 2020 у.
 *      Author: Serkin
 */

//#include <iterator>
//#include <algorithm>
#include <iostream>
#include <boost/heap/binomial_heap.hpp>
#include <queue>
#include <stack>
#include "graph.h"
#include "algo.h"

AlgoResult bfs(Vertex *source, Vertex *target, Callback callback, void* user_context) {
	if (!source || !target) {
		return NoSourceOrTarget;
	};

	queue<Vertex*> q;
	q.push(source);
	source->Context = (void*)true;

	while (!q.empty()) {
		Vertex *v = q.front();
		q.pop();
		if (callback) callback(VertexProcessingStarted, v, user_context);
		for (const auto &e : *(v->Edges)) {
			if (!e->ToVertex->Context) {
				e->ToVertex->Context = (void*)v;
				q.push(e->ToVertex);
				if (callback) callback(VertexDiscovered, e->ToVertex, user_context);
			}
			if (e->ToVertex == target) {
				if (callback) callback(TargetFound, e->ToVertex, user_context);
				if (callback) callback(AlgorithmFinished, nullptr, user_context);
				return Found;
			}
		}
		if (callback) callback(VertexProcessingFinished, v, user_context);
	}
	if (callback) callback(TargetNotFound, nullptr, user_context);
	if (callback) callback(AlgorithmFinished, nullptr, user_context);

    return NotFound;
}

AlgoResult dfs(Vertex *source, Vertex *target, Callback callback, void* user_context) {
	if (!source || !target) {
		return NoSourceOrTarget;
	};

	stack<Vertex*> q;
	q.push(source);
	source->Context = (void*)true;

	while (!q.empty()) {
		Vertex *v = q.top();
		q.pop();
		if (callback) callback(VertexProcessingStarted, v, user_context);
		for (const auto &e : *(v->Edges)) {
			if (!e->ToVertex->Context) {
				e->ToVertex->Context = (void*)v;
				q.push(e->ToVertex);
				if (callback) callback(VertexDiscovered, e->ToVertex, user_context);
			}
			if (e->ToVertex == target) {
				if (callback) callback(TargetFound, e->ToVertex, user_context);
				if (callback) callback(AlgorithmFinished, nullptr, user_context);
				return Found;
			}
		}
		if (callback) callback(VertexProcessingFinished, v, user_context);
	}
	if (callback) callback(TargetNotFound, nullptr, user_context);
	if (callback) callback(AlgorithmFinished, nullptr, user_context);
    return NotFound;
}

bool VertexComparator::operator()(const PVertex& v1, const PVertex& v2) const {
	DijkstraContext *c1, *c2;
	c1 = static_cast<DijkstraContext*>(v1->Context);
	c2 = static_cast<DijkstraContext*>(v2->Context);
	if (c1->Weight > c2->Weight )
		return true;
	else
		return false;
	// return "true" if "p1" is ordered before "p2", for example:
	//return static_cast<DijkstraContext*>(v1->Context)->Weight > static_cast<DijkstraContext*>(v2->Context)->Weight;
}

AlgoResult dijkstra(Vertex* source, Vertex* target, Graph& graph, Callback callback, void* user_context) {
	AlgoResult result = NotFound;
	boost::heap::binomial_heap<PVertex, boost::heap::compare<VertexComparator>> queue;
	DijkstraContext *context;
	for (const auto& v : graph) {
		context = new DijkstraContext();
		if (v == source) {
			context->Weight = 0;
		}
		v->Context = context;
		static_cast<DijkstraContext*>(v->Context)->Handle = queue.push(v);
	}

	Vertex* v;
	while (!queue.empty()) {
		v = queue.top();
		context = static_cast<DijkstraContext*>(v->Context);
		if (callback) callback(VertexProcessingStarted, v, user_context);
		for (const auto &e : *(v->Edges)) {
			if (static_cast<DijkstraContext*>(e->ToVertex->Context)->Processed) continue;
			if (callback) callback(VertexDiscovered, e->ToVertex, user_context);

			if (static_cast<DijkstraContext*>(e->ToVertex->Context)->Weight > context->Weight + e->Weight) {
				static_cast<DijkstraContext*>(e->ToVertex->Context)->Weight = context->Weight + e->Weight;
				static_cast<DijkstraContext*>(e->ToVertex->Context)->Parent = v;

				queue.increase(static_cast<VertexHandle>(static_cast<DijkstraContext*>(e->ToVertex->Context)->Handle), e->ToVertex);

				//cout << "After increase:" << std::endl;
			 	//for (auto it = queue.ordered_begin(); it != queue.ordered_end(); ++it) {
			    //	std::cout << (*it)->Name << ":" << static_cast<DijkstraContext*>((*it)->Context)->Weight << std::endl;
			    //}
			}
		}
		static_cast<DijkstraContext*>(v->Context)->Processed = true;
		if (callback) callback(VertexProcessingFinished, v, user_context);

		if (v == target) {
			if (callback) callback(TargetFound, v, user_context);
			result = Found;
			break;
		}
		queue.pop();
	};

	if (callback) callback(AlgorithmFinished, nullptr, user_context);
	return result;
}

AlgoResult bellman_ford(Vertex* source, Vertex* target, Graph& graph, Callback callback, void* user_context) {
	DijkstraContext *context;
	for (const auto& v : graph) {
		context = new DijkstraContext();
		if (v == source) {
			context->Weight = 0;
		}
		v->Context = context;
	}
	for (std::vector<Vertex*>::size_type i = 0; i <= graph.size(); i++) {
		for (const auto& v : graph) {
			if (callback) callback(VertexProcessingStarted, v, user_context);
			context = static_cast<DijkstraContext*>(v->Context);
			for (const auto &e : *(v->Edges)) {
				if (callback) callback(VertexDiscovered, e->ToVertex, user_context);

				if (static_cast<DijkstraContext*>(e->ToVertex->Context)->Weight > context->Weight + e->Weight) {
					static_cast<DijkstraContext*>(e->ToVertex->Context)->Weight = context->Weight + e->Weight;
					static_cast<DijkstraContext*>(e->ToVertex->Context)->Parent = v;
					if (i == graph.size()) {
						if (callback) callback(NegativeLoopDetected, v, user_context);
						return NotFound;
					}
				}
			}
			if (callback) callback(VertexProcessingFinished, v, user_context);
		}
	}

	if (callback) callback(AlgorithmFinished, nullptr, user_context);

	return (static_cast<DijkstraContext*>(target->Context)->Weight < INFINITY_WEIGHT) ? Found : NotFound;
}

AlgoResult bidirectional_dijkstra(Vertex* source, Vertex* target, Graph& graph, Callback callback, void* user_context) {
	AlgoResult result = NotFound;
	boost::heap::binomial_heap<PVertex, boost::heap::compare<VertexComparator>> forward_queue;
	boost::heap::binomial_heap<PVertex, boost::heap::compare<VertexComparator>> backward_queue;
	BidirectionalDijkstraContext *context;
	for (const auto& v : graph) {
		context = new BidirectionalDijkstraContext(new DijkstraContext(), new DijkstraContext());
		if (v == source) {
			context->ForwardContext->Weight = 0;
		}
		if (v == target) {
			context->BackwardContext->context->Weight = 0;
		}
		static_cast<BidirectionalDijkstraContext*>(v->Context)->ForwardContex->Handle = forward_queue.push(v);
		static_cast<BidirectionalDijkstraContext*>(v->Context)->BackwardContex->Handle = backward_queue.push(v);
	}

	Vertex* forward_v, backward_v;
	while (!forward_queue.empty() || !backward_queue.empty()) {
		if (!forward_queue.empty()) {
			forward_v = queue.top();
			forward_context = static_cast<BidirectionalDijkstraContext*>(v->Context);
			if (callback) callback(VertexProcessingStarted, v, user_context);
		
			for (const auto &fe : *(forward_v->Edges)) {
				if (static_cast<DijkstraContext*>(e->ToVertex->Context)->Processed) continue;
				if (callback) callback(VertexDiscovered, e->ToVertex, user_context);

				if (static_cast<DijkstraContext*>(e->ToVertex->Context)->Weight > context->Weight + e->Weight) {
					static_cast<DijkstraContext*>(e->ToVertex->Context)->Weight = context->Weight + e->Weight;
					static_cast<DijkstraContext*>(e->ToVertex->Context)->Parent = v;

					queue.increase(static_cast<VertexHandle>(static_cast<DijkstraContext*>(e->ToVertex->Context)->Handle), e->ToVertex);

					//cout << "After increase:" << std::endl;
					//for (auto it = queue.ordered_begin(); it != queue.ordered_end(); ++it) {
					//	std::cout << (*it)->Name << ":" << static_cast<DijkstraContext*>((*it)->Context)->Weight << std::endl;
					//}
				}
			}
			static_cast<DijkstraContext*>(v->Context)->Processed = true;
			if (callback) callback(VertexProcessingFinished, v, user_context);

			if (v == target) {
				if (callback) callback(TargetFound, v, user_context);
				result = Found;
				break;
			}
			queue.pop();
		};

		if (callback) callback(AlgorithmFinished, nullptr, user_context);
	return result;
}
