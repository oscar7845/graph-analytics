#include "types.h"
#include "graph.h" // For GRAPH_TYPE definition and access to rowPtr, colInd

UINT_t tc_fast_llm(const GRAPH_TYPE *graph) {
    const UINT_t *restrict Ap = graph->rowPtr;
    const UINT_t *restrict Ai = graph->colInd;
    const UINT_t n = graph->numVertices;
    UINT_t count = 0;

    // Iterate over each vertex v
    for (UINT_t v = 0; v < n; v++) {
        for (UINT_t i = Ap[v]; i < Ap[v + 1]; i++) {
            UINT_t w = Ai[i]; // w is a neighbor of v
            if (v < w) { // Ensure v < w to avoid counting twice
                // Merge path intersection for v and w
                UINT_t ptr1 = Ap[v];
                UINT_t ptr2 = Ap[w];

                while (ptr1 < Ap[v + 1] && ptr2 < Ap[w + 1]) {
                    if (Ai[ptr1] == Ai[ptr2]) {
                        // Common neighbor found, count triangle
                        count++;
                        ptr1++;
                        ptr2++;
                    } else if (Ai[ptr1] < Ai[ptr2]) {
                        ptr1++;
                    } else {
                        ptr2++;
                    }
                }
            }
        }
    }

    // Since each triangle is counted 3 times (once per vertex), divide by 3
    return count / 3;
}
