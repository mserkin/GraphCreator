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

void bfs(Vertex *source, Vertex *target, Callback callback, AlgoResult& result, void* user_context) {
	if (!source || !target) {
		result.ResultCode = NoSourceOrTarget;
		return;
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
				result.ResultCode = Found;
				return;
			}
		}
		if (callback) callback(VertexProcessingFinished, v, user_context);
	}
	if (callback) callback(TargetNotFound, nullptr, user_context);
	if (callback) callback(AlgorithmFinished, nullptr, user_context);

    result.ResultCode = NotFound;
}

void dfs(Vertex *source, Vertex *target, Callback callback, AlgoResult& result, void* user_context) {
	if (!source || !target) {
		result.ResultCode = NoSourceOrTarget;
		return;
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
				result.ResultCode = Found;
				return;
			}
		}
		if (callback) callback(VertexProcessingFinished, v, user_context);
	}
	if (callback) callback(TargetNotFound, nullptr, user_context);
	if (callback) callback(AlgorithmFinished, nullptr, user_context);
    result.ResultCode = NotFound;
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

void dijkstra(Vertex* source, Vertex* target, Graph& graph, Callback callback, AlgoResult& result, void* user_context) {
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
			result.ResultCode = Found;
			if (callback) callback(AlgorithmFinished, nullptr, user_context);
			break;
		}
		queue.pop();
	};

	if (callback) callback(AlgorithmFinished, nullptr, user_context);
	result.ResultCode = NotFound;
}

void bellman_ford(Vertex* source, Vertex* target, Graph& graph, Callback callback, AlgoResult& result, void* user_context) {
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
						if (callback) callback(AlgorithmFinished, nullptr, user_context);	
						result.ResultCode = NotFound;
						return;
					}
				}
			}
			if (callback) callback(VertexProcessingFinished, v, user_context);
		}
	}

	if (callback) callback(AlgorithmFinished, nullptr, user_context);
	result.ResultCode = (static_cast<DijkstraContext*>(target->Context)->Weight < INFINITY_WEIGHT) ? Found : NotFound;
}

