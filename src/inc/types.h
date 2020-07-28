/*
 * types.h
 *
 *  Created on: 22 èþë. 2020 ã.
 *      Author: Serkin
 */

#ifndef INC_TYPES_H_
#define INC_TYPES_H_

#include <string>
#include <vector>
#include <map>
#include <boost/heap/binomial_heap.hpp>

#define INFINITE_WEIGHT 1E30

enum class Algorithm {
	None,
	BreadthFirstSearch,
	DepthFirstSearch,
	Dijkstra,
	FastDijkstra,
	BellmanFord,
	Dijkstra2D,
};

enum class AlgoEvent {
	VertexDiscovered,            //Vertex were inspected but not yet fully processed
	VertexProcessingStarted,     //Start to process vertex
	VertexProcessingFinished,    //Vertex processing finished
	NegativeLoopDetected,        //Algorithm detected negative cycle
	TargetFound,                 //Target vertex was found during search from source vertex
	TargetNotFound,              //Processing finished but target vertex was not found
	AlgorithmFinished            //Finished executing the algorithm
};

enum class AlgoResultCode {
	Ok,							//Finished executing the algorithm
	Found,                      //Found a path, shortest path or minimal-weight path
	NotFound,                   //No paths from source to target was not found
	NoSourceOrTarget,           //Source or target vertex was not found in graph
	SourceIsTarget,             //Source and target are the same vertex
	NegativeLoopFound		    //Negative loop was detected and algorithm stopped
};

enum class GraphType {
	GraphNonpositional,
	Graph2D
};

struct AlgoResult {
	AlgoResultCode ResultCode = AlgoResultCode::NotFound;
	std::string getText();
};

struct Edge;

typedef std::vector<Edge*> EdgeList;

struct Vertex {
	std::string Name;
	EdgeList *OutcomingEdges = nullptr;
	EdgeList *IncomingEdges = nullptr;
	void* Context = nullptr;
	Vertex(std::string _Name);
	virtual ~Vertex();
	//makes copy of vertex. Edges and context are not cloned
	virtual Vertex* edgelessClone();
};

typedef Vertex* PVertex;

struct Edge {
	Vertex *FromVertex;
	Vertex *ToVertex;
	double Weight;
	Edge(Vertex* _FromVertex, Vertex* _ToVertex, double _Weight):
		FromVertex(_FromVertex), ToVertex(_ToVertex), Weight(_Weight) {};
};

typedef Edge* PEdge;

typedef std::map<std::string, Vertex*> Graph;

struct DijkstraContext;

struct DijkstraVertexComparator {  //Compare functor that is used in Dijkstra algorithm
    bool operator()(const PVertex& v1, const PVertex& v2) const;
};

struct FastDijkstraForwardComparator { //Compare functor that is used in bidirectional Dijkstra algorithm in forward search
    bool operator()(const PVertex& v1, const PVertex& v2) const;
};

struct FastDijkstraBackwardComparator { //Compare functor that is used in bidirectional Dijkstra algorithm in backward search
    bool operator()(const PVertex& v1, const PVertex& v2) const;
};

typedef boost::heap::detail::node_handle<boost::heap::detail::parent_pointing_heap_node<PVertex>*, boost::heap::detail::make_binomial_heap_base<PVertex, boost::parameter::aux::flat_like_arg_list<boost::parameter::aux::flat_like_arg_tuple<boost::heap::tag::compare, boost::heap::compare<DijkstraVertexComparator>, std::integral_constant<bool, true> > > >::type, PVertex&> VertexHandle;
typedef boost::heap::detail::node_handle<boost::heap::detail::parent_pointing_heap_node<PVertex>*, boost::heap::detail::make_binomial_heap_base<PVertex, boost::parameter::aux::flat_like_arg_list<boost::parameter::aux::flat_like_arg_tuple<boost::heap::tag::compare, boost::heap::compare<FastDijkstraForwardComparator>, std::integral_constant<bool, true> > > >::type, PVertex&> ForwardSearchVertexHandle;
typedef boost::heap::detail::node_handle<boost::heap::detail::parent_pointing_heap_node<PVertex>*, boost::heap::detail::make_binomial_heap_base<PVertex, boost::parameter::aux::flat_like_arg_list<boost::parameter::aux::flat_like_arg_tuple<boost::heap::tag::compare, boost::heap::compare<FastDijkstraBackwardComparator>, std::integral_constant<bool, true> > > >::type, PVertex&> BackwardSearchVertexHandle;

typedef double weight_t;


struct BidirectionalDijkstraResult : AlgoResult{
	Vertex* ForwardSearchLastVertex = nullptr;     //Last vertex of the minimal weight path in forward search (before connecting edge)
	Vertex* BackwardSearchLastVertex = nullptr;    //Last vertex of the minimal weight path in backward search (after connecting edge)
	weight_t ConnectingEdgeWeight = INFINITE_WEIGHT; //Weight of the edge that connects ForwardSearchLastVertex and BackwardSearchLastVertex
	BidirectionalDijkstraResult() {
		ResultCode = AlgoResultCode::NotFound;
	}
};

struct DijkstraContext {   //Vertex additional information for Dijkstra algorithm.
	weight_t Weight = INFINITE_WEIGHT;  //Weight of the minimal-weight path from source to that vertex
	Vertex* Parent = nullptr;           //Previous vertex in the minimal-weight path from source to that vertex
	VertexHandle Handle;                //Handle of the vertex to be used in priority queue (benomial heap)
	bool Processed = false;             //Vertex processing completion mark
};

struct BidirectionalDijkstraContext { //Vertex additional information for bidirectional Dijkstra algorithm.
	weight_t WeightInForwardSearch = INFINITE_WEIGHT;  //Weight of the minimal-weight path from source to that vertex
	Vertex* ParentInForwardSearch = nullptr;           //Previous vertex in the minimal-weight path from source to that vertex
	ForwardSearchVertexHandle HandleInForwardSearch;   //Handle of the vertex to be used in forward search priority queue (benomial heap)
	bool ProcessedByForwardSearch = false; 			   //Vertex forward search processing completion mark
	weight_t WeightInBackwardSearch = INFINITE_WEIGHT; //Weight of the minimal-weight path from target to that vertex
	Vertex* ParentInBackwardSearch = nullptr;          //Previous vertex in the minimal-weight path from target to that vertex
	BackwardSearchVertexHandle HandleInBackwardSearch; //Handle of the vertex to be used in backward search priority queue (benomial heap)
	bool ProcessedByBackwardSearch = false;            //Vertex backward search processing completion mark
	BidirectionalDijkstraContext(){};
};

//Callback prototype
//event - event type
//vertex - vertex the event is about
//user_context - the context that was set when the algorithm started.
//       it may be needed to understand the origin of event if several algorithm launches were made simultaneously
typedef void (*Callback)(AlgoEvent event, Vertex* vertex, void* user_context);

#endif /* INC_TYPES_H_ */
