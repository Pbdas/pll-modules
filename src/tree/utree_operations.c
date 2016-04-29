#include "pll_tree.h"

static void utree_nodes_at_dist(pll_utree_t * node,
                                pll_utree_t ** outbuffer,
                                unsigned int * index,
                                unsigned int distance,
                                unsigned int depth,
                                int fixed);



/******************************************************************************/
/* Topological operations */

PLL_EXPORT int pll_utree_bisect(pll_utree_t * edge,
                                pll_utree_t ** parent_subtree,
                                pll_utree_t ** child_subtree)
{
  assert(parent_subtree);
  assert(child_subtree);

  pll_utree_t * aux_tree;

  if (!edge->next)
    return PLL_FAILURE;

  pll_utree_t * c_edge = edge->back;

  /* connect parent subtree */
  (*parent_subtree) = edge->next->back;
  aux_tree = edge->next->next->back;

  pll_utree_connect_nodes(*parent_subtree,
                      aux_tree,
                      (*parent_subtree)->length + aux_tree->length);

  edge->next->pmatrix_index = edge->next->next->pmatrix_index;

  /* connect child subtree */
  (*child_subtree) = c_edge->next->back;
  aux_tree = c_edge->next->next->back;

  pll_utree_connect_nodes(*child_subtree,
                      aux_tree,
                      (*child_subtree)->length + aux_tree->length);

  c_edge->next->pmatrix_index = c_edge->next->next->pmatrix_index;

  return PLL_SUCCESS;
}

/**
 * Reconnects two subtrees by adding 2 new nodes and 1 edge.
 *
 * Adds 1 new edge connecting edges \p edge.parent and \p edge.child with
 * length \p edge.length.
 *
 *   A       C         A              C
 *   |       |  ---->   \            /
 *                       e1--edge--e2
 *   |       |          /            \
 *   B       D         B              D
 *   A,B,C,D are subtrees
 *
 * @param[in] edge                 new edge
 * @param[in] parent_pmatrix_index matrix index of e1-A
 * @param[in] parent_clv_index     clv index of e1
 * @param[in] parent_scaler_index  scaler index of e1
 * @param[in] child_pmatrix_index  matrix index of e2-C
 * @param[in] child_clv_index      clv index of e2
 * @param[in] child_scaler_index   scaler index of e2
 * @param[in] edge_pmatrix_index   matrix index of e1-e2
 *
 * @returns the new created edge
 */
PLL_EXPORT pll_tree_edge_t pll_utree_reconnect(pll_tree_edge_t * edge,
                                               pll_utree_t * pruned_edge)
{
  /* create and connect 2 new nodes */
  pll_utree_t *parent_node, *child_node;
  assert(pruned_edge->back);

  parent_node = pruned_edge;
  child_node  = pruned_edge->back;
  assert(parent_node->back == child_node && child_node->back == parent_node);

  assert(!pll_utree_is_tip(parent_node));
  assert(!pll_utree_is_tip(child_node));

  pll_tree_edge_t new_edge;
  new_edge.edge.utree.child = child_node;
  new_edge.length = edge->length;

  /* set length */
  pll_utree_set_length(parent_node, edge->length);

  /* reconnect parent close to edge.parent */
  pll_utree_connect_nodes(parent_node->next->next,
                          edge->edge.utree.parent->back,
                          edge->edge.utree.parent->back->length);
  //parent_node->next->pmatrix_index = parent_pmatrix_index;
  pll_utree_connect_nodes(edge->edge.utree.parent,
                          parent_node->next,
                          0);

  /* reconnect child close to edge.child */
  pll_utree_connect_nodes(child_node->next->next,
                          edge->edge.utree.child->back,
                          edge->edge.utree.child->back->length);
  //child_node->next->pmatrix_index = child_pmatrix_index;
  pll_utree_connect_nodes(edge->edge.utree.child,
                          child_node->next,
                          0);

  return new_edge;
}

