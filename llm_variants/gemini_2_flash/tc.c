#include "types.h"
#include "graph.h" 

UINT_t tc_fast_llm(const GRAPH_TYPE *graph) {
    UINT_t count = 0;
    const UINT_t *restrict Ap = graph->rowPtr;
    const UINT_t *restrict Aj = graph->colInd;
    const UINT_t n = graph->numVertices;

    for (UINT_t i = 0; i < n; ++i) {
        for (UINT_t j = Ap[i]; j < Ap[i + 1]; ++j) {
            UINT_t k = Aj[j];
            if (k > i) { // Avoid duplicate counting
                UINT_t p1 = Ap[i], p2 = Ap[k];
                while (p1 < Ap[i + 1] && p2 < Ap[k + 1]) {
                    if (Aj[p1] == Aj[p2]) { // Common neighbor found
                        count++;
                        p1++;
                        p2++;
                    } else if (Aj[p1] < Aj[p2]) {
                        p1++;
                    } else {
                        p2++;
                    }
                }
            }
        }
    }
    return count;
}
