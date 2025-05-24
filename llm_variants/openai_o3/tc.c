#include "types.h"
#include "graph.h"

static inline UINT_t intersect_cnt (const UINT_t *a, UINT_t len_a,
                                    const UINT_t *b, UINT_t len_b)
{
    UINT_t i = 0, j = 0, c = 0;
    while (i < len_a && j < len_b)
    {
        UINT_t x = a[i];
        UINT_t y = b[j];
        if (x == y) { ++c; ++i; ++j; }
        else if (x < y) { ++i; }
        else            { ++j; }
    }
    return c;
}

UINT_t tc_fast_llm (const GRAPH_TYPE *graph)
{
    const UINT_t n = graph->numVertices;
    const UINT_t *restrict Ap = graph->rowPtr;   // CSR row pointer (size n+1)
    const UINT_t *restrict Ai = graph->colInd;   // CSR column index (size 2m)

    // ------------------------------------------------------
    // 1) Compute vertex degrees
    // ------------------------------------------------------
    UINT_t *restrict deg = (UINT_t *) malloc (n * sizeof(UINT_t));
    if (!deg) { fprintf (stderr, "tc_fast: out of memory (deg)\n"); exit (EXIT_FAILURE); }

    for (UINT_t v = 0; v < n; ++v) deg[v] = Ap[v+1] - Ap[v];

    // ------------------------------------------------------
    // 2) Build forward (oriented) adjacency lists in CSR form
    //    The oriented graph contains exactly |E| edges (each undirected edge
    //    kept once), thus memory footprint <= original CSR.
    // ------------------------------------------------------
    UINT_t *restrict Fp = (UINT_t *) malloc ((n+1) * sizeof(UINT_t));
    if (!Fp) { fprintf (stderr, "tc_fast: out of memory (Fp)\n"); exit (EXIT_FAILURE); }

    Fp[0] = 0;
    UINT_t fcnt = 0;
    for (UINT_t u = 0; u < n; ++u)
    {
        UINT_t cnt = 0;
        for (UINT_t p = Ap[u]; p < Ap[u+1]; ++p)
        {
            UINT_t v = Ai[p];
            if ( (deg[u] < deg[v]) || (deg[u] == deg[v] && u < v) ) ++cnt;
        }
        fcnt += cnt;
        Fp[u+1] = fcnt;
    }

    UINT_t *restrict Fi = (UINT_t *) malloc (fcnt * sizeof(UINT_t));
    if (!Fi) { fprintf (stderr, "tc_fast: out of memory (Fi)\n"); exit (EXIT_FAILURE); }

    for (UINT_t u = 0, idx = 0; u < n; ++u)
    {
        for (UINT_t p = Ap[u]; p < Ap[u+1]; ++p)
        {
            UINT_t v = Ai[p];
            if ( (deg[u] < deg[v]) || (deg[u] == deg[v] && u < v) )
                Fi[idx++] = v;           // CSR already sorted ⇒ order preserved
        }
    }

    // ------------------------------------------------------
    // 3) Triangle enumeration by list intersection
    // ------------------------------------------------------
    UINT_t tri = 0;
    for (UINT_t u = 0; u < n; ++u)
    {
        UINT_t u_begin = Fp[u];
        UINT_t u_end   = Fp[u+1];
        UINT_t u_len   = u_end - u_begin;
        const UINT_t *Nu = Fi + u_begin;

        for (UINT_t idx = u_begin; idx < u_end; ++idx)
        {
            UINT_t v = Fi[idx];
            UINT_t v_begin = Fp[v];
            UINT_t v_len   = Fp[v+1] - v_begin;
            const UINT_t *Nv = Fi + v_begin;

            tri += intersect_cnt (Nu, u_len, Nv, v_len);
        }
    }

    // Each triangle counted exactly once → return count
    free (deg);
    free (Fp);
    free (Fi);
    return tri;
}
