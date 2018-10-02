#include "defragmenter.h"
#include "DefragRunner.h"
#include "mynew.h"
#include "QuadraticProbing.h"
#include "BinaryHeap.h"
#include "vector.h"

Defragmenter::Defragmenter(DiskDrive *diskDrive)
{
  defrag(diskDrive);
}

void Defragmenter::defrag(DiskDrive *diskDrive)
{
  int maxSize = 207400;
  QuadraticHashTable<DiskBlock*>* temp = new QuadraticHashTable<DiskBlock*>(NULL, 500);
  QuadraticHashTable<int>* posTracker = new QuadraticHashTable<int>((unsigned) -1, 20000);
  BinaryHeap<unsigned>* freeTracker = new BinaryHeap<unsigned>(maxSize);
  unsigned maxEmpty;
  for(int i = diskDrive->getCapacity() - 1; i >= 2; i--)
  {
    if(diskDrive->FAT[i] == false)
      freeTracker->insert(i);
  }
  unsigned j = 2;
  unsigned prev = 0;
  DiskBlock* CB = NULL; //current Block
  DiskBlock* DB = NULL; //this holds the block we have to move
  for(int i = 0; i < diskDrive->getNumFiles(); i++)
  {
    unsigned curBlock = diskDrive->directory[i].getFirstBlockID();
    unsigned numBlocksOccupied =  diskDrive->directory[i].getSize();
    for(unsigned k = j; k < (j + numBlocksOccupied); k++)
    {
      if(k == curBlock) //if block already at k
      {
        CB = diskDrive->readDiskBlock(k);
        curBlock = CB->getNext(); //k == curBlock
        if(k == j + numBlocksOccupied - 1)
          diskDrive->directory[i].setFirstBlockID(j);
      //  else
        //  CB->setNext(k + 1);
        //diskDrive->writeDiskBlock(CB, k);
        delete CB;
      } //if block is already at k
      else //if k != curBlock
      {
        if(curBlock > k) //if curBlock has not been moved
        {
          if(diskDrive->FAT[k] == true) //if FAT[k] == true
          {
            if(temp->currentSize < temp->array.size() - 90) //if temp is not full
            {
              DB = diskDrive->readDiskBlock(k);
              temp->insert(DB, k);
              CB = diskDrive->readDiskBlock(curBlock);
              diskDrive->FAT[curBlock] = false;
              if(freeTracker->currentSize < maxSize)
                freeTracker->insert(curBlock);
              curBlock = CB->getNext(); //curBlock > k && FAT[k] == true && temp not full
              if(k == j + numBlocksOccupied - 1)
                diskDrive->directory[i].setFirstBlockID(j);
            //  else
                //CB->setNext(k+1);
              diskDrive->writeDiskBlock(CB, k);
              delete CB;
            } //if temp is not full
            else //if temp is full
            {
              DB = diskDrive->readDiskBlock(k);
              while(diskDrive->FAT[freeTracker->findMax()] == true)
                freeTracker->deleteMax();
              maxEmpty = freeTracker->findMax();
              freeTracker->deleteMax();
              posTracker->insert(maxEmpty, k); //curBlock > k && FAT[k] == true && temp full
              diskDrive->writeDiskBlock(DB, maxEmpty);
              diskDrive->FAT[maxEmpty] = true;
              delete DB;
              CB = diskDrive->readDiskBlock(curBlock);
              diskDrive->FAT[curBlock] = false;
              if(freeTracker->currentSize < maxSize)
                freeTracker->insert(curBlock);
              curBlock = CB->getNext();
              if(k == j + numBlocksOccupied - 1)
                diskDrive->directory[i].setFirstBlockID(j);
              //else
                //CB->setNext(k+1);
              diskDrive->writeDiskBlock(CB, k);
              delete CB;
            } //if temp is full
          } //if FAT[k] == true
          else // if FAT[k] == false
          {
            CB = diskDrive->readDiskBlock(curBlock);
            diskDrive->FAT[curBlock] = false; //FAT[k] == false
            if(freeTracker->currentSize < maxSize)
              freeTracker->insert(curBlock);
            curBlock = CB->getNext();
            if(k == j + numBlocksOccupied - 1)
              diskDrive->directory[i].setFirstBlockID(j);
          //  else
              //CB->setNext(k+1);
            diskDrive->writeDiskBlock(CB, k);
            diskDrive->FAT[k] = true;
            delete CB;
          } //if FAT[k] == false
        } //if curBlock has not been moved
        else //if disk has been moved
        {
          CB = temp->find(curBlock);
          if(CB) //if curBlock is in RAM
          {
            temp->remove(curBlock);
            if(diskDrive->FAT[k] == true) // if FAT[k] == true
            {
              if(temp->currentSize < temp->array.size() - 90) // if temp is not full
              {
                DB = diskDrive->readDiskBlock(k);
                temp->insert(DB, k);
                curBlock = CB->getNext(); //in temp && FAT[k] == true && temp not full
                if(k == j + numBlocksOccupied - 1)
                  diskDrive->directory[i].setFirstBlockID(j);
              //  else
                  //CB->setNext(k+1);
                diskDrive->writeDiskBlock(CB, k);
                delete CB;
              }// if temp not full
              else //if temp is full
              {
                DB = diskDrive->readDiskBlock(k);
                while(diskDrive->FAT[freeTracker->findMax()] == true)
                  freeTracker->deleteMax();
                maxEmpty = freeTracker->findMax();
                freeTracker->deleteMax();
                posTracker->insert(maxEmpty, k);
                diskDrive->writeDiskBlock(DB, maxEmpty);
                diskDrive->FAT[maxEmpty] = true; ////in temp && FAT[k] == true && temp full
                delete DB;
                //CB = diskDrive->readDiskBlock(curBlock); this is wrong
                curBlock = CB->getNext();
                if(k == j + numBlocksOccupied - 1)
                  diskDrive->directory[i].setFirstBlockID(j);
                //else
                  //CB->setNext(k+1);
                diskDrive->writeDiskBlock(CB, k);
                delete CB;
              }//if temp is full
            } //if FAT[k] == true
            else //if FAT[k] == false
            {
              curBlock = CB->getNext(); // in temp && FAT[k] == false
              if(k == j + numBlocksOccupied - 1)
                diskDrive->directory[i].setFirstBlockID(j);
              //else
                //CB->setNext(k+1);
              diskDrive->writeDiskBlock(CB, k);
              delete CB;
              diskDrive->FAT[k] = true;
            } //if FAT[k] == false
          }//if curblock is in RAM
          else //if curBlock has been moved in Disk
          {
            do {
              prev = curBlock;
              curBlock = posTracker->find(prev);
              posTracker->remove(prev);
            } while(curBlock != (unsigned) -1);
            CB = diskDrive->readDiskBlock(prev);
            diskDrive->FAT[prev] = false;
            if(freeTracker->currentSize < maxSize)
              freeTracker->insert(prev);
            if(diskDrive->FAT[k] == false) //if k is empty
            {
              curBlock = CB->getNext(); //if curBlock moved and k empty
              if(k == j + numBlocksOccupied - 1)
                diskDrive->directory[i].setFirstBlockID(j);
              //else
                //CB->setNext(k+1);
              diskDrive->writeDiskBlock(CB, k);
              delete CB;
              diskDrive->FAT[k] = true;
            } //if k is empty
            else //if k is not empty
            {
              if(temp->currentSize < temp->array.size() - 90) //if temp is not full
              {
                DB = diskDrive->readDiskBlock(k);
                temp->insert(DB, k);
                curBlock = CB->getNext(); //if temp not full and curBlock moved
                if(k == j + numBlocksOccupied - 1)
                  diskDrive->directory[i].setFirstBlockID(j);
                //else
                  //CB->setNext(k+1);
                diskDrive->writeDiskBlock(CB, k);
                delete CB;
              } //if temp is not full
              else //if temp is full
              {
                while(diskDrive->FAT[freeTracker->findMax()] == true)
                  freeTracker->deleteMax();
                maxEmpty = freeTracker->findMax();
                freeTracker->deleteMax();
                DB = diskDrive->readDiskBlock(k);
                posTracker->insert(maxEmpty, k);
                diskDrive->writeDiskBlock(DB, maxEmpty);
                diskDrive->FAT[maxEmpty] = true; //if temp full and curBlock moved
                delete DB;
                curBlock = CB->getNext();
                if(k == j + numBlocksOccupied - 1)
                  diskDrive->directory[i].setFirstBlockID(j);
              //  else
                  //CB->setNext(k+1);
                diskDrive->writeDiskBlock(CB, k);
                delete CB;
              }//if temp is full
            }//if k is not empty
          } //if curBlock has been moved in Disk
        } //if disk has been moved
      } //if k != curBlock
    } //inner for
    j += numBlocksOccupied;
  } //outer for
} // Defragmenter()
