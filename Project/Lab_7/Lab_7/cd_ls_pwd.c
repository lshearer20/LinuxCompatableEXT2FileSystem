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

#define OWNER 000700
#define GROUP 000070
#define OTHER 000007

change_dir() //cd
{
  int ino = getino(pathname, 0);
  printf("ino = %d\n)", ino);
  if (ino != 0)
  {
    MINODE *mip = iget(dev, ino);
    iput(mip);
    running->cwd = mip;
  }
}

int list_file() //ls
{
  char temp[256];
  int ino = 0;

  if (pathname[0] != NULL)
  {
    ino = getino(pathname, 0);
    printf("ino = %d\n", ino);

    if (ino != 0)
    {

      MINODE *mip = iget(dev, ino);
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
      //strncpy(temp, dp->name, dp->name_len);
      //temp[dp->name_len] = 0;
      printf("%s\n", name[0]);

      
    }
    return;
  }
  else
  {
    //printf("PRINT ALL\n");

    ino = getino(pathname, 1);
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


