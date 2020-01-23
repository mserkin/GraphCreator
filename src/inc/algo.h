/*
 * algo.h
 *
 *  Created on: 5 џэт. 2020 у.
 *      Author: Serkin
 */

#ifndef ALGO_H_
#define ALGO_H_

#include <boost/heap/binomial_heap.hpp>
#include "graph.h"

enum AlgoEvent {
	VertexDiscovered,
	VertexProcessingStarted,
	VertexProcessingFinished,
	NegativeLoopDetected,
	TargetFound,
	TargetNotFound,
	AlgorithmFinished
};

enum AlgoResultCode {
	Ok,
	Found,
	NotFound,
	NoSourceOrTarget
};

struct AlgoResult {
	AlgoResultCode ResultCode = NotFound;
}

struct BidirectionalDijkstraResult : AlgoResult{
	Vertex* ForwardSearchLastVertex = nullptr;
	Vertex* BackwardSearchLastVertex = nullptr;
	Edge* ConnectingEdge = nullptr;
	BidirectionalDijkstraResult(AlgoResultCode code, Vertex* forward_search_vertex, Vertex* backward_search_vertex, Edge* edge) {
		ResultCode = code;
		ForwardSearchLastVertex = forward_search_vertex;
		ForwardSearchLastVertex = backward_search_vertex;
		ConnectingEdge = edge;
	}
}

struct DijkstraContext;

struct VertexComparator {
    bool operator()(const PVertex& v1, const PVertex& v2) const;
};

typedef boost::heap::detail::node_handle<boost::heap::detail::parent_pointing_heap_node<PVertex>*, boost::heap::detail::make_binomial_heap_base<PVertex, boost::parameter::aux::flat_like_arg_list<boost::parameter::aux::flat_like_arg_tuple<boost::heap::tag::compare, boost::heap::compare<VertexComparator>, std::integral_constant<bool, true> > > >::type, PVertex&> VertexHandle;
typedef double weight_t;

const double INFINITY_WEIGHT = 10000000000;

struct DijkstraContext {
	weight_t Weight = INFINITY_WEIGHT;
	Vertex* Parent = nullptr;
	VertexHandle Handle;
	bool Processed = false;
};

struct BidirectionalDijkstraContext {
	DijkstraContext* ForwardContext = nullptr;
	DijkstraContext* BackwardContext = nullptr;
	BidirectionalDijkstraContext(DijkstraContext* forward, DijkstraContext* backward): ForwardContext(forward), BackwardContext(backward) {};
};

typedef void (*Callback)(AlgoEvent, Vertex*, void* user_context);
typedef void (*BidiCallback)(AlgoEvent, Vertex*, Vertex*, void* user_context);


void bfs(Vertex* source, Vertex* target, Callback callback, void* user_context = nullptr, AlgoResult& result);

void dfs(Vertex* source, Vertex* target, Callback callback, void* user_context = nullptr, AlgoResult& result);

void dijkstra(Vertex* source, Vertex* target, Graph& graph, Callback callback, void* user_context = nullptr, AlgoResult& result);

void bellman_ford(Vertex* source, Vertex* target, Graph& graph, Callback callback, void* user_context = nullptr, AlgoResult& result);

void bidirectionalDijkstra(Vertex* source, Vertex* target, Graph& graph, Callback callback, void* user_context = nullptr, BidirectionalDijkstraResult& result);

#endif /* ALGO_H_ */
