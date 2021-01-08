/************* cd_ls_pwd.c file **************/

/**** globals defined in main.c file ****/
extern MINODE minode[NMINODE];
extern MINODE *root;
extern PROC proc[NPROC], *running;
extern char gpath[256];
extern char *name[64];
extern int n;
extern int fd, dev;
extern int nblocks, ninodes, bmap, imap, inode_start;
extern char line[256], cmd[32], pathname[256];
char *pathname_g;
char   *name1[64];  // assume at most 64 components in line (for link)


#define OWNER 000700
#define GROUP 000070
#define OTHER 000007

change_dir() //cd
{
  int ino = getino(pathname, 0);
  printf("ino = %d xxx\n", ino);
  if (ino != 0)
  {
    MINODE *mip = iget(dev, ino);
    if(mip->INODE.i_mode == 0x41ed)
    {
      iput(mip);
      running->cwd = mip;
      printf("running = %d\n", mip->ino);
    }
    else
      printf("this is not a dir!\n");
  }
}

get_mip(char *bname, char *dname, int filetype) 
{
  int check = 0;
  int ino = 0;
  if(!strcmp(dname, ".")) // case when we do not enter an absolute pathname
  {
    ino = running->cwd->ino;
    printf("ino = %d test\n", ino);
    MINODE *mip = iget(dev, ino);
    printf("ISDIR = %x\n", mip->INODE.i_mode);
   
    if(bname != NULL)
      check = search(mip, bname, 0);
    if(check == 0)
      kmkdir(mip, bname, filetype, 0);
    else
      printf("that directory already exists!\n");
    
    if(filetype == 0)
      mip->INODE.i_links_count++;

    mip->dirty = 1;
    iput(mip);
  }
  else
  {
    int ino = getino(dname, 0);
    printf("ino = %d\n", ino);
    if (ino != 0)
    {
      MINODE *mip = iget(dev, ino);
      printf("ISDIR = %x\n", mip->INODE.i_mode);
      check = search(mip, bname, 0);

      if (bname != NULL)
        check = search(mip, bname, 0);
      if (check == 0)
        kmkdir(mip, bname, filetype, 0);
      else
        printf("that directory already exists!\n");

      if(filetype == 0)
        mip->INODE.i_links_count++;

      mip->dirty = 1;
      iput(mip);
    }
  }

  return;
}

get_mip1(char *bname, char *dname, int filetype) 
{
  int check = 0;
  int ino = 0;
  if(!strcmp(dname, ".")) // case when we do not enter an absolute pathname
  {
    ino = running->cwd->ino;
    printf("parent ino = %d\n", ino);
    MINODE *mip = iget(dev, ino);
    printf("parent ISDIR = %x\n", mip->INODE.i_mode);
   
    if(bname != NULL)
      check = search(mip, bname, 0);
    if(check == 0)
      printf("that directory doesn't exist!\n");
    else
       krmdir(mip, bname, filetype); //CHANGETHIS
    
    if(filetype == 0)
      mip->INODE.i_links_count++;

    mip->dirty = 1;
    iput(mip);
  }
  else
  {
    int ino = getino(dname, 0);
    printf("parent ino = %d\n", ino);
    if (ino != 0)
    {
      MINODE *mip = iget(dev, ino);
      printf("parent ISDIR = %x\n", mip->INODE.i_mode);

      if (bname != NULL)
        check = search(mip, bname, 0);
      if (check == 0)
        printf("that directory doesn't exists!\n");
      else
        krmdir(mip, bname, filetype); //CHANGETHIS
        

      if(filetype == 0)
        mip->INODE.i_links_count++;

      mip->dirty = 1;
      iput(mip);
    }
  }


  return;
}

