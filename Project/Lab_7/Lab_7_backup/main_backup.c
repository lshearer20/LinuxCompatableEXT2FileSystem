/****************************************************************************
*                   KCW testing ext2 file system                            *
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <libgen.h>
#include <sys/stat.h>
#include <ext2fs/ext2_fs.h>

#include "type.h"

MINODE minode[NMINODE];
MTABLE mtable[NMTABLE];
MINODE *root;
PROC   proc[NPROC], *running;
OFT oft[NOFT];

char   gpath[256]; // global for tokenized components
char   *name[64];  // assume at most 64 components in pathname
int    n;          // number of component strings

int    fd, dev;
int    nblocks, ninodes, bmap, imap, inode_start;
char   line[256], cmd[32], pathname[256],buf[BLKSIZE];

#include "util.c"
#include "cd_ls_pwd.c"

int init()
{
  int i, j;
  MINODE *mip;
  PROC   *p;

  printf("init()\n");

  for (i=0; i<NMINODE; i++){ //initialize all minodes as free
    mip = &minode[i];
    mip->dev = mip->ino = 0;
    mip->refCount = 0;
    mip->mounted = 0;
    mip->mptr = 0;
  }
  for(i=0; i<NMTABLE; i++)
  {
    mtable[i].dev = 0;
  }
  for(i=0; i<NOFT; i++)
  {
    oft[i].refCount = 0;
  }
  for (i=0; i<NPROC; i++){ //initialize PROCS
    p = &proc[i];
    p->pid = i;
    p->uid = 0;
    p->cwd = 0;
    p->status = FREE;
    for (j=0; j<NFD; j++)
      p->fd[j] = 0;
  }
}

// load root INODE and set root pointer to it
int mount_root()
{  
  printf("mount_root()\n");
  root = iget(dev, 2);
}

char *disk = "newdisk";
char *rootdev = "newdisk"; 

int main(int argc, char *argv[ ])
{
  MTABLE *mp;
  int ino;
  char buf[BLKSIZE];
  if (argc > 1)
    disk = argv[1];


  printf("checking EXT2 FS ....");
  if ((fd = open(disk, O_RDWR)) < 0){
    printf("open %s failed\n", disk);  exit(1);
  }
  dev = fd;
  /********** read super block at 1024 ****************/
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;

  /* verify it's an ext2 file system *****************/
  if (sp->s_magic != 0xEF53){
      printf("magic = %x : %s is not an ext2 filesystem\n", sp->s_magic, rootdev);
      exit(1);
  }   
  // fill mout table mtable [0] with rootdev information
  mp = &mtable[0];     // use mtable[0]  
  mp->dev = dev;
  ninodes = mp->ninodes = sp->s_inodes_count;
  nblocks = mp->nblocks = sp->s_blocks_count;
  strcpy(mp->devName, rootdev);
  strcpy(mp->mntName, "/");

  printf("total sp inodes = %d total sp blocks = %d\n", ninodes, nblocks); //total sp inodes and total blocks
  nblocks = mp->free_blocks = sp->s_free_blocks_count;
  ninodes = mp->free_inodes = sp->s_free_inodes_count;
 
  printf("free sp ninodes = %d free sp nblocks = %d\n", ninodes, nblocks); //free sp inodes and free sp blocks



  get_block(dev, 2, buf); 
  gp = (GD *)buf;

  bmap = mp->bmap = gp->bg_block_bitmap;
  imap = mp->imap = gp->bg_inode_bitmap;
  inode_start = mp->iblock = gp->bg_inode_table;
  printf("bmp=%d imap=%d inode_start = %d\n", bmap, imap, inode_start);

  init();  
  mount_root();
  
  printf("mount : %s mounted on\n", rootdev);
  running = &proc[0]; //we are in running process 0
  running->status = READY; //process status is ready
  root = running->cwd = iget(dev, 2); //current working directory of process is 
  for(int i=0; i<NPROC; i++) // set PROC's CWD
    proc[i].cwd = iget(dev, 2);
  
  printf("root refCount = %d\n", root->refCount);

  //printf("hit a key to continue : "); getchar();
  while(1){
    printf("input command : [ls|cd|pwd|mkdir|creat|quit] ");
    fgets(line, 128, stdin);
    line[strlen(line)-1] = 0;
    if (line[0]==0)
      continue;
    pathname[0] = 0;
    cmd[0] = 0;
    
    sscanf(line, "%s %s", cmd, pathname);
    printf("cmd=%s pathname=%s\n", cmd, pathname);

    if (strcmp(cmd, "ls")==0)
       list_file();
    if (strcmp(cmd, "cd")==0)
       change_dir();
    if (strcmp(cmd, "pwd")==0)
       pwd(running->cwd);
    if (strcmp(cmd, "quit")==0)
       quit();
    if (strcmp(cmd, "mkdir")==0)
       mk_dir(0);
    if (strcmp(cmd, "creat")==0)
       mk_dir(1);
    if (strcmp(cmd, "rmdir")==0)
       rm_dir(0);

  }
}
 
int quit()
{
  int i;
  MINODE *mip;
  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    if (mip->refCount > 0)
      iput(mip);
  }
  exit(0);
}

