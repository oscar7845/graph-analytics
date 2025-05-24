#include <stdlib.h>      // malloc, calloc, free
#include "types.h"       // UINT_t
#include "graph.h"       // GRAPH_TYPE
#include "tc.h"          // prototype for tc_fast

UINT_t tc_fast_llm(const GRAPH_TYPE *graph) {
    const UINT_t n  = graph->numVertices;
    const UINT_t *Ap = graph->rowPtr;    // CSR row pointers, length n+1
    const UINT_t *Ai = graph->colInd;    // CSR column indices, length numEdges

    // 1) Compute degrees
    UINT_t *deg = (UINT_t*)malloc(n * sizeof(UINT_t));
    if (!deg) return 0;
    for (UINT_t i = 0; i < n; i++) {
        deg[i] = Ap[i+1] - Ap[i];
    }

    // 2) Marker array for fast “is a forward-neighbor?”
    unsigned char *mark = (unsigned char*)calloc(n, sizeof(unsigned char));
    if (!mark) {
        free(deg);
        return 0;
    }

    UINT_t count = 0;

    // 3) For each vertex u, mark its forward neighbors, then intersect
    for (UINT_t u = 0; u < n; u++) {
        // Mark all neighbors v for which (u→v) in our degree‐ordering
        for (UINT_t p = Ap[u]; p < Ap[u+1]; p++) {
            UINT_t v = Ai[p];
            if (deg[u] < deg[v] || (deg[u] == deg[v] && u < v)) {
                mark[v] = 1;
            }
        }

        // For each such forward neighbor v, look at v’s forward neighbors w and test mark[w]
        for (UINT_t p = Ap[u]; p < Ap[u+1]; p++) {
            UINT_t v = Ai[p];
            if (deg[u] < deg[v] || (deg[u] == deg[v] && u < v)) {
                for (UINT_t q = Ap[v]; q < Ap[v+1]; q++) {
                    UINT_t w = Ai[q];
                    // only consider the orientation v→w
                    if ((deg[v] < deg[w] || (deg[v] == deg[w] && v < w))
                        && mark[w]) {
                        count++;
                    }
                }
            }
        }

        // Clear marks for u’s forward neighbors
        for (UINT_t p = Ap[u]; p < Ap[u+1]; p++) {
            UINT_t v = Ai[p];
            if (deg[u] < deg[v] || (deg[u] == deg[v] && u < v)) {
                mark[v] = 0;
            }
        }
    }

    free(mark);
    free(deg);
    return count;
}

