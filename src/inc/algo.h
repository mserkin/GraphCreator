/*
 * algo.h
 */

#ifndef ALGO_H_
#define ALGO_H_

#include <boost/heap/binomial_heap.hpp>
#include <map>
#include "graph.h"


//Breadth-first search. Finds the shortest path from source to target in the sense of the minimum number of transitions between vertices.
//The algorithm does not consider (ignores) the weight of the edges.
//Complexity O(|V| + |E|)
//source - source vertex
//target - target vertex
//callback - function that is called by algorithm to supple events to caller
//result - result of algorithm execution: Found - path from source to target was found or NotFound if no path from source to target was found
//user_context - pointer to some info - may be needed to understand the origin of event if several algorithm
//               launches were made simultaneously
//Way to get found path:
//target->Context will contain a pointer to a previous vertex. The previous vertex's Context field will pointer to it's previous vertex
//and so until source is met.
void bfs(Vertex* source, Vertex* target, Callback callback, AlgoResult& result, void* user_context = nullptr);

//Depth-first search. Finds any path from source to target
//Complexity O(|V| + |E|)
//source - source vertex
//target - target vertex
//callback - function that is called by algorithm to supple events to caller
//result - result of algorithm execution: Found - path from source to target was found or NotFound if no path from source to target was found
//user_context - pointer to some info - may be needed to understand the origin of event if several algorithm
//               launches were made simultaneously
//Way to get found path:
//target->Context will contain a pointer to a previous vertex. The previous vertex's Context field will pointer to it's previous vertex
//and so until source is met.
void dfs(Vertex* source, Vertex* target, Callback callback, AlgoResult& result, void* user_context = nullptr);

//Dijkstra algorithm implementation. Finds the minimum weight path from source to target in a weighted graph.
//Complexity O(|E|+|V|*log(|V|)
//source - source vertex
//target - target vertex
//callback - function that is called by algorithm to supple events to caller
//result - result of algorithm execution: Found - path from source to target found or NotFound if no path from source to target was found
//user_context - pointer to some info - may be needed to understand the origin of event if several algorithm
//               launches were made simultaneously
//Way to get found path:
//    static_cast<DijkstraContext*>(target->Context)->Parent will contain a pointer to a previous vertex.
//    Do with it the same to get it's previous vertex. Do so until source is met.
void dijkstra(Vertex* source, Vertex* target, Graph& graph, Callback callback,  AlgoResult& result, void* user_context = nullptr);

//Bidirectional Dijkstra algorithm implementation. Finds the minimum weight path from source to target in a weighted graph.
//The search is performed from source to target and from target to source using incoming edges - Vertex::IncomingEdged should be provided for each vertex.
//source - source vertex
//target - target vertex
//callback - function that is called by algorithm to supple events to caller
//result - result of algorithm execution: Found - path from source to target found or NotFound if no path from source to target was found
//user_context - pointer to some info - may be needed to understand the origin of event if several algorithm
//               launches were made simultaneously
//Way to get found path:
//    path is represented by two parts:
//        1) result.ForwardSearchLastVertex will contain the last vertex in minimum weight path that was found by forward search.
//        go from it to source (opposite direction):
//		     get previous vertex: static_cast<BidirectionalDijkstraContext*>(result.ForwardSearchLastVertex->Context)->ParentInForwardSearch
//           do so until the source is met
//        2) result.BackwardSearchLastVertex will contain the last vertex in minimum weight path that was found by backward search.
//        go from it to target (the direction will be the same as of path itself):
//		     get previous vertex: static_cast<BidirectionalDijkstraContext*>(result.BackwardSearchLastVertex->Context)->ParentInBackwardSearch
//           do so until the target is met
void bidirectionalDijkstra(Vertex* source, Vertex* target, Graph& graph, Callback callback, BidirectionalDijkstraResult& result, void* user_context = nullptr);

//Bellman-Ford algorithm implementation. Finds the minimum weight path from single source to *ALL* other vertices in a weighted graph.
//Found paths and its weights are accessible using context information associated to vertices after run.
//Do not send VertexDiscovered, VertexProcessingStarted and VertexProcessingFinished events as all vertices processed several times
//Sends NegativeLoopDetected with vertex where negative loop was detected
//Complexity O(|V||E|)
//source - source vertex
//target - target vertex
//callback - function that is called by algorithm to supple events to caller
//result - result of algorithm execution:
//          Found - path from source to target found
//          NotFound - no path from source to target was found
//          NegativeLoopFound - negative loop was detected and algorithm stopped. Results (vertex context information) are undefined.
//user_context - pointer to some info - may be needed to understand the origin of event if several algorithm
//               launches were made simultaneously
//Way to get found path is the same as for Dijkstra algorithm
void bellmanFord(Vertex* source, Vertex* target, Graph& graph, Callback callback,  AlgoResult& result, void* user_context = nullptr);
#endif /* ALGO_H_ */

//Delete all context allocated during algorithm work. Should be called after results of algorithm are no more needed
//and before next algorithm call on the same graph.
//Graph& graph - graph
void clearContext (Graph& graph);
