/*********** util.c file ****************/

/**** globals defined in main.c file ****/
extern MINODE minode[NMINODE];
extern MINODE *root;
extern PROC proc[NPROC], *running;
extern char gpath[256];
extern char *name[64];
extern int n, ct = 0;
extern int fd, dev;
extern int nblocks, ninodes, bmap, imap, inode_start;
extern char line[256], cmd[32], pathname[256];

int get_block(int dev, int blk, char *buf)
{
  lseek(dev, (long)blk * BLKSIZE, 0);
  read(dev, buf, BLKSIZE);
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
  n++;
  while (s)
  {
    name[p] = s;
    s = strtok(0, "/");
    p++;
    n++;
  }
  name[p] = "\n";
}

// return minode pointer to loaded INODE
MINODE *iget(int dev, int ino)
{
  int i;
  MINODE *mip;
  char buf[BLKSIZE];
  int blk, disp;
  INODE *ip;
  for (i = 0; i < NMINODE; i++)
  {
    mip = &minode[i];
    if (mip->dev == dev && mip->ino == ino)
    {
      mip->refCount++;
      return mip;
    }
  }

  for (i = 0; i < NMINODE; i++)
  {
    mip = &minode[i];
    if (mip->refCount == 0)
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

  if (mip->refCount > 0)
    return;
  if (!mip->dirty)
    return;

  /* write back */
  printf("iput: dev=%d ino=%d\n", mip->dev, mip->ino);

  block = ((mip->ino - 1) / 8) + inode_start;
  offset = (mip->ino - 1) % 8;

  /* first get the block containing this inode */
  get_block(mip->dev, block, buf);

  ip = (INODE *)buf + offset;
  *ip = mip->INODE;

  put_block(mip->dev, block, buf);
}

int search(MINODE *mip, char *name, int check)
{
  char sbuf[BLKSIZE], temp[256];
  struct ext2_dir_entry_2 *dp;
  char *cp;
  int i = 0, iValue = 0, rVal = 0;
  int counter = 0;
  for (i = 0; i < 12; i++)
  { // assume DIR at most 12 direct blocks
    if (mip->INODE.i_block[i] == 0)
      break;

    get_block(dev, mip->INODE.i_block[i], sbuf);
    dp = (DIR *)sbuf;
    cp = sbuf;
    while (cp < sbuf + BLKSIZE)
    {

      strncpy(temp, dp->name, dp->name_len);
      temp[dp->name_len] = 0;
      int i = strcmp(temp, name);

      if (!strcmp(temp, name))
      {
        iValue = dp->inode;
        return dp->inode;
      }
      else
        iValue = 0;

      if (iValue > 0)
      {
        printf("SECCUESS\n");
        rVal = iValue;
      }

      if (check == 1)
      {

        int ino = 0;
        strncpy(temp, dp->name, dp->name_len);
        temp[dp->name_len] = 0;

        INODE *ip = &(mip->INODE);

        if (S_ISDIR(ip->i_mode))
          printf("d");
        if (S_ISREG(ip->i_mode))
          printf("-");
        if (S_ISLNK(ip->i_mode))
          printf("l");

        if (ip->i_mode & (1 << 8))
          printf("r");
        else
          printf("-");

        if (ip->i_mode & (1 << 7))
          printf("w");
        else
          printf("-");

        if (ip->i_mode & (1 << 6))
          printf("x");
        else
          printf("-");

        if (ip->i_mode & (1 << 5))
          printf("r");
        else
          printf("-");

        if (ip->i_mode & (1 << 4))
          printf("w");
        else
          printf("-");

        if (ip->i_mode & (1 << 3))
          printf("x");
        else
          printf("-");

        if (ip->i_mode & (1 << 2))
          printf("r");
        else
          printf("-");

        if (ip->i_mode & (1 << 1))
          printf("w");
        else
          printf("-");

        if (ip->i_mode & (1 << 0))
          printf("x ");
        else
          printf("- ");

        char ftime[64];

        printf("%d ", ip->i_links_count);
        printf("%d %d ", ip->i_gid, ip->i_uid);
        printf("%d ", ip->i_size);
        strcpy(ftime, ctime(&ip->i_mtime));
        ftime[strlen(ftime) - 1] = 0;
        printf("%s ", ftime);
        printf("%s\n", temp);
      }

      strncpy(temp, dp->name, dp->name_len);
      temp[dp->name_len] = 0;
      cp += dp->rec_len;
      dp = (DIR *)cp;
    }
    ct++;
  }
  if (rVal > 0)
    return rVal;
  else
    return 0;
}

int getino(char *pathname, int check)
{
  int i, ino, blk, disp;
  INODE *ip;
  MINODE *mip;

  //printf("getino: pathname=%s\n", pathname);
  if (strcmp(pathname, "/") == 0)
    return 2;

  if (pathname[0] == '/')
  {
    mip = iget(dev, 2);
    printf("MADEIT\n");
  }
    
  else
    mip = iget(running->cwd->dev, running->cwd->ino);

  tokenize(pathname);
  //printf("check = %d\n", check);
  //if (check == 0)
  {
    for (i = 0; i < n; i++)
    {
      printf("===========================================\n");
      ino = search(mip, name[i], check);

      if (ino == 0)
      {
        iput(mip);
        //printf("name %s does not exist\n", name[i]);
        return 0;
      }
      else
      {
        if(check == 1)
          ls(ino, check);
        return ino;
      }

      iput(mip);
      mip = iget(dev, ino);
    }
  }

  return ino;
}
ls(int ino, int check)
{
  MINODE *mip = iget(dev, ino);
  search(mip, pathname, check);
}