int list_file() //ls
{
  char temp[256];
  int ino = 0, blk = 0, offset = 0;
  MINODE *tempRun = running->cwd;

  if (pathname[0] != NULL)
  {
    ino = getino(pathname, 0);   //get inode to ls
    MINODE *mip = iget(dev, ino);
    iput(mip);
    running->cwd = mip;          //set running process to inode to ls
    ino = getino(" ", 1);        //print cwd of i node
    running->cwd = tempRun;      //switch running process back to original
  }
  else
  {
    ino = getino(pathname, 1); //blank pathname (print nodes in cwd)
    return;
  }
  return;
}

void pwd(MINODE *wd)
{
  if (wd == root)
    printf("/");
  else
    rpwd(wd, 0);

  printf("\n");
 
  return;
}

void rpwd(MINODE *wd, int i)
{
   int parentino, myino;
   char buf[BLKSIZE];
   char *cp;
   char name[64];
   DIR *dp;
   MINODE *pip;

   get_block(fd, wd->INODE.i_block[0], buf);
   dp = (DIR *) buf;
   cp = buf + dp->rec_len;
   dp = (DIR *) cp;
   if (wd->ino != root->ino)
   {
       int ino = dp->inode;
       pip = iget(fd, ino);
       rpwd(pip, wd->ino);
   }
   if (i != 0)
   {
       while (dp->inode != i)
       {
           cp += dp->rec_len;
           dp = (DIR *) cp;
       }
       strncpy(name,dp->name,dp->name_len);
       name[dp->name_len] = '\0';  
       printf("/");
       printf("%s",name);
   }
   return;
}

void mk_dir(int filetype)
{
  char *dname;
  char *bname;
  char *temp;
  strcpy(bname, pathname);
  strcpy(dname, pathname);
  dname = dirname(dname);
  bname = basename(bname);
  
  printf("dname =%s\nbname =%s\n", dname, bname);

  strcpy(pathname, dname);
  get_mip(bname, dname, filetype);

  return;
}

void rm_dir(int filetype)
{
  char *dname;
  char *bname;
  char *temp;
  strcpy(bname, pathname);
  strcpy(dname, pathname);
  dname = dirname(dname);
  bname = basename(bname);
  get_mip1(bname, dname, filetype);
 

  return;
}

void link(char *line)
{
  tokenize1(line);
  if(name1[1] == NULL || name1[2] == NULL)
  {
      printf("bad command\n");
      return;
  }
  char *old = name1[1];
  char *new = name1[2];

  int oino = getino(old, 0);
  printf("oino = %d\n", oino);
  MINODE *mip = iget(dev, oino);
  if (mip->INODE.i_mode == 0x41ed) // file is a file
  {
    printf("not a file\n");
    return;
  }

  int nino = getino(new, 0); // new file does not already exist
  if (!nino)
  {
    char *dname;
    char *bname;
    char *parent;
    char *child;
    char *temp;
    strcpy(child, new);
    strcpy(parent, new);
    parent = dirname(parent);
    child = basename(child);
    printf("parent = %s\nchild = %s\n", parent, child);
  
    enter_name(oino, child, parent);  // creat entry in new parents dir with same inode # of old field
  }

  return;
}

void enter_name(int oino, char *child, char *parent)
{
  int pino = getino(parent, 0);
  MINODE *pmip = iget(dev, pino);
  if(!strcmp(parent, ".")) // case when we do not enter an absolute pathname
    {
      running->cwd = pmip;
    }
  enter_child(pmip, oino, child);

  int ino = getino(child, 0);
  MINODE *mip = iget(dev, ino);
  mip->INODE.i_links_count++;
  return;
}

void unlink()
{
  int ino = getino(pathname, 0);
  printf("ino = %d\n", ino);
  MINODE *mip = iget(dev, ino);
  if (mip->INODE.i_mode != 0x81A4) // check if file is a file
  {
    printf("not a file\n");
    return;
  }

  if (!ino)
  {
    printf("file doesnt exist\n");
    return;
  }
  char *dname;
  char *bname;
  char *parent;
  char *child;
  char *temp;
  strcpy(child, pathname);
  strcpy(parent, pathname);
  parent = dirname(parent);
  child = basename(child);
  printf("parent = %s\nchild = %s\n", parent, child);

  remove_name(parent, child, ino, mip);

  return;
}

