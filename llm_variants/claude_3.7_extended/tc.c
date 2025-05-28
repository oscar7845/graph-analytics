#include "types.h"
#include "graph.h"

UINT_t tc_fast_llm(const GRAPH_TYPE *graph) {
    // Reorder the graph by degree (highest degree first) to optimize intersections
    GRAPH_TYPE *graph2 = reorder_graph_by_degree(graph, REORDER_HIGHEST_DEGREE_FIRST);
    
    const UINT_t n = graph2->numVertices;
    const UINT_t m = graph2->numEdges;
    const UINT_t* restrict Ap = graph2->rowPtr;
    const UINT_t* restrict Ai = graph2->colInd;
    
    UINT_t count = 0;
    
    // Allocate hash table for fast lookups
    bool* Hash = (bool *)calloc(n, sizeof(bool));
    assert_malloc(Hash);
    
    // Allocate forward data structures
    UINT_t* Size = (UINT_t *)calloc(n, sizeof(UINT_t));
    assert_malloc(Size);
    
    UINT_t* A = (UINT_t *)calloc(m, sizeof(UINT_t));
    assert_malloc(A);
    
    // Forward algorithm with hash-based intersection and early termination
    for (UINT_t s = 0; s < n; s++) {
        const UINT_t sb = Ap[s];
        const UINT_t se = Ap[s+1];
        
        for (UINT_t i = sb; i < se; i++) {
            const UINT_t t = Ai[i];
            
            // Direction-oriented processing (only process when s < t)
            if (s < t) {
                // Skip empty sets to avoid unnecessary work
                if (Size[s] > 0 && Size[t] > 0) {
                    // Determine which list is smaller for optimal hashing
                    if (Size[s] < Size[t]) {
                        // Hash s's forward list
                        for (UINT_t j = 0; j < Size[s]; j++) {
                            Hash[A[Ap[s] + j]] = true;
                        }
                        
                        // Count intersections with t's forward list
                        for (UINT_t j = 0; j < Size[t]; j++) {
                            if (Hash[A[Ap[t] + j]]) {
                                count++;
                            }
                        }
                        
                        // Reset hash table
                        for (UINT_t j = 0; j < Size[s]; j++) {
                            Hash[A[Ap[s] + j]] = false;
                        }
                    } else {
                        // Hash t's forward list
                        for (UINT_t j = 0; j < Size[t]; j++) {
                            Hash[A[Ap[t] + j]] = true;
                        }
                        
                        // Count intersections with s's forward list
                        for (UINT_t j = 0; j < Size[s]; j++) {
                            if (Hash[A[Ap[s] + j]]) {
                                count++;
                            }
                        }
                        
                        // Reset hash table
                        for (UINT_t j = 0; j < Size[t]; j++) {
                            Hash[A[Ap[t] + j]] = false;
                        }
                    }
                }
                
                // Add s to t's forward list
                A[Ap[t] + Size[t]] = s;
                Size[t]++;
            }
        }
    }
    
    free(Hash);
    free(Size);
    free(A);
    free_graph(graph2);
    
    return count;
}
