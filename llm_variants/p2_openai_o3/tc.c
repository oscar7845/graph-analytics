#include "types.h"
#include "graph.h"

/* Forward-degree orientation + hybrid (merge≤32 vs stamp-hash) intersections.
   Aux arrays: deg[n], fwd_start[n+1], fwd_edges[oriented_m], visited[n] */

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "types.h"
#include "graph.h"

UINT_t tc_fast(const GRAPH_TYPE *__restrict graph)
{
    const UINT_t n = graph->numVertices;
    const UINT_t * __restrict row_ptr = graph->rowPtr;
    const UINT_t * __restrict col_ind = graph->colInd;

    UINT_t *deg = (UINT_t *)malloc(n * sizeof(UINT_t));
    if (!deg) return 0;
    for (UINT_t i = 0; i < n; ++i)
        deg[i] = row_ptr[i + 1] - row_ptr[i];

    UINT_t *fwd_start = (UINT_t *)malloc((n + 1) * sizeof(UINT_t));
    if (!fwd_start) { free(deg); return 0; }
    fwd_start[0] = 0;
    for (UINT_t u = 0; u < n; ++u) {
        UINT_t cnt = 0;
        for (UINT_t p = row_ptr[u]; p < row_ptr[u + 1]; ++p) {
            UINT_t v = col_ind[p];
            if (deg[u] < deg[v] || (deg[u] == deg[v] && u < v)) ++cnt;
        }
        fwd_start[u + 1] = fwd_start[u] + cnt;
    }

    const UINT_t oriented_m = fwd_start[n];
    UINT_t *fwd_edges = (UINT_t *)malloc(oriented_m * sizeof(UINT_t));
    if (!fwd_edges) { free(deg); free(fwd_start); return 0; }

    for (UINT_t u = 0; u < n; ++u) {
        UINT_t pos = fwd_start[u];
        for (UINT_t p = row_ptr[u]; p < row_ptr[u + 1]; ++p) {
            UINT_t v = col_ind[p];
            if (deg[u] < deg[v] || (deg[u] == deg[v] && u < v))
                fwd_edges[pos++] = v;
        }
    }

    uint32_t *visited = (uint32_t *)malloc(n * sizeof(uint32_t));
    if (!visited) { free(deg); free(fwd_start); free(fwd_edges); return 0; }
    memset(visited, 0, n * sizeof(uint32_t));
    uint32_t mark = 1;

    UINT_t triangles = 0;

    for (UINT_t u = 0; u < n; ++u) {
        const UINT_t u_start = fwd_start[u];
        const UINT_t u_end   = fwd_start[u + 1];
        const UINT_t du      = u_end - u_start;

        for (UINT_t ei = u_start; ei < u_end; ++ei) {
            const UINT_t v = fwd_edges[ei];
            const UINT_t v_start = fwd_start[v];
            const UINT_t v_end   = fwd_start[v + 1];
            const UINT_t dv      = v_end - v_start;

            const UINT_t *small_ptr, *big_ptr;
            UINT_t small_len, big_len;
            if (du < dv) {
                small_ptr = &fwd_edges[u_start];
                small_len = du;
                big_ptr   = &fwd_edges[v_start];
                big_len   = dv;
            } else {
                small_ptr = &fwd_edges[v_start];
                small_len = dv;
                big_ptr   = &fwd_edges[u_start];
                big_len   = du;
            }

            if (small_len <= 32) {
                UINT_t i = 0, j = 0;
                const UINT_t *pu = &fwd_edges[u_start];
                const UINT_t *pv = &fwd_edges[v_start];
                while (i < du && j < dv) {
                    UINT_t a = pu[i], b = pv[j];
                    if (a == b) { ++triangles; ++i; ++j; }
                    else if (a < b) ++i;
                    else ++j;
                }
            } else {
                ++mark;
                if (mark == 0) { memset(visited, 0, n * sizeof(uint32_t)); mark = 1; }
                for (UINT_t k = 0; k < big_len; ++k) visited[big_ptr[k]] = mark;
                for (UINT_t k = 0; k < small_len; ++k)
                    if (visited[small_ptr[k]] == mark) ++triangles;
            }
        }
    }

    free(visited);
    free(fwd_edges);
    free(fwd_start);
    free(deg);
    return triangles;
}
