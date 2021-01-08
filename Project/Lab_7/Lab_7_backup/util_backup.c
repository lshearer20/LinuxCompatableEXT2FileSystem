/*********** util.c file ****************/

/**** globals defined in main.c file ****/
extern MINODE minode[NMINODE];    //in memory inodes
extern MINODE *root;
extern PROC proc[NPROC], *running;
extern char gpath[256];
extern char *name[64];
extern int nname = 0, ct = 0;
extern int fd, dev;
extern int nblocks, ninodes, bmap, imap, inode_start;
extern char line[256], cmd[32], pathname[256];
extern char *t1 = "xwrxwrxwr-------";
extern char *t2 = "----------------";
int dummy = 0;
extern int emptyCheck = 0, entryCount = 0, extraDir = 0;

int get_block(int dev, int blk, char *buf)
{
  lseek(dev, (long)blk * BLKSIZE, 0);
  read(dev, buf, BLKSIZE);
  if(n<0) printf("get_block [%d %d] error\n", fd, blk);
}

int put_block(int dev, int blk, char *buf)
{
  lseek(dev, (long)blk * BLKSIZE, 0);
  write(dev, buf, BLKSIZE);
}

int tokenize(char *pathname)
{
  char *s;
  int i;
  int p = 0;
  s = strtok(pathname, "/");
  nname = 0;
  while (s)
  {
    name[p] = s;
    s = strtok(0, "/");
    p++;
    nname++;
  }
  name[p] = "\n";
}

// return minode pointer to loaded INODE
MINODE *iget(int dev, int ino)
{
  int i;
  MINODE *mip;
  MTABLE *mp;
  char buf[BLKSIZE];
  int blk, disp;
  INODE *ip;
  //search in memory inodes first
  for (i = 0; i < NMINODE; i++)
  {
    mip = &minode[i];
    if (mip->refCount && mip->dev == dev && mip->ino == ino)
    {
      mip->refCount++;
      return mip;
    }
  }
  //needed inode not in memory
  for (i = 0; i < NMINODE; i++)
  {
    mip = &minode[i];
    if (mip->refCount == 0) //allocate a free MINODE
    {
      //printf("allocating NEW minode[%d] for [%d %d]\n", i, dev, ino);
      mip->refCount = 1;
      mip->dev = dev;
      mip->ino = ino;

      // get INODE of ino to buf
      blk = (ino - 1) / 8 + inode_start;
      disp = (ino - 1) % 8;

      //printf("iget: ino=%d blk=%d disp=%d\n", ino, blk, disp);

      get_block(dev, blk, buf);
      ip = (INODE *)buf + disp;
      // copy INODE to mp->INODE
      mip->INODE = *ip;

      return mip;
    }
  }
  printf("PANIC: no more free minodes\n");
  return 0;
}

iput(MINODE *mip)
{
  int i, block, offset;
  char buf[BLKSIZE];
  INODE *ip;

  if (mip == 0)
    return;

  mip->refCount--;

  if (mip->refCount > 0) //still has user
    return;
  if (!mip->dirty)       //no need to write back (modified)
    return;

  /* write back */
  printf("iput: dev=%d ino=%d\n", mip->dev, mip->ino);

  block = ((mip->ino - 1) / 8) + inode_start;
  offset = (mip->ino - 1) % 8;

  /* first get the block containing this inode */
  get_block(mip->dev, block, buf);
  
  ip = (INODE *)buf + offset;
  *ip = mip->INODE;

  put_block(mip->dev, block, buf); // write block back to disk
  mip->refCount = 0; // midalloc
}

int search(MINODE *mip, char *name, int check)
{
  char sbuf[BLKSIZE], temp[256];
  struct ext2_dir_entry_2 *dp;
  char *cp;
  int i = 0, iValue = 0, rVal = 0;
  int counter = 0;
  dummy = 0;
  emptyCheck = 0;

  for (i = 0; i < 12; i++)
  { // assume DIR at most 12 direct blocks
    if (mip->INODE.i_block[i] == 0)
      return 0;

    get_block(mip->dev, mip->INODE.i_block[i], sbuf);
    dp = (DIR *)sbuf;
    cp = sbuf;
    
    
    while (cp < sbuf + BLKSIZE)
    {
      
      strncpy(temp, dp->name, dp->name_len);
      temp[dp->name_len] = 0;
      mip = iget(running->cwd->dev, dp->inode);
      if (check == 0)
      {
        if (!strcmp(temp, name))
          return dp->inode;
        emptyCheck++;
      }
      if (check == 1)
      {
      INODE *ip = &(mip->INODE);

      if (S_ISDIR(ip->i_mode))
        printf("d");
      if (S_ISREG(ip->i_mode))
        printf("-");
      if (S_ISLNK(ip->i_mode))
        printf("l");

      for (int i = 8; i >= 0; i--)
      {
        if (ip->i_mode & (1 << i))
        {
          printf("%c", t1[i]);
        }
        else
        {
          printf("%c", t2[i]);
        }
      }

      char ftime[64];

      printf(" %d ", ip->i_links_count);
      printf("%d %d ", ip->i_gid, ip->i_uid);
      strcpy(ftime, ctime(&ip->i_mtime));
      ftime[strlen(ftime) - 1] = 0;
      printf("%s ", ftime);
      printf("%d ", ip->i_size);
      printf("%s\n", temp); 
      }
      cp += dp->rec_len;
      dp = (DIR *)cp;
      dummy ++;
     if(dummy > 5)
        return 0;
    }
    return 0;
  }
  return 0;
}


int getino(char *pathname, int check)
{
  int i, ino, blk, disp;
  //INODE *ip;
  MINODE *mip;

  //printf("getino: pathname=%s\n", pathname);
  if (strcmp(pathname, "/") == 0) // if root return 2
    return 2;

  if (pathname[0] == '/') // if absolute pathname: start from root
  {
    mip = iget(dev, 2);
  }
  else
    mip = iget(running->cwd->dev, running->cwd->ino); //if relative pathname: start from cwd

  mip->refCount++;
  tokenize(pathname);

  //printf("name[i] = %s\n", name[i]);
  for (i = 0; i < nname; i++)
  {
    if (!S_ISDIR(mip->INODE.i_mode))
    {
      printf("%s is not a directory!\n", name[i]);
      iput(mip);
      return 0;
    }
    ino = search(mip, name[i], check);
    if (ino == 0)
    {
      iput(mip);
      printf("name %s does not exist\n", name[i]);
      iput(mip);
      return 0;
    }

    iput(mip); // releases used minods pointed to by mip
    mip = iget(dev, ino); // switch to new minode
    
  }
  if (check == 1) // for printing directories
  {
    search(mip, pathname, 1);
  }
  iput(mip);
  return ino;
}

