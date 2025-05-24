#include "types.h"
#include "graph.h"

UINT_t tc_fast_llm(const GRAPH_TYPE *graph) {
    const UINT_t n = graph->numVertices;
    const UINT_t m = graph->numEdges;
    const UINT_t* restrict Ap = graph->rowPtr;
    const UINT_t* restrict Ai = graph->colInd;
    
    // Early return for empty graphs
    if (n <= 2 || m == 0) return 0;
    
    UINT_t count = 0;
    
    // Compute vertex degrees for sorting and pruning
    UINT_t* degree = (UINT_t*)malloc(n * sizeof(UINT_t));
    assert_malloc(degree);
    for (UINT_t i = 0; i < n; i++) {
        degree[i] = Ap[i+1] - Ap[i];
    }
    
    // Create degeneracy ordering (smallest to largest)
    UINT_t* order = (UINT_t*)malloc(n * sizeof(UINT_t));
    UINT_t* rank = (UINT_t*)malloc(n * sizeof(UINT_t));
    assert_malloc(order);
    assert_malloc(rank);
    
    // Initialize ordering data structures
    for (UINT_t i = 0; i < n; i++) {
        order[i] = i;
    }
    
    // Sort vertices by degree (simple insertion sort for small graphs, 
    // would use counting/bucket sort for larger graphs)
    for (UINT_t i = 1; i < n; i++) {
        UINT_t j = i;
        UINT_t temp = order[i];
        while (j > 0 && degree[temp] < degree[order[j-1]]) {
            order[j] = order[j-1];
            j--;
        }
        order[j] = temp;
    }
    
    // Compute rank (position in order)
    for (UINT_t i = 0; i < n; i++) {
        rank[order[i]] = i;
    }
    
    // Allocate compact neighborhoods - only store higher-ranked neighbors
    UINT_t** compact_neighbors = (UINT_t**)malloc(n * sizeof(UINT_t*));
    UINT_t* compact_size = (UINT_t*)calloc(n, sizeof(UINT_t));
    assert_malloc(compact_neighbors);
    assert_malloc(compact_size);
    
    // First compute sizes
    for (UINT_t i = 0; i < n; i++) {
        UINT_t v = order[i];
        UINT_t size = 0;
        
        for (UINT_t j = Ap[v]; j < Ap[v+1]; j++) {
            UINT_t nbr = Ai[j];
            if (rank[nbr] > i) { // Only keep higher-ranked neighbors
                size++;
            }
        }
        
        compact_size[i] = size;
        if (size > 0) {
            compact_neighbors[i] = (UINT_t*)malloc(size * sizeof(UINT_t));
            assert_malloc(compact_neighbors[i]);
        } else {
            compact_neighbors[i] = NULL;
        }
    }
    
    // Fill compact neighborhoods
    for (UINT_t i = 0; i < n; i++) {
        UINT_t v = order[i];
        UINT_t pos = 0;
        
        for (UINT_t j = Ap[v]; j < Ap[v+1]; j++) {
            UINT_t nbr = Ai[j];
            if (rank[nbr] > i) { // Only keep higher-ranked neighbors
                compact_neighbors[i][pos++] = rank[nbr];
            }
        }
    }
    
    // Allocate hash table for large intersection operations
    // Size is based on the max degree in the graph
    UINT_t max_degree = 0;
    for (UINT_t i = 0; i < n; i++) {
        if (degree[i] > max_degree) max_degree = degree[i];
    }
    
    bool* hash_table = (bool*)calloc(n, sizeof(bool));
    assert_malloc(hash_table);
    
    // Main triangle counting loop - process vertices in degeneracy order
    for (UINT_t i = 0; i < n; i++) {
        UINT_t v = order[i];
        UINT_t v_nbrs_size = compact_size[i];
        
        if (v_nbrs_size < 2) continue; // Need at least 2 higher neighbors to form triangles
        
        UINT_t* v_nbrs = compact_neighbors[i];
        
        // Use hash-based intersection for larger neighborhoods
        if (v_nbrs_size > 32) {
            // Insert all of v's neighbors into hash table
            for (UINT_t j = 0; j < v_nbrs_size; j++) {
                hash_table[v_nbrs[j]] = true;
            }
            
            // Check for triangles between all pairs of v's neighbors
            for (UINT_t j = 0; j < v_nbrs_size; j++) {
                UINT_t w_idx = v_nbrs[j];
                UINT_t w = order[w_idx];
                UINT_t* w_nbrs = compact_neighbors[w_idx];
                UINT_t w_nbrs_size = compact_size[w_idx];
                
                for (UINT_t k = 0; k < w_nbrs_size; k++) {
                    if (hash_table[w_nbrs[k]]) {
                        count++;
                    }
                }
            }
            
            // Reset hash table
            for (UINT_t j = 0; j < v_nbrs_size; j++) {
                hash_table[v_nbrs[j]] = false;
            }
        } else {
            // Use direct comparison for smaller neighborhoods - more cache-friendly
            for (UINT_t j = 0; j < v_nbrs_size; j++) {
                UINT_t w_idx = v_nbrs[j];
                UINT_t* w_nbrs = compact_neighbors[w_idx];
                UINT_t w_nbrs_size = compact_size[w_idx];
                
                // Get the intersection size of v_nbrs and w_nbrs using merge-path
                UINT_t k1 = 0, k2 = 0;
                while (k1 < v_nbrs_size && k2 < w_nbrs_size) {
                    if (v_nbrs[k1] < w_nbrs[k2]) {
                        k1++;
                    } else if (v_nbrs[k1] > w_nbrs[k2]) {
                        k2++;
                    } else {
                        count++;
                        k1++;
                        k2++;
                    }
                }
            }
        }
    }
    
    // Cleanup
    free(hash_table);
    free(degree);
    free(rank);
    free(order);
    free(compact_size);
    
    // Free compact neighbors
    for (UINT_t i = 0; i < n; i++) {
        if (compact_neighbors[i] != NULL) {
            free(compact_neighbors[i]);
        }
    }
    free(compact_neighbors);
    
    return count;
}
