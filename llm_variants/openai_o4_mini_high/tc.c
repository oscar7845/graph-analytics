#include "types.h"
#include "graph.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

UINT_t tc_fast_llm(const GRAPH_TYPE *graph) {
    const UINT_t n    = graph->numVertices;
    const UINT_t *RP  = graph->rowPtr;   // CSR row pointers
    const UINT_t *CI  = graph->colInd;   // CSR column indices

    // 1) Compute degree of each vertex.
    UINT_t *degree = malloc(n * sizeof(UINT_t));
    assert(degree);
    for (UINT_t u = 0; u < n; ++u) {
        degree[u] = RP[u+1] - RP[u];
    }

    // 2) Compute out‐degree in the degree‐ordering (forward) orientation.
    //    outDeg[u] = |{ v ∈ N(u) : (deg[u]<deg[v]) OR (deg[u]==deg[v] && u<v) }|
    UINT_t *outDeg = calloc(n, sizeof(UINT_t));
    assert(outDeg);
    UINT_t totalOut = 0;
    for (UINT_t u = 0; u < n; ++u) {
        for (UINT_t p = RP[u]; p < RP[u+1]; ++p) {
            UINT_t v = CI[p];
            if (degree[u] < degree[v] ||
               (degree[u] == degree[v] && u < v)) {
                outDeg[u]++;
            }
        }
        totalOut += outDeg[u];
    }

    // 3) Build a new CSR for the oriented graph.
    UINT_t *ORP = malloc((n+1) * sizeof(UINT_t));
    assert(ORP);
    ORP[0] = 0;
    for (UINT_t u = 0; u < n; ++u) {
        ORP[u+1] = ORP[u] + outDeg[u];
    }

    UINT_t *OCI = malloc(totalOut * sizeof(UINT_t));
    assert(OCI);
    // reuse outDeg[] as write‐cursor into OCI
    memcpy(outDeg, ORP, n * sizeof(UINT_t));
    for (UINT_t u = 0; u < n; ++u) {
        for (UINT_t p = RP[u]; p < RP[u+1]; ++p) {
            UINT_t v = CI[p];
            if (degree[u] < degree[v] ||
               (degree[u] == degree[v] && u < v)) {
                OCI[outDeg[u]++] = v;
            }
        }
    }

    // 4) Triangle counting: for each directed edge u→v, intersect
    //    the forward lists of u and v.
    UINT_t triangles = 0;
    for (UINT_t u = 0; u < n; ++u) {
        UINT_t u_start = ORP[u], u_end = ORP[u+1];
        for (UINT_t e = u_start; e < u_end; ++e) {
            UINT_t v = OCI[e];
            // two‐pointer intersection of OCI[ORP[u]..ORP[u+1]) and
            //                        OCI[ORP[v]..ORP[v+1])
            UINT_t i = u_start, j = ORP[v];
            UINT_t i_end = u_end, j_end = ORP[v+1];
            while (i < i_end && j < j_end) {
                UINT_t ni = OCI[i], nj = OCI[j];
                if (ni < nj) {
                    ++i;
                } else if (nj < ni) {
                    ++j;
                } else {
                    // match ⇒ one triangle (u,v,ni)
                    ++triangles;
                    ++i; ++j;
                }
            }
        }
    }

    // 5) Clean up
    free(degree);
    free(outDeg);
    free(ORP);
    free(OCI);

    return triangles;
}
