#include "types.h"
#include "graph.h" 
#include "queue.h"

UINT_t tc_fast_llm(const GRAPH_TYPE *graph) {
  /* Bader's new algorithm for triangle counting based on BFS */
  /* Uses Hash array to detect triangles (v, w, x) if x is adjacent to v */
  /* For level, 0 == unvisited. Needs a modified BFS starting from level 1 */
  /* Mark horizontal edges during BFS */
  /* Use directionality to only use one counter for triangles where v < w < x */
  /* Direction orientied. */
  UINT_t* restrict level;
  UINT_t count;
  register UINT_t x;
  bool *Hash;
  bool *horiz;
  bool *visited;
  const UINT_t *restrict Ap = graph->rowPtr;
  const UINT_t *restrict Ai = graph->colInd;
  const UINT_t n = graph->numVertices;
  const UINT_t m = graph->numEdges;

  level = (UINT_t *)calloc(n, sizeof(UINT_t));
  assert_malloc(level);
  
  visited = (bool *)calloc(n, sizeof(bool));
  assert_malloc(visited);

  Hash = (bool *)calloc(n, sizeof(bool));
  assert_malloc(Hash);

  horiz = (bool *)malloc(m * sizeof(bool));
  assert_malloc(horiz);

  Queue *queue = createQueue(n);

  count = 0;
  for (UINT_t v = 0; v < n; v++) {
    if (!level[v])
      bfs_mark_horizontal_edges(graph, v, level, queue, visited, horiz);
    const UINT_t s = Ap[v  ];
    const UINT_t e = Ap[v+1];
    const UINT_t l = level[v];

    for (UINT_t j = s; j < e; j++)
      Hash[Ai[j]] = true;
    
    for (UINT_t j = s; j < e; j++) {
      if (horiz[j]) {
	const UINT_t w = Ai[j];
	if (v < w) {
	  for (UINT_t k = Ap[w]; k < Ap[w+1]; k++) {
	    x = Ai[k];
	    if (Hash[x]) {
	      if ( (l!= level[x]) || ((l == level[x]) && (w < x)) ) {
		count++;
	      }
	    }
	  }
	}
      }
    }

    for (UINT_t j = s; j < e; j++)
      Hash[Ai[j]] = false;
  }

  free_queue(queue);

  free(Hash);
  free(visited);
  free(level);
  free(horiz);

  return count;
}
