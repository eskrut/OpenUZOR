#ifndef REORDERER_H
#define REORDERER_H

#include <vector>

class sbfMesh;

/**
 * @brief The Reorderer class use METIS libbrary to perform
 * nodes renumbering in sbfMesh, sbfSElement
 */

class Reorderer
{
public:
    Reorderer();
    ~Reorderer();
public:
    using ComputeResult = std::pair<std::vector<int> /*perm*/, std::vector<int> /*iperm*/>;
    //! Reordering routines
    //@{
    /**
     * @brief renumber Most general routine. Reorder graph on base of CSR format.
     * @param rowShifts Array of indexes to access columns IDs of sertain row.
     * @param columnIDs Array of columns IDs of all rows sorted by row ID.
     * @param makeReport flag to make report on progress.
     * @return pair of vectors. First one is original IDs of reordered graph.
     * iperm is new IDs of nodes after reordering.
     */
    ComputeResult computeReordering ( const std::vector<int> &rowShifts,
                                      const std::vector<int> &columnIDs,
                                      bool makeReport = true ) const;
    /**
     * @brief renumber This function computes reordering arrays.
     * @param mesh sbfMesh to perform reordering.
     * @param makeReport flag to make report on progress.
     * @return pair of vectors. perm is original IDs of reordered mesh.
     * iperm is new IDs of nodes after reordering.
     */
    ComputeResult computeReordering ( const sbfMesh *mesh,
                                      bool makeReport = true ) const;
    //@}
    void makeReordering ( sbfMesh *mesh, bool makeReport = true ) const;
};

#endif // REORDERER_H
