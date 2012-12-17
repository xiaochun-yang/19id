/*
 *	read_port_raw:
 *
 *		Read data until there is none left.  Don't block.
 */

int		read_port_raw(int fd,char *stbuf,int stbufsize)
  {
	int				nread;
	fd_set			readmask;
	int				ret;
	struct timeval	timeout;

	nread = 0;

	while(1)
	  {
	    FD_ZERO(&readmask);
	    FD_SET(fd,&readmask);
	    timeout.tv_usec = 0;
	    timeout.tv_sec = 0;
	    ret = select(FD_SETSIZE, &readmask, (fd_set *) 0, (fd_set *) 0, &timeout);
	    if(ret == 0)
			return(nread);
	    if(ret == -1)
	      {
			if(errno == EINTR)
		  	  continue;	/* Ignore interrupted system calls */
			notify_server_eof(fd);
			return(-1);
	      }
	    if(0 == FD_ISSET(fd,&readmask))
	      {
			return(nread);
	      }
	    ret = recv(fd,stbuf + nread,stbufsize - nread,0);
	    if(ret == -1)
	      {
			if(errno == EINTR)
		  	continue;	/* Ignore interrupted system calls */
			notify_server_eof(fd);
			return(-1);
	      }
	    if(ret == 0)
	      {
			notify_server_eof(fd);
			return(-1);
	      }
	    nread += ret;
	  }
  }
  
int		check_port_raw(int fd)
  {
	fd_set			readmask;
	int				ret;
	struct timeval	timeout;

	FD_ZERO(&readmask);
	FD_SET(fd,&readmask);
	timeout.tv_usec = 0;
	timeout.tv_sec = 1;
	ret = select(FD_SETSIZE, &readmask, (fd_set *) 0, (fd_set *) 0, &timeout);
	if(ret == 0)
		return(0);
	if(ret == -1)
	  {
	    if(errno == EINTR)
		    return(0);		/* Ignore interrupted system calls */
		  else
		  	return(-1);
	  }
	 return(1);
  }

int     probe_port_raw(int fd)
  {
        fd_set                  readmask;
        int                             ret;
        struct timeval  timeout;
        char                    cbuf;

        FD_ZERO(&readmask);
        FD_SET(fd,&readmask);
        timeout.tv_usec = 10000;
        timeout.tv_sec = 0;
        ret = select(FD_SETSIZE, &readmask, (fd_set *) 0, (fd_set *) 0, &timeout);
        if(ret == 0)
                return(0);
        if(ret == -1)
          {
            if(errno == EINTR)
                    return(0);          /* Ignore interrupted system calls */
                  else
                        return(-1);
          }
         if(1 != recv(fd,&cbuf,1,MSG_PEEK))
                return(-1);
          else
                return(1);
  }

int     probe_port_raw_with_timeout(int fd, int nmicrosecs)
  {
        fd_set                  readmask;
        int                             ret;
        struct timeval  timeout;
        char                    cbuf;
	int			nsec;

	nsec = nmicrosecs / 1000000;
	nmicrosecs -= (nsec * 1000000);

        FD_ZERO(&readmask);
        FD_SET(fd,&readmask);
        timeout.tv_usec = nmicrosecs;
        timeout.tv_sec = nsec;
        ret = select(FD_SETSIZE, &readmask, (fd_set *) 0, (fd_set *) 0, &timeout);
        if(ret == 0)
                return(0);
        if(ret == -1)
          {
            if(errno == EINTR)
                    return(0);          /* Ignore interrupted system calls */
                  else
                        return(-1);
          }
         if(1 != recv(fd,&cbuf,1,MSG_PEEK))
                return(-1);
          else
                return(1);
  }

/*
 *	Function to do a write, with possible multiple chunks.
 *	We need this because of unknown buffering over the network.
 *
 *	The write blocks.
 *
 *	Returns the number of characters written, or -1 if an error.
 */

long	rep_write(int fd, char *buf, int count)
  {
	char huge	*pos;
	int			remcount,i;

	if(count == 0)
		return(0);

	pos = buf;
	remcount = count;

	while(remcount > 0)
	  {
		i = send(fd,pos,remcount,0);
		if(i < 0)
		  {
		    fprintf(fplog,"rep_write: Error (%d) on file descriptor %d\n",errno,fd);
		    fflush(fplog);
		    perror("rep_write");
		    return(-1);
		  }
		remcount -= i;
		pos += i;
	  }
	return(count);
  }
