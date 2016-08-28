/*********************************************************************
 * This file is distributed as part of the C++ port of the APRIL tags
 * library. The code is licensed under GPLv2.
 *
 * Original author: Edwin Olson <ebolson@umich.edu>
 * C++ port and modifications: Matt Zucker <mzucker1@swarthmore.edu>
 ********************************************************************/

#ifndef _UNIONFINDSIMPLE_H_
#define _UNIONFINDSIMPLE_H_

#include <vector>

// Implement a union find algorithm (see original April tags paper for
// details).

class UnionFindSimple {
public:

  std::vector<int> data;

  enum { SZ = 2 };

  /** @param maxid The maximum node id that will be referenced. **/
  UnionFindSimple(int maxid);

  int getSetSize(int id);

  int getRepresentative(int id);

  int connectNodes(int aid, int bid);

};

#endif