void remove_name(char *parent, char *child, int ino, MINODE *mip)
{
  int pino = getino(parent, 0);
  MINODE *pmip = iget(dev, pino);
  findname(pmip, ino, child, mip, 1);
}

void symlink(char *line)
{
  int filetype = 2;
  tokenize1(line);
  
  if (name1[1] == NULL || name1[2] == NULL)
  {
    printf("bad command\n");
    return;
  }
  
  char *old = name1[1];
  char *new = name1[2];

  int oino = getino(old, 0);
  printf("oino = %d\n", oino);
  MINODE *mip = iget(dev, oino);
  if (mip->INODE.i_mode == 0x41ed) // file is a file
  {
    filetype == 2;
    printf("not a file\n");
  }

  int nino = getino(new, 0); // new file does not already exist
  if (!nino)
  {
    char *dname;
    char *bname;
    char *parent;
    char *child;
    char *temp;
    strcpy(child, new);
    strcpy(parent, new);
    parent = dirname(parent);
    child = basename(child);
    printf("parent = %s\nchild = %s\n", parent, child);
    make_link(old, parent, new, oino, filetype);

  }

  return;
}

void make_link(char *old, char *parent, char *new, int oino, int filetype)
{
  int pino = getino(parent, 0);
  printf("pino = %d\n", pino);
  MINODE *pmip = iget(dev, pino);
  kmkdir(pmip, new, filetype, old);

}

void readlink()
{
  int ino = getino(pathname, 0);
  printf("ino = %d\n", ino);
  MINODE *mip = iget(dev, ino);
  if (mip->INODE.i_mode != 0xA1FF) // check if file is a link
  {
    printf("not a link\n");
    return;
  }
  char *temp;

  strcpy(temp, mip->INODE.i_block);
  printf("link: %s , size: %d\n", temp, mip->INODE.i_size);


  return;
}

int tokenize1(char *line)
{
  char *s;
  int i;
  int p = 0;
  s = strtok(line, " ");
  printf("IN TOK\n");
  while (s)
  {
    name1[p] = s;
    s = strtok(0, " ");
    p++;
  }
  name1[p] = "\n";
}

void krmdir(MINODE *mip, char *bname, int filetype)
{
  MINODE *pmip = mip; // get parents INO
  printf("parent ino = %d\n", pmip->ino);
  int ino = getino(pathname, 0);
  printf("child ino = %d\n", ino);
  mip = iget(dev, ino);
  printf("ref count = %d\n", mip->refCount);
  search(mip, bname, 0);

  if(emptyCheck == 2 && mip->INODE.i_mode == 0x41ed)
  {
     findname(pmip, ino, bname, mip, 0);
  }
  else
  {
      printf("NOT EMPTY\n");
  }
  
  return;
}

