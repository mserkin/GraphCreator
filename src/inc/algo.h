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
};

struct DijkstraContext;

struct DijkstraVertexComparator {
    bool operator()(const PVertex& v1, const PVertex& v2) const;
};

struct FastDijkstraForwardComparator {
    bool operator()(const PVertex& v1, const PVertex& v2) const;
};

struct FastDijkstraBackwardComparator {
    bool operator()(const PVertex& v1, const PVertex& v2) const;
};

typedef boost::heap::detail::node_handle<boost::heap::detail::parent_pointing_heap_node<PVertex>*, boost::heap::detail::make_binomial_heap_base<PVertex, boost::parameter::aux::flat_like_arg_list<boost::parameter::aux::flat_like_arg_tuple<boost::heap::tag::compare, boost::heap::compare<DijkstraVertexComparator>, std::integral_constant<bool, true> > > >::type, PVertex&> VertexHandle;
typedef boost::heap::detail::node_handle<boost::heap::detail::parent_pointing_heap_node<PVertex>*, boost::heap::detail::make_binomial_heap_base<PVertex, boost::parameter::aux::flat_like_arg_list<boost::parameter::aux::flat_like_arg_tuple<boost::heap::tag::compare, boost::heap::compare<FastDijkstraForwardComparator>, std::integral_constant<bool, true> > > >::type, PVertex&> ForwardSearchVertexHandle;
typedef boost::heap::detail::node_handle<boost::heap::detail::parent_pointing_heap_node<PVertex>*, boost::heap::detail::make_binomial_heap_base<PVertex, boost::parameter::aux::flat_like_arg_list<boost::parameter::aux::flat_like_arg_tuple<boost::heap::tag::compare, boost::heap::compare<FastDijkstraBackwardComparator>, std::integral_constant<bool, true> > > >::type, PVertex&> BackwardSearchVertexHandle;
typedef double weight_t;

#define INFINITY_WEIGHT 1000000000.0

struct BidirectionalDijkstraResult : AlgoResult{
	Vertex* ForwardSearchLastVertex = nullptr;
	Vertex* BackwardSearchLastVertex = nullptr;
	weight_t ConnectingEdgeWeight = INFINITY_WEIGHT;
	BidirectionalDijkstraResult() {
		ResultCode = NotFound;
	}
};

struct DijkstraContext {
	weight_t Weight = INFINITY_WEIGHT;
	Vertex* Parent = nullptr;
	VertexHandle Handle;
	bool Processed = false;
};

struct BidirectionalDijkstraContext {
	//DijkstraContext* ForwardContext = nullptr;
	//DijkstraContext* BackwardContext = nullptr;
	weight_t WeightInForwardSearch = INFINITY_WEIGHT;
	Vertex* ParentInForwardSearch = nullptr;
	ForwardSearchVertexHandle HandleInForwardSearch;
	bool ProcessedByForwardSearch = false;
	weight_t WeightInBackwardSearch = INFINITY_WEIGHT;
	Vertex* ParentInBackwardSearch = nullptr;
	BackwardSearchVertexHandle HandleInBackwardSearch;
	bool ProcessedByBackwardSearch = false;
	BidirectionalDijkstraContext(){};
};

typedef void (*Callback)(AlgoEvent, Vertex*, void* user_context);

void bfs(Vertex* source, Vertex* target, Callback callback, AlgoResult& result, void* user_context = nullptr);

void dfs(Vertex* source, Vertex* target, Callback callback, AlgoResult& result, void* user_context = nullptr);

void dijkstra(Vertex* source, Vertex* target, Graph& graph, Callback callback,  AlgoResult& result, void* user_context = nullptr);

void bellmanFord(Vertex* source, Vertex* target, Graph& graph, Callback callback,  AlgoResult& result, void* user_context = nullptr);

void bidirectionalDijkstra(Vertex* source, Vertex* target, Graph& graph, Callback callback, BidirectionalDijkstraResult& result, void* user_context = nullptr);

#endif /* ALGO_H_ */
