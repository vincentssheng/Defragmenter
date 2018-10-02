// Author Sean Davis
#ifndef defragmenterH
  #define defragmenterH

#include "mynew.h"
#include "DefragRunner.h"
#include "QuadraticProbing.h"
#include "BinaryHeap.h"

class Defragmenter
{
public:
  Defragmenter(DiskDrive *diskDrive);
  void defrag(DiskDrive *diskDrive);
}; // class Defragmenter

#endif
