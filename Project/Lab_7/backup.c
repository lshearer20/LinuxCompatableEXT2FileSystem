
      if (check == 1)
      {
        
        int ino = 0;
        //printf("name = %s\n", dp->name);
        strncpy(temp, dp->name, dp->name_len);
        temp[dp->name_len] = 0;
        //pathname[0] = temp;
        //printf("pathname[0] = %s\n", pathname[0]);
        ino = getino(temp, 1);

        if (ino != 0 && global == 0)
        {

          MINODE *mip = iget(dev, ino);
          INODE *ip = &(mip->INODE);

          if (S_ISDIR(ip->i_mode))
            printf("d");
          if (S_ISREG(ip->i_mode))
            printf("-");
          if (S_ISLNK(ip->i_mode))
            printf("l");

          if (ip->i_mode & (1 << 1))
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
          printf("%s\n", temp);
          
        }
        //printf("counter = %d\n",counter);
      }
