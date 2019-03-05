#ifndef AOFS_H
#define AOFS_H

#define MAGICUM 69
#define

typedef Superblock {
  uint32_t magicnum;
  uint32_t totalblocks;
  uint32_t bitmap[];
  
  

} Superblock;



#endif