PLL_EXPORT pll_utree_t * pll_utree_prune(pll_utree_t * edge)
{
  pll_utree_t *edge1, *edge2;

  assert(edge);
  if (!edge->next)
  {
    /* invalid node */
    snprintf (pll_errmsg, 200, "Attempting to prune a tip node");
    pll_errno = PLL_ERROR_SPR_INVALID_NODE;
    return NULL;
  }

  edge1 = edge->next->back;
  edge2 = edge->next->next->back;
  pll_utree_connect_nodes(edge1, edge2, edge1->length + edge2->length);

  edge->next->back = edge->next->next->back = NULL;

  return edge1;
}

PLL_EXPORT int pll_utree_regraft(pll_utree_t * edge,
                                 pll_utree_t * tree)
{
  pll_utree_t *edge1, *edge2;
  double new_length;

  assert(edge && tree);
  if (!edge->next)
      {
        /* invalid node */
        snprintf (pll_errmsg, 200, "Attempting to regraft a tip node");
        pll_errno = PLL_ERROR_SPR_INVALID_NODE;
        return PLL_FAILURE;
      }
  if (edge->next->back || edge->next->next->back)
  {
    /* invalid node */
    snprintf (pll_errmsg, 200, "Attempting to regraft a connected node");
    pll_errno = PLL_ERROR_SPR_INVALID_NODE;
    return PLL_FAILURE;
  }

  edge1      = tree;
  edge2      = tree->back;
  new_length = tree->length/2;
  pll_utree_connect_nodes(edge1, edge->next,       new_length);
  pll_utree_connect_nodes(edge->next->next, edge2, new_length);

  return PLL_SUCCESS;
}

/**
 * Interchanges 2 edges, represented by 2 internal nodes
 *
 * CLV and scaler indices, and labels are interchanged between nodes to match
 * the other 2 nodes in the triplet.
 *
 * @returns true, if the move was applied correctly
 */
PLL_EXPORT int pll_utree_interchange(pll_utree_t * node1,
                                     pll_utree_t * node2)
{
  pll_utree_t *next1 = node2->back;
  pll_utree_t *next2 = node1->back;

  pll_utree_connect_nodes(node1, next1, next1->length);
  pll_utree_connect_nodes(node2, next2, next2->length);

  return PLL_SUCCESS;
}

/**
 * @brief Creates a new circular node
 *
 *           n2
 *          / |
 *        n1  |
 *          \ |
 *           n3
 *
 * All parameters are shared among the nodes in the triplet
 *
 * @param clv_index    the clv_index
 * @param scaler_index the scaler index
 * @param label        the node label
 * @param data         the data pointer
 *
 * @returns the new node
 */
PLL_EXPORT pll_utree_t * pll_utree_create_node(unsigned int clv_index,
                                               int scaler_index,
                                               char * label,
                                               void * data)
{
  pll_utree_t * new_node = (pll_utree_t *)calloc(1, sizeof(pll_utree_t));
  new_node->next         = (pll_utree_t *)calloc(1, sizeof(pll_utree_t));
  new_node->next->next   = (pll_utree_t *)calloc(1, sizeof(pll_utree_t));
  new_node->next->next->next = new_node;
  new_node->label = label;
  new_node->next->label = new_node->next->next->label = new_node->label;
  new_node->next->data = new_node->next->next->data = new_node->data = data;
  new_node->next->length = new_node->next->next->length = new_node->length = 0;
  new_node->next->clv_index = new_node->next->next->clv_index = new_node->clv_index = clv_index;
  new_node->next->scaler_index = new_node->next->next->scaler_index = new_node->scaler_index = scaler_index;
  new_node->back = new_node->next->back = new_node->next->next->back = NULL;
  return new_node;
}

/**
 * connects 2 nodes
 */
PLL_EXPORT int pll_utree_connect_nodes(pll_utree_t * parent,
                                       pll_utree_t * child,
                                       double length)
{
  if(!(parent && child))
    return PLL_FAILURE;

  parent->back = child;
  child->back = parent;
  pll_utree_set_length(parent, length);

  /* PMatrix index is set to parent node */
  child->pmatrix_index = parent->pmatrix_index;

  return PLL_SUCCESS;
}




