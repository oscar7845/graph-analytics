#include "types.h"
#include "graph.h" // For GRAPH_TYPE definition and access to rowPtr, colInd
#include <stdlib.h> // For calloc and free
#include <stdio.h>  // For fprintf and stderr, for basic error handling

// Helper function to find the intersection size of two adjacency lists
// using a boolean (hash) array for marking.
// This function assumes `Ai` (column indices) are sorted for each row.
static UINT_t intersect_neighbors_count(const UINT_t *Ap, const UINT_t *Ai, bool *Hash, UINT_t u_idx, UINT_t v_idx) {
    UINT_t common_neighbors = 0;

    // Get start and end pointers for adjacency list of u_idx
    UINT_t u_start = Ap[u_idx];
    UINT_t u_end = Ap[u_idx + 1];
    UINT_t u_deg = u_end - u_start;

    // Get start and end pointers for adjacency list of v_idx
    UINT_t v_start = Ap[v_idx];
    UINT_t v_end = Ap[v_idx + 1];
    UINT_t v_deg = v_end - v_start;

    const UINT_t *list_to_mark_start_ptr;
    UINT_t list_to_mark_count;
    const UINT_t *list_to_check_start_ptr;
    UINT_t list_to_check_count;

    // Optimize by marking the shorter adjacency list in the Hash array.
    // Then iterate through the longer list to count common elements.
    if (u_deg <= v_deg) {
        list_to_mark_start_ptr = &Ai[u_start];
        list_to_mark_count = u_deg;
        list_to_check_start_ptr = &Ai[v_start];
        list_to_check_count = v_deg;
    } else {
        list_to_mark_start_ptr = &Ai[v_start];
        list_to_mark_count = v_deg;
        list_to_check_start_ptr = &Ai[u_start];
        list_to_check_count = u_deg;
    }

    // Mark elements from the smaller list in the Hash array
    for (UINT_t i = 0; i < list_to_mark_count; ++i) {
        Hash[list_to_mark_start_ptr[i]] = true;
    }

    // Count common neighbors by checking the larger list against the Hash array
    for (UINT_t i = 0; i < list_to_check_count; ++i) {
        if (Hash[list_to_check_start_ptr[i]]) {
            common_neighbors++;
        }
    }

    // Unmark elements to prepare for the next intersection operation
    for (UINT_t i = 0; i < list_to_mark_count; ++i) {
        Hash[list_to_mark_start_ptr[i]] = false;
    }

    return common_neighbors;
}

/**
 * @brief Implements a fast sequential triangle counting routine for a graph
 * in Compressed Sparse Row (CSR) format.
 * @param graph A pointer to the graph structure (GRAPH_TYPE).
 * @return The total number of triangles in the graph. Returns (UINT_t)-1 on memory allocation failure.
 */
UINT_t tc_fast_llm(const GRAPH_TYPE *graph) {
    UINT_t triangle_count = 0;
    const UINT_t *Ap = graph->rowPtr; // Row pointers
    const UINT_t *Ai = graph->colInd; // Column indices
    const UINT_t n = graph->numVertices; // Number of vertices

    // Allocate and initialize the boolean array for marking neighbors.
    // This array acts as a hash set, allowing O(1) average-time lookups.
    bool *Hash = (bool *)calloc(n, sizeof(bool));
    if (Hash == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for Hash array in tc_fast.\n");
        return (UINT_t)-1; // Return error indicator
    }

    // Iterate over each vertex 'u'
    for (UINT_t u = 0; u < n; ++u) {
        // Iterate over neighbors 'v' of 'u'
        // Ap[u] and Ap[u+1] define the range of column indices for row u in Ai
        for (UINT_t i = Ap[u]; i < Ap[u+1]; ++i) {
            UINT_t v = Ai[i];

            // Apply direction-oriented check: only consider edges (u, v) where u < v.
            // This ensures each edge is processed exactly once, and contributes to counting
            // triangles consistently (e.g., triangle (1,2,3) with 1<2<3 is implicitly
            // covered when processing edges (1,2), (1,3), and (2,3)).
            if (u < v) {
                // Find common neighbors between 'u' and 'v'. Each common neighbor 'w'
                // forms a triangle (u, v, w). The `intersect_neighbors_count`
                // returns the number of such 'w's.
                triangle_count += intersect_neighbors_count(Ap, Ai, Hash, u, v);
            }
        }
    }

    free(Hash);
    
    // In this approach (iterating edges (u,v) with u<v and finding common neighbors w),
    // each triangle (u,v,w) is counted exactly three times:
    // 1. When considering edge (u,v), finding w.
    // 2. When considering edge (u,w), finding v.
    // 3. When considering edge (v,w), finding u.
    // Therefore, the total count needs to be divided by 3 to get the true number of triangles.
    return triangle_count / 3;
}
