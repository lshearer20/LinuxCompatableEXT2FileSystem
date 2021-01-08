/*************** type.h file ************************/
typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

typedef struct ext2_super_block SUPER;
typedef struct ext2_group_desc  GD;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;

SUPER *sp;
GD    *gp;
INODE *ip;
DIR   *dp;  
//MTABLE *mp;
 

#define FREE        0
#define READY       1

#define BLKSIZE  1024
#define NMINODE    64
#define NMTABLE    10
#define NFD         8
#define NPROC       2
#define NOFT       40

typedef struct minode{
  INODE INODE;
  int dev, ino;
  int refCount;
  int dirty;
  // for level-3
  int mounted;
  struct mntable *mptr;
}MINODE;

typedef struct oft{
  int  mode;
  int  refCount;
  MINODE *mptr;
  int  offset;
}OFT;

typedef struct proc{
  struct proc *next;
  int          pid;
  int          uid;
  int          status;
  MINODE      *cwd;
  OFT         *fd[NFD];
}PROC;

typedef struct mtable{
  int dev;  //device number
  int ninodes;  //from superblock
  int nblocks;
  int free_blocks; //from superblock and gd
  int free_inodes; 
  int bmap;        //from gd
  int imap;
  int iblock;   //inodes start block
  MINODE *mntDirPtr;  //mount point dir pointer
  char devName[64];    //device name
  char mntName[64];    //mount point dir name
}MTABLE;