/**
 * Bisects the tree by removing one edge
 *
 * Removes the edge \p edge and frees the nodes defining that edge.
 * Reconnects the subtrees at the sides of the edge (figure below).
 * The branch lengths of the new edges are the sum of the removed ones.
 * The join branch contains the pmatrix index of the parent edges
 * The removed pmatrix indices are returned in the field
 *     'additional_pmatrix_index' of both output subtrees
 *
 * Returns the new parent and child edges, where parent is the closest to \p edge.
 *
 *   A            C              A        C
 *    \___edge___/       ---->   |        |
 *    /          \               |        |
 *   B            D              B        D
 *   A,B,C,D are subtrees
 *
 * @param[in] edge edge to remove
 * @param[out] parent_subtree edge corresponding to the 'edge' subtree
 * @param[out] child_subtree  edge corresponding to the 'edge->back' subtree
 * @returns PLL_SUCCESS if OK
 */



/******************************************************************************/
/* Topological search */

/**
 * Returns the list of nodes at a certain distance from a specified edge
 *
 * @param[in] root the root edge
 * @param[out] outbuffer the list of nodes. Outbuffer should be allocated
 * @param[out] n_nodes the number of nodes returned in \p outbuffer
 * @param[in] distance the maximum distance to check
 * @param[in] fixed if true, returns only the nodes at distance \p distance,
 *            otherwise, the nodes at distance <= \p distance.
 */
PLL_EXPORT int pll_utree_nodes_at_edge_dist(pll_utree_t * root,
                                            pll_utree_t ** outbuffer,
                                            unsigned int * n_nodes,
                                            unsigned int distance,
                                            int fixed)
{
  unsigned int depth = 0;
  if (!root->next) return PLL_FAILURE;

  *n_nodes = 0;

  /* we will traverse an unrooted tree in the following way

       3          1
        \        /
         * ---- *
        /        \
       4          2
   */

  utree_nodes_at_dist(root->back, outbuffer, n_nodes, distance, depth+1, fixed);
  utree_nodes_at_dist(root, outbuffer, n_nodes, distance, depth, fixed);

  return PLL_SUCCESS;
}

/**
 * Returns the list of nodes at a certain distance from a specified node
 *
 * @param[in] node the root node
 * @param[out] outbuffer the list of nodes. Outbuffer should be allocated
 * @param[out] n_nodes the number of nodes returned in \p outbuffer
 * @param[in] distance the maximum distance to check
 * @param[in] fixed if true, returns only the nodes at distance \p distance,
 *            otherwise, the nodes at distance <= \p distance.
 */
PLL_EXPORT int pll_utree_nodes_at_node_dist(pll_utree_t * node,
                                 pll_utree_t ** outbuffer,
                                 unsigned int * n_nodes,
                                 unsigned int distance,
                                 int fixed)
{
  unsigned int depth = 0;
  if (!node->next) return PLL_FAILURE;

  *n_nodes = 0;

  /* we will traverse an unrooted tree in the following way

               1
             /
          --*
             \
               2
    */

  utree_nodes_at_dist(node, outbuffer, n_nodes, distance, depth, fixed);

  return PLL_SUCCESS;
}



/******************************************************************************/
/* static functions */

static void utree_nodes_at_dist(pll_utree_t * node,
                                pll_utree_t ** outbuffer,
                                unsigned int * index,
                                unsigned int distance,
                                unsigned int depth,
                                int fixed)
{
  if (depth == distance || !fixed)
  {
    outbuffer[*index] = node;
    *index = *index + 1;
  }

  if (depth >= distance || !(node->next)) return;

  utree_nodes_at_dist(node->next->back, outbuffer, index,
                      distance, depth+1, fixed);
  utree_nodes_at_dist(node->next->next->back, outbuffer, index,
                      distance, depth+1, fixed);
}