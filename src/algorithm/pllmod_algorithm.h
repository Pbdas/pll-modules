/*
    Copyright (C) 2016 Diego Darriba

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    Contact: Diego Darriba <Diego.Darriba@h-its.org>,
    Exelixis Lab, Heidelberg Instutute for Theoretical Studies
    Schloss-Wolfsbrunnenweg 35, D-69118 Heidelberg, Germany
*/
#ifndef PLL_ALGORITHM_H_
#define PLL_ALGORITHM_H_

#include "pll_optimize.h"
#include "pll_tree.h"

#define PLLMOD_ALGO_MIN_WEIGHT_RATIO   0.1
#define PLLMOD_ALGO_MAX_WEIGHT_RATIO    10
#define PLLMOD_ALGO_BFGS_FACTR         1e9

/*
 * Optimize stationary frequencies for parameters `params_index`.
 */
PLL_EXPORT double pllmod_algo_opt_frequencies (pll_partition_t * partition,
                                               pll_utree_t * tree,
                                               unsigned int params_index,
                                               unsigned int * params_indices,
                                               double tolerance);

/*
 * Optimize substitution rates for parameters `params_index`.
 * symmetries is an array with as many positions as substitution parameters,
 *            or 'NULL' for GTR (e.g., for DNA, 012345 and NULL are equivalent)
 *            Must be sorted and start with '0'.
 *            e.g., 000000 = JC/F81, 010010 = K80/HKY, 012314 = TrN
 */
PLL_EXPORT double pllmod_algo_opt_subst_rates (pll_partition_t * partition,
                                               pll_utree_t * tree,
                                               unsigned int params_index,
                                               unsigned int * params_indices,
                                               int * symmetries,
                                               double min_rate,
                                               double max_rate,
                                               double tolerance);

PLL_EXPORT double pllmod_algo_opt_alpha (pll_partition_t * partition,
                                         pll_utree_t * tree,
                                         unsigned int * params_indices,
                                         double min_alpha,
                                         double max_alpha,
                                         double *alpha,
                                         double tolerance);

PLL_EXPORT double pllmod_algo_opt_pinv (pll_partition_t * partition,
                                        pll_utree_t * tree,
                                        unsigned int * params_indices,
                                        double min_pinv,
                                        double max_pinv,
                                        double tolerance);

/*
 * Optimize free rates and rate weights together, linked to `partition->rate_cats`.
 * Uses 2 step L-BFGS-B algorithm.
 */
PLL_EXPORT double pllmod_algo_opt_rates_weights (pll_partition_t * partition,
                                                 pll_utree_t * tree,
                                                 unsigned int * params_indices,
                                                 double min_rate,
                                                 double max_rate,
                                                 double tolerance);

#endif