void bidirectional_dijkstra(Vertex* source, Vertex* target, Graph& graph, BidiCallback callback, BidirectionalDijkstraResult& result, void* user_context) {
	boost::heap::binomial_heap<PVertex, boost::heap::compare<VertexComparator>> forward_queue;
	boost::heap::binomial_heap<PVertex, boost::heap::compare<VertexComparator>> backward_queue;
	BidirectionalDijkstraContext *context;
	result.ResultCode = NotFound;
	for (const auto& v : graph) {
		context = new BidirectionalDijkstraContext(new DijkstraContext(), new DijkstraContext());
		if (v == source) {
			context->ForwardContext->Weight = 0;
		}
		if (v == target) {
			context->BackwardContext->Weight = 0;
		}
		static_cast<BidirectionalDijkstraContext*>(v->Context)->ForwardContext->Handle = forward_queue.push(v);
		static_cast<BidirectionalDijkstraContext*>(v->Context)->BackwardContext->Handle = backward_queue.push(v);
	}
	
	weight_t shortest_ever_path = INFINITY_WEIGHT, new_short_path;
	Vertex *forward_vertex, *backward_vertex, *forward_shortest_path_vertex = nullptr, *backward_shortest_path_vertex = nullptr;
	DijkstraContext *neighbour_forward_context, *neighbour_backward_context;
	while (!forward_queue.empty() && !backward_queue.empty()) {
		forward_vertex = forward_queue.top();
		backward_vertex = backward_queue.top();
		
		if (shortest_ever_path < static_cast<BidirectionalDijkstraContext*>(forward_queue.top()->Context)->ForwardContext->Weight +
				static_cast<BidirectionalDijkstraContext*>(backward_queue.top()->Context)->BackwardContext->Weight)
		{
			if (callback) callback(TargetFound, forward_shortest_path_vertex, backward_shortest_path_vertex, user_context);
			if (callback) callback(AlgorithmFinished, nullptr, nullptr, user_context);
		}
		
		if (!forward_queue.empty()) {
			context = static_cast<BidirectionalDijkstraContext*>(forward_vertex->Context);
			if (callback) callback(VertexProcessingStarted, forward_vertex, nullptr, user_context);
		
			for (const auto &fe : *(forward_vertex->Edges)) {
				neighbour_forward_context = static_cast<BidirectionalDijkstraContext*>(fe->ToVertex->Context)->ForwardContext;
				neighbour_backward_context = static_cast<BidirectionalDijkstraContext*>(fe->ToVertex->Context)->BackwardContext;
				if (neighbour_forward_context->Processed) continue;
				if (callback) callback(VertexDiscovered, fe->ToVertex, nullptr, user_context);
				
				if (neighbour_forward_context->Weight > context->ForwardContext->Weight + fe->Weight) {
					neighbour_forward_context->Weight = context->ForwardContext->Weight + fe->Weight;
					neighbour_forward_context->Parent = forward_vertex;

					forward_queue.increase(static_cast<VertexHandle>(neighbour_forward_context->Handle), fe->ToVertex);
				}
				if (neighbour_backward_context->Processed) {
					new_short_path = neighbour_backward_context->Weight + fe->Weight + context->ForwardContext->Weight;
					if (new_short_path < shortest_ever_path) {
						shortest_ever_path = new_short_path;
						forward_shortest_path_vertex = forward_vertex;
						backward_shortest_path_vertex = fe->ToVertex;
					}
				}
			}
			context->ForwardContext->Processed = true;
			if (callback) callback(VertexProcessingFinished, forward_vertex, nullptr, user_context);

			if (forward_vertex == target) {
				if (callback) callback(TargetFound, forward_vertex, nullptr, user_context);
				result.ResultCode = Found;
				break;
			}
			forward_queue.pop();
		};

		if (!backward_queue.empty()) {
			context = static_cast<BidirectionalDijkstraContext*>(backward_vertex->Context);
			if (callback) callback(VertexProcessingStarted, nullptr, backward_vertex, user_context);
		
			for (const auto &be : *(backward_vertex->IncomingEdges)) {
				neighbour_backward_context = static_cast<BidirectionalDijkstraContext*>(be->ToVertex->Context)->BackwardContext;
				neighbour_forward_context = static_cast<BidirectionalDijkstraContext*>(be->ToVertex->Context)->ForwardContext;
				if (neighbour_backward_context->Processed) continue;
				if (callback) callback(VertexDiscovered, nullptr, be->FromVertex, user_context);
				
				if (neighbour_backward_context->Weight > context->BackwardContext->Weight + be->Weight) {
					neighbour_backward_context->Weight = context->BackwardContext->Weight + be->Weight;
					neighbour_backward_context->Parent = backward_vertex;

					backward_queue.increase(static_cast<VertexHandle>(neighbour_backward_context->Handle), be->FromVertex);
				}
				if (neighbour_forward_context->Processed) {
					new_short_path = neighbour_forward_context->Weight + be->Weight + context->BackwardContext->Weight;
					if (new_short_path < shortest_ever_path) {
						shortest_ever_path = new_short_path;
						forward_shortest_path_vertex = be->FromVertex;
						backward_shortest_path_vertex = backward_vertex;
					}
				}
			}
			context->BackwardContext->Processed = true;
			if (callback) callback(VertexProcessingFinished, nullptr, backward_vertex, user_context);

			if (backward_vertex == source) {
				if (callback) callback(TargetFound, nullptr, backward_vertex, user_context);
				result.ResultCode = Found;
				break;
			}
			backward_queue.pop();
		};
	}
	if (callback) callback(AlgorithmFinished, nullptr, nullptr, user_context);
};
