#include "types.h"
#include "graph.h" 

UINT_t tc_fast(const GRAPH_TYPE *__restrict graph) {
    GRAPH_TYPE *graph2 = reorder_graph_by_degree(graph, REORDER_HIGHEST_DEGREE_FIRST);
    const UINT_t n = graph2->numVertices;
    const UINT_t *__restrict Ap = graph2->rowPtr;
    const UINT_t *__restrict Ai = graph2->colInd;
    UINT_t count = 0;

    bool *__restrict hash = (bool*)calloc(n, sizeof(bool));
    if (!hash) {
        free_graph(graph2);
        return 0;
    }

    for (UINT_t u = 0; u < n; ++u) {
        const UINT_t u_beg = Ap[u];
        const UINT_t u_end = Ap[u+1];

        // Mark u's neighbors
        for (UINT_t i = u_beg; i < u_end; ++i) hash[Ai[i]] = 1;

        // Process each v > u
        for (UINT_t i = u_beg; i < u_end; ++i) {
            const UINT_t v = Ai[i];
            if (__builtin_expect(v <= u, 0)) continue;

            const UINT_t v_beg = Ap[v];
            const UINT_t v_end = Ap[v+1];
            UINT_t j = v_beg;

            // Binary search for first w > v
            UINT_t l = v_beg, r = v_end;
            while (l < r) {
                UINT_t m = l + (r-l)/2;
                if (Ai[m] <= v) l = m + 1;
                else r = m;
            }
            j = l;

            // Count overlaps in u's hash
            UINT_t local = 0;
            for (; j < v_end; ++j) local += hash[Ai[j]];
            count += local;
        }

        // Reset hash
        for (UINT_t i = u_beg; i < u_end; ++i) hash[Ai[i]] = 0;
    }

    free(hash);
    free_graph(graph2);
    return count;
}