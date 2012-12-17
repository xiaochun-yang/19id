#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/time.h>
#include	<termio.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<errno.h>
#include	<math.h>

static	int	motfd;
static	char	vmin0;
static	int	em_debug = 0;
static	int	em_delay1 = 0;
static	int	testdelay1 = 0;
static	int	testdelay2 = 0;

static short    em_channo;              /* the channel number */
static int      em_senseret[3];         /* status info for set/reset */
static char     em_retstring[80];
static char     em_retcopy[80];         /* a copy of the last returned string */
static char     em_sentcopy[80];        /* a copy of the last command sent */
static char     ex_sent_str[80];
static char     ex_ret_str[80];
static int      em_retlen;

static struct timeval	*readtimeout = NULL;
static struct timeval  readtimeval;

em_setnoecho()
  {
        struct termio arg;

        if(-1 == ioctl(motfd,TCGETA,&arg))
          {
            fprintf(stderr,"Setnoecho: Error on IOCTL call to get term params\n");
            exit(0);
          }
	arg.c_oflag &= ~ONLCR;
	arg.c_oflag &= ~OCRNL;
	arg.c_oflag &= ~ONOCR;
	arg.c_oflag &= ~ONLRET;
        arg.c_lflag &= ~ECHO;
        arg.c_lflag &= ~ICANON;
	arg.c_cflag &= ~CLOCAL;
        vmin0 = arg.c_cc[VMIN];
        arg.c_cc[VMIN] = 1;
        if(-1 == ioctl(motfd,TCSETA,&arg))
          {
            fprintf(stderr,"Setnoecho: Error on IOCTL call to set term params\n");
            exit(0);
          }
  }

em_printttymode()
  {
        struct termio arg;

        if(-1 == ioctl(motfd,TCGETA,&arg))
          {
            fprintf(stderr,"Setnoecho: Error on IOCTL call to get term params\n");
            exit(0);
          }
	fprintf(stderr,"tty modes:\n");
	fprintf(stderr,"c_iflag: %x (octal %o)\n",arg.c_iflag,arg.c_iflag);
	fprintf(stderr,"c_oflag: %x (octal %o)\n",arg.c_oflag,arg.c_oflag);
	fprintf(stderr,"c_lflag: %x (octal %o)\n",arg.c_lflag,arg.c_lflag);
	fprintf(stderr,"c_cflag: %x (octal %o)\n",arg.c_cflag,arg.c_cflag);
  }

em_setecho()
  {
        struct termio arg;

        if(-1 == ioctl(motfd,TCGETA,&arg))
          {
            fprintf(stderr,"Setecho: Error on IOCTL call to get term params\n");
            exit(0);
          }
	arg.c_oflag |= ONLCR;
	arg.c_oflag |= OCRNL;
	arg.c_oflag |= ONOCR;
	arg.c_oflag |= ONLRET;
        arg.c_lflag |= ECHO;
        arg.c_lflag |= ICANON;
	arg.c_cflag |= CLOCAL;
        arg.c_cc[VMIN] = vmin0;
        if(-1 == ioctl(motfd,TCSETA,&arg))
          {
            fprintf(stderr,"Setecho: Error on IOCTL call to set term params\n");
            exit(0);
          }
  }

/*
 *      Return a character from the input channel.  Return -1 if
 *      this operation timed out.
 */

int     em_readraw()
  {
	fd_set  readmask, writemask, exceptmask;
        char    rbuf;
	int	nb;

  redo_read:

        FD_ZERO(&readmask);
        FD_SET(motfd,&readmask);
        nb = select(FD_SETSIZE, &readmask, (fd_set *) 0, (fd_set *) 0, readtimeout);
        if(nb == -1)
          {
                if(errno == EINTR)
		  goto redo_read;

                fprintf(stderr,"tty_em: select error (in readraw).  Should never happen.\n");
                perror("tty_em: select in ccd_check_alive");
		em_setecho();
                exit(0);
          }
        if(nb == 0)
          {
                return(-1);         /* timed out */
          }
        if(0 == FD_ISSET(motfd, &readmask))
          {
                return(-1);         /* timed out */
          }


        if(-1 == read(motfd,&rbuf,1))
          {
            fprintf(stderr,"readraw: error reading tty input\n");
            exit(0);
          }
        return(rbuf);
  }

int     em_writeraw(c)
char    c;
  {
        char    wbuf;
        wbuf = c;

        if(-1 == write(motfd,&wbuf,1))
          {
            fprintf(stderr,"writeraw: error writing tty input\n");
            exit(0);
          }
        return(0);
  }

int	em_init(ttyname)
char	*ttyname;
  {
	char	c;
	int	i1,i2,i3,i4,i5;
	int	mind;
	char	line[132];

	if(-1 == (motfd = open(ttyname,2)))
	  {
	    fprintf(stderr,"tty_em: Canot open %s as kappa goniostat tty.\n",ttyname);
	    return(1);
	  }
	em_setnoecho();

	readtimeval.tv_sec = 0;
	readtimeval.tv_usec = 0;
	readtimeout = &readtimeval;

	return(0);
  }

int	em_close()
  {
	em_setecho();
	close(motfd);
  }

void	read_mot(arg)
int	arg;
  {
	int	i;
	char	c;
	char	cprev;

	cprev = '\0';

	while(-1 != (i = em_readraw()))
	  {
	    c = i & 0xff;
	    if(c != '\r')
	      {
		if(cprev == '\n' && c == '\n')
		  {
		    cprev = c;
		  }
		 else
		  {
		    cprev = c;
	            write(1,&c,1);
		  }
	      }
	  }
	enqueue_fcn(read_mot,arg,0.1);
  }

main(argc,argv)
int	argc;
char	*argv[];
  {
	char	line[256];
	int	i;
	FILE	*fp;

	if(argc < 2)
	{
	    fprintf(stderr,"Usage: tty_em <tty-device>\n");
	    exit(0);
	}
	if(em_init(argv[1]))
	    exit(0);

	init_clock();

	enqueue_fcn(read_mot,0,0.1);

	while(NULL != fgets(line,sizeof line, stdin))
	{
	    if(line[0] == '@')
	    {
		line[strlen(line) - 1] = '\0';

		if(NULL == (fp = fopen(&line[1],"r")))
		{
			fprintf(stderr,"tty_em: cannot open %s as file to download\n", &line[1]);
			continue;
		}
		while(NULL != fgets(line, sizeof line, fp))
		{
	    		for(i = 0; line[i] != '\n'; i++)
				em_writeraw(line[i]);
	    		em_writeraw('\r');
			usleep(100000);
		}
		fclose(fp);
		continue;
	    }

	    for(i = 0; line[i] != '\n'; i++)
		em_writeraw(line[i]);
	    em_writeraw('\r');
	}
	em_close();
	fprintf(stderr,"em_tty: Exiting on user's EOF\n");
  }