void findname(MINODE *pmip, int ino, char *bname, MINODE *mip, int unlink)
{
  char *cp;
  char temp[256];
  INODE *ip = &(pmip->INODE);
  printf("bname = %s\n", bname);
  entryCount = 0;
  extraDir = 0;
  for(int i=0; i<12; i++)
  {
    if(ip->i_block[i] != 0)
    {
      get_block(pmip->dev, pmip->INODE.i_block[i], buf); // read parents block information into a buf
      dp = (DIR *)buf;
      cp = buf;
      int ideal_length = 4 * ((8 + dp->name_len + 3) / 4); // a multiple of four
      int need_length = 4 * ((8 + strlen(bname) + 3) / 4); // a multiple of four
      strncpy(temp, dp->name, dp->name_len);
      temp[dp->name_len] = 0;

      while (cp + dp->rec_len < buf + BLKSIZE && strcmp(temp, bname))
      {
        cp += dp->rec_len;
        dp = (DIR *)cp;
        strncpy(temp, dp->name, dp->name_len);
        temp[dp->name_len] = 0;
        entryCount++;
        if(cp + dp->rec_len < buf + BLKSIZE && !strcmp(temp, bname)) //if there are more entrys after we find our name
        {
          while (cp + dp->rec_len < buf + BLKSIZE)
          {
            cp += dp->rec_len;
            dp = (DIR *)cp;
            extraDir++;  // number of extra dirs past it
          }
          entryCount = -1; // denotes that it is a dir in the middle
        }
          
      }
      printf("dp->name = %s\n", dp->name);
      if (!strcmp(dp->name, bname))
        printf("success\n");

      rm_child(pmip, bname, entryCount, ideal_length, cp);
      printf("parent links = %d\n", ip->i_links_count);

      pmip->INODE.i_links_count--;
      pmip->INODE.i_links_count--;
      pmip->dirty = 1;
      iput(pmip);
      if (unlink == 1) // if we are unlinking a file
      {
        mip->INODE.i_links_count--;
        if (mip->INODE.i_links_count > 0)
          mip->dirty = 1;
        else // if it is the last link
        {
          bdealloc(mip->dev, mip->INODE.i_block[0]);
          idealloc(mip->dev, mip->ino);
          iput(mip);
        }
      }
      else
      {

        bdealloc(mip->dev, mip->INODE.i_block[0]);
        idealloc(mip->dev, mip->ino);
        iput(mip);
      }
      put_block(pmip->dev, pmip->INODE.i_block[i], buf);
    }
  }

  return;
}


void rm_child(MINODE *pmip, char *bname, int entry, int ideal_length, char *cp)
{
  int currentlength = 0;
  int templength = dp->rec_len;
  printf("entry count = %d\n", entryCount); 
  if (entryCount == 0) //first and only entry in data block
  {
    printf("first entry\n");
  }
  else if (entryCount != -1) // last entry in data block
  {
    int remain = dp->rec_len;
    printf("last entry\n");
    dp->rec_len = 0;
    dp->inode = 0; // delete entry as last entry
    dp->name_len = 0;
    //strcpy(dp->name, NULL);

    cp -= ideal_length;
    dp = (DIR *)cp;
    currentlength = dp->rec_len;
    dp->rec_len = remain + currentlength; // add previos rec_len to current length
  }
  else
  {
    int tempoffset = extraDir;
    printf("middle entry\nextra dirs = %d\n", extraDir); // middle entry in data block
    int currentlength = dp->rec_len;
    dp->rec_len = currentlength + ideal_length; // add ideal length to last entry length
    //cp -= ideal_length;
    //dp = (DIR *)cp;
    while(extraDir > 0) // cycle back to the removing inode
    {
      cp -= ideal_length;
      dp = (DIR *)cp;
      extraDir--;
    }
    dp->rec_len = 0;
    dp->inode = 0; // delete entry in middle
    dp->name_len = 0;
    
    while(tempoffset > 0) // copy the extra dirs left
    {
      cp += ideal_length;
      memcpy(dp, cp, ideal_length);
      dp = (DIR *)cp;
      tempoffset--;
    }

  }

  return;
}

