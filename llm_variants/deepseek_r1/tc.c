#include "types.h"
#include "graph.h" 

UINT_t tc_fast_llm(const GRAPH_TYPE *graph) {
    const UINT_t *Ap = graph->rowPtr;
    const UINT_t *Ai = graph->colInd;
    const UINT_t n = graph->numVertices;

    bool *hash = (bool *)calloc(n, sizeof(bool));
    assert_malloc(hash);

    UINT_t count = 0;

    for (UINT_t u = 0; u < n; u++) {
        // Mark all neighbors of u
        UINT_t u_start = Ap[u];
        UINT_t u_end = Ap[u + 1];
        for (UINT_t i = u_start; i < u_end; i++) {
            UINT_t w = Ai[i];
            hash[w] = true;
        }

        // Check each neighbor v of u where v > u
        for (UINT_t i = u_start; i < u_end; i++) {
            UINT_t v = Ai[i];
            if (v <= u) continue;

            // Look for w in v's neighbors where w > v and hash[w] is true
            UINT_t v_start = Ap[v];
            UINT_t v_end = Ap[v + 1];
            for (UINT_t j = v_start; j < v_end; j++) {
                UINT_t w = Ai[j];
                if (w > v && hash[w]) {
                    count++;
                }
            }
        }

        // Unmark neighbors of u
        for (UINT_t i = u_start; i < u_end; i++) {
            UINT_t w = Ai[i];
            hash[w] = false;
        }
    }

    free(hash);
    return count;
}
