/*********************************************************************
 * This file is distributed as part of the C++ port of the APRIL tags
 * library. The code is licensed under GPLv2.
 *
 * Original author: Edwin Olson <ebolson@umich.edu>
 * C++ port and modifications: Matt Zucker <mzucker1@swarthmore.edu>
 ********************************************************************/

#include "UnionFindSimple.h"

/** @param maxid The maximum node id that will be referenced. **/
UnionFindSimple::UnionFindSimple(int maxid): data(maxid*SZ) {

    for (int i = 0; i < maxid; i++) {
      // everyone is their own cluster of size 1
      data[SZ*i+0] = i;
      data[SZ*i+1] = 1;
    }

  }

  int UnionFindSimple::getSetSize(int id) {
    return data[SZ*getRepresentative(id)+1];
  }

  int UnionFindSimple::getRepresentative(int id) {

    // terminal case: a node is its own parent.
    if (data[SZ*id]==id) {
      return id;
    }

    // otherwise, recurse...
    int root = getRepresentative(data[SZ*id]);

    // short circuit the path.
    data[SZ*id] = root;

    return root;

  }

  int UnionFindSimple::connectNodes(int aid, int bid) {

    int aroot = getRepresentative(aid);
    int broot = getRepresentative(bid);

    if (aroot == broot) {
      return aroot;
    }

    int asz = data[SZ*aroot+1];
    int bsz = data[SZ*broot+1];
    
    if (asz > bsz) {
      data[SZ*broot] = aroot;
      data[SZ*aroot+1] += bsz;
      return aroot;
    } else {
      data[SZ*aroot] = broot;
      data[SZ*broot+1] += asz;
      return broot;
    }

  }