void kmkdir(MINODE *mip, char *bname, int filetype, char *lname)
{
  int ino = 0;
  // allocate block and inode
  ino = ialloc(dev);
  
  MINODE *pmip = mip; // parent MINODE

  // modify newly created inode
  mip = iget(dev, ino); // new child MINODE
  

  INODE *ip = &(mip->INODE);
  if (filetype == 0)
  {
    int blk = balloc(dev);
    ip->i_mode = 0x41ed; // make into a dir type inode
    ip->i_uid = running->uid;
    ip->i_size = BLKSIZE;
    ip->i_links_count = 2; // for '.' and '..'
    ip->i_atime = time(0L);
    ip->i_ctime = time(0L);
    ip->i_mtime = time(0L);
    ip->i_blocks = 2;     // data blocks assigned for dir types
    ip->i_block[0] = blk; // allocate our block to inode block [0]
    for (int i = 1; i < 15; i++)
      ip->i_block[i] = 0;

    mip->dirty = 1;
    iput(mip); // write inode to disk

    // modify data block for new dir
    char buf[BLKSIZE];
    bzero(buf, BLKSIZE);
    DIR *dp = (DIR *)buf;
    dp->inode = ino;
    dp->rec_len = 12;
    dp->name_len = 1;
    dp->name[0] = '.';
    // make .. entry: pino = parent DIR ino, blk = allocated blk
    dp = (char *)dp + 12;
    dp->inode = pmip->ino;
    dp->rec_len = BLKSIZE - 12; // rec_len spans block
    dp->name_len = 2;
    dp->name[0] = '.';
    dp->name[1] = '.';
    put_block(dev, blk, buf); // write block to disk
  }
  if(filetype == 1 || filetype == 2)
  {
    if(filetype == 1)
      ip->i_mode = 0x81A4; // make into a file type inode
    else
      ip->i_mode = 0xA1FF; // make into a link type inode
    ip->i_uid = running->uid;
    if(filetype == 2)
      ip->i_size = strlen(lname); 
    else
       ip->i_size = 0;  
    ip->i_links_count = 1; // for file
    ip->i_atime = time(0L);
    ip->i_ctime = time(0L);
    ip->i_mtime = time(0L);
    ip->i_blocks = 0;
    ip->i_block[0] = 0; // allocate our block to inode block [0]
    for (int i = 1; i < 15; i++)
      ip->i_block[i] = 0;

    mip->dirty = 1;
    if(filetype == 1)
      iput(mip); // write inode to disk
  }
  if(filetype == 2)
  {
     enter_child(pmip, ino, bname);
     mip->dirty = 1;
     strcpy(ip->i_block, lname);
     iput(mip); // write inode to disk
  }
  else
    enter_child(pmip, ino, bname);
  
  
  
  

  return;
}

void enter_child(MINODE *pmip, int ino, char *bname) // used to enter new dir_entry into parents diectory
{
  char *cp;
  INODE *ip = &(pmip->INODE);
  for(int i=0; i<12; i++)
  {
    if(ip->i_block[i] == 0) // No space exists in data block(s)
      {
        balloc(dev);
        pmip->INODE.i_size += BLKSIZE;  // incriment parents size by blocksize;
        get_block(pmip->dev, pmip->INODE.i_block[i], buf); // read parents block information into a buf
        dp = (DIR *)buf;
        dp->inode = ino; // enter new entry as last entry
        dp->rec_len = BLKSIZE; 
        dp->name_len = strlen(bname);
        strcpy(dp->name, bname);
        put_block(pmip->dev, pmip->INODE.i_block[i], buf);
        printf("HELLO?\n");
        return;
      }
    if(ip->i_block[i] != 0)
      {
        get_block(pmip->dev, pmip->INODE.i_block[i], buf); // read parents block information into a buf
        dp = (DIR *)buf;
        cp = buf;
        int ideal_length = 4 * ((8 + dp->name_len + 3) / 4); // a multiple of four
        int need_length = 4 * ((8 + strlen(bname) + 3) / 4); // a multiple of four
        while (cp + dp->rec_len < buf + BLKSIZE)
        {
          cp += dp->rec_len;
          dp = (DIR *)cp;
        }
        int remain = dp->rec_len - ideal_length;
        if (remain >= need_length) // we have space to enter our new entry into this block
        {
          printf("upper dp->rec_len = %d\n", dp->rec_len);
          dp->rec_len = ideal_length; // trim previos rec_len to ideal length
          printf("upper dp->rec_len = %d\n", dp->rec_len);
          cp += dp->rec_len;
          dp = (DIR *)cp;
          dp->inode = ino;            // enter new entry as last entry
          dp->rec_len = remain; // trim previos rec_len to ideal length
          printf("new dp->rec_len = %d\n", dp->rec_len);
          dp->name_len = strlen(bname);
          strcpy(dp->name, bname);
          put_block(pmip->dev, pmip->INODE.i_block[i], buf);
          return;
        }
      }
  }
  

  return;
}

int ialloc(int dev)
{
  int i;
  // read inode_bitmap block
  get_block(dev, imap, buf);

  for (i = 0; i < ninodes; i++)
  {
    if (tst_bit(buf, i) == 0)
    {
      set_bit(buf, i);
      put_block(dev, imap, buf);
      decFreeInodes(dev);
      return i + 1;
    }
  }
  return 0;
}

int balloc(int dev)
{
  int i;
  // read block_bitmap block
  get_block(dev, bmap, buf);

  for (i = 0; i < nblocks; i++)
  {
    if (tst_bit(buf, i) == 0)
    {
      set_bit(buf, i);
      put_block(dev, bmap, buf);
      decFreeBlocks(dev);
      return i + 1;
    }
  }
  return 0;
}

void idealloc(int dev, int ino)
{
  char buf[BLKSIZE];
  int i = 0;
  MTABLE *mp = &mtable[0]; // (MTABLE *)get_mtable(dev)
  if(ino > mp->ninodes)
  {
    printf("inumber %d out of range\n", ino);
    return;
  }
  // get inode bitmap block
  get_block(dev, mp->imap, buf);
  clr_bit(buf, ino-1);
  // write buf back
  put_block(dev, mp->imap, buf);
  // update free inode count in SUPER and GD
  incFreeInodes(dev);
  return;
}

void bdealloc(int dev, int blockNum)
{
  char buf[BLKSIZE];
  int i = 0;
  MTABLE *mp = &mtable[0]; // (MTABLE *)get_mtable(dev)
  // get inode bitmap block
  get_block(dev, mp->bmap, buf);
  clr_bit(buf, blockNum);
  // write buf back
  put_block(dev, mp->bmap, buf);
  // update free inode count in SUPER and GD
  incFreeBlocks(dev);
}

int decFreeInodes(int dev)
{
  // bzero(buf, BLKSIZE);
  // dec free inodes count in SUPER and GD
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_inodes_count--;
  put_block(dev, 1, buf);
  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_inodes_count--;
  put_block(dev, 2, buf);
}

int incFreeInodes(int dev)
{
  char buf[BLKSIZE];
  // int free inodes count in SUPER and GD
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_inodes_count++;
  put_block(dev, 1, buf);
  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_inodes_count++;
  put_block(dev, 2, buf);
}

int decFreeBlocks(int dev)
{
  // dec free inodes count in SUPER and GD
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_blocks_count--;
  put_block(dev, 1, buf);
  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_blocks_count--;
  put_block(dev, 2, buf);
}

int incFreeBlocks(int dev)
{
  char buf[BLKSIZE];
  // int free inodes count in SUPER and GD
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_blocks_count++;
  put_block(dev, 1, buf);
  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_blocks_count++;
  put_block(dev, 2, buf);
}

int tst_bit(char *buf, int bit)
{
  int i, j;
  i = bit / 8;
  j = bit % 8;
  if (buf[i] & (1 << j))
    return 1;
  return 0;
}

int set_bit(char *buf, int bit)
{
  int i, j;
  i = bit / 8;
  j = bit % 8;
  buf[i] |= (1 << j);
}

int clr_bit(char *buf, int bit)
{
  int i, j;
  i = bit / 8;
  j = bit % 8;
  buf[i] &= ~(1 << j);
}





