#include	"ccd_dc_ext.h"

/*
 *	Module to handle the network connections for
 *	the data collection server.
 *
 *	First, ccd_server_init() is called to initialize
 *	the network handling code.
 *
 *	Second, ccd_server_update() is called at heartbeat
 *	to determine if a connection is requested.  If so,
 *	a notation is made for this socket and an accept is done.
 *
 *	All active sockets are tested for read select.
 *	A socket for which a connection has been accepted
 *	but for which no purpose has been established must
 *	first present:
 *
 *	connect <purpose>
 *
 *	where <purpose> is:
 *
 *		command
 *		output
 *		status
 *		xform
 *		det_cmd
 *		det_status
 *		bl_cmd
 *		bl_status
 *
 *	Once the purpose of the connection has been established,
 *	the usual file descriptors are prepared for this socket
 *	so that the rest of the code works properly.
 *
 *	Notations of socket connections which close or otherwise
 *	die are also made.
 */


/*
 *      Entries for network names, ports, etc.
 */

extern struct serverlist        dcserver;
extern struct serverlist        daserver;
extern struct serverlist        xfserver;
extern struct serverlist        stserver;
extern struct serverlist	dtserver;
extern struct serverlist	dtdserver;
extern struct serverlist	blserver;
extern struct serverlist        conserver;
extern struct serverlist        viewserver;
extern int                      ccd_communication;

#define	MAX_FDS		10

#define	COMMAND_ID	0
#define	OUTPUT_ID	1
#define	STATUS_ID	2
#define	XFORM_ID	3
#define	DET_CMD_ID	4
#define	DET_STATUS_ID	5
#define	BL_CMD_ID	6
#define	BL_STATUS_ID	7

char	*connect_types[] = {
				"command",
				"output",
				"status",
				"bl_cmd",
				"bl_status",
				NULL
			    };
int	connect_ids[] = {
			  COMMAND_ID,
			  OUTPUT_ID,
			  STATUS_ID,
			  BL_CMD_ID,
			  BL_STATUS_ID,
			  -1,
			};

int	valid_fds[MAX_FDS];

int	server_s = -1;

int	connected_fds[MAX_FDS];

void	catch_sigpipe()
  {
	fprintf(stderr,"ccd_dc_server: SIG :          caught SIGPIPE signal\n");
  }

void	catch_sighup()
  {
	fprintf(stdout,"ccd_dc_server: SIG :          caught SIGHUP signal (ccd_daemon issued for KILL).\n");
	fprintf(stdout,"ccd_dc_server: ACT :          Sending exit to beamline process (if running).\n");
	if(fdblcmd == -1)
	  fprintf(stdout,"ccd_dc_server: WARN:          beamline process NOT running.\n");
	 else
	  {
	  	output_blcmd(fdblcmd,"cmd","exit");
	  }
	fprintf(stdout,"ccd_dc_server: CONT:          Continue the exit sequence.\n");
	cleanexit(GOOD_STATUS);
  }

char	*timestamp(fp)
FILE	*fp;
  {
	long	clock;

	time(&clock);
	fprintf(fp,"timestamp    : INFO:          %s",(char *) ctime(&clock));
	fflush(fp);
  }

/*
 *	This is to facilitate the discovery of disconnections.
 *
 *	For some purposes, like status and command, the disconnect
 *	of the remote process is discovered in a timely fashon.
 *
 *	This routine checks a connection like the transform command.
 *	In general, it should be a write only socket and written only
 *	occasionally.
 */

ccd_check_alive(key)
int	key;
  {
	fd_set	readmask, writemask, exceptmask;
	struct	timeval	timeout;
	int	nb;
	char	buf[512];

	if(valid_fds[key] == -1)
		return;

	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	FD_ZERO(&readmask);
	FD_SET(valid_fds[key],&readmask);
	nb = select(FD_SETSIZE, &readmask, (fd_set *) 0, (fd_set *) 0, &timeout);
	if(nb == -1)
	  {
		if(errno == EINTR)
		  {
		    enqueue_fcn(ccd_check_alive,key,5.0);
		    return;		/* timed out */
		  }
		fprintf(stderr,
			"ccd_dc_server: ERR!:          select error (ccd_check_alive on %d).  Should never happen.\n",key);
		timestamp(fplog);
		fprintf(fplog,"ccd_dc_server: select error (ccd_check_alive on %d).  Should never happen.\n",key);
		fflush(fplog);
		perror("ccd_dc_server: select in ccd_check_alive");
		cleanexit(BAD_STATUS);
	  }
	if(nb == 0)
	  {
		enqueue_fcn(ccd_check_alive,key,5.0);
		return;		/* timed out */
	  }
	if(0 == FD_ISSET(valid_fds[key],&readmask))
	  {
		enqueue_fcn(ccd_check_alive,key,5.0);
		return;		/* timed out */
	  }

	nb = recv(valid_fds[key],buf,512,MSG_PEEK);
	if(nb <= 0)
	  {
		notify_server_eof(valid_fds[key]);
		return;
	  }
	enqueue_fcn(ccd_check_alive,key,5.0);
  }
/*
 *	This is to facilitate the discovery of disconnections.
 *
 *	Used with other servers.
 *
 *	For some purposes, like status and command, the disconnect
 *	of the remote process is discovered in a timely fashon.
 *
 *	This routine checks a connection like the transform command.
 *	In general, it should be a write only socket and written only
 *	occasionally.
 */

ccd_check_alive_server(int key_p)
  {
	fd_set	readmask, writemask, exceptmask;
	struct	timeval	timeout;
	int	nb;
	char	buf[512];

	if(key_p == -1)
		return;

	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	FD_ZERO(&readmask);
	FD_SET(key_p,&readmask);
	nb = select(FD_SETSIZE, &readmask, (fd_set *) 0, (fd_set *) 0, &timeout);
	if(nb == -1)
	  {
		if(errno == EINTR)
		  {
		    enqueue_fcn(ccd_check_alive_server,key_p,5.0);
		    return;		/* timed out */
		  }
		fprintf(stderr,
			"ccd_dc_server: ERR!:          select error (ccd_check_alive on %d).  Should never happen.\n",key_p);
		timestamp(fplog);
		fprintf(fplog,"ccd_dc_server: select error (ccd_check_alive_server on %d).  Should never happen.\n",key_p);
		fflush(fplog);
		perror("ccd_dc_server: select in ccd_check_alive_server");
		cleanexit(BAD_STATUS);
	  }
	if(nb == 0)
	  {
		enqueue_fcn(ccd_check_alive_server,key_p,5.0);
		return;		/* timed out */
	  }
	if(0 == FD_ISSET(key_p,&readmask))
	  {
		enqueue_fcn(ccd_check_alive_server,key_p,5.0);
		return;		/* timed out */
	  }

	nb = recv(key_p,buf,512,MSG_PEEK);
	if(nb <= 0)
	  {
		notify_server_eof(key_p);
		return;
	  }
	enqueue_fcn(ccd_check_alive_server,key_p,5.0);
  }

ccd_server_init()
  {
	struct	sockaddr_in	from;
	int	i;
	int	optname,optval,optlen;

	signal(SIGPIPE,catch_sigpipe);
	signal(SIGHUP,catch_sighup);

	for(i = 0; i < MAX_FDS; i++)
		connected_fds[i] = -1;
	for(i = 0; i < MAX_FDS; i++)
		valid_fds[i] = -1;

	if(-1 == (server_s = socket(AF_INET, SOCK_STREAM, 0)))
	  {
	  	timestamp(fplog);
		fprintf(stdout,"ccd_dc_server: ERR!:          cannot create socket.\n");
		fprintf(stdout,"\n\nNote well:  This is almost always an indication that\n");
		fprintf(stdout,"there is another copy of ccd_dc_api running.\n\n\n");
		fprintf(fplog,"ccd_dc_server: cannot create socket\n");
		fflush(fplog);
		cleanexit(BAD_STATUS);
	  }
	/*
	 *	Set the KEEPALIVE and RESUSEADDR socket options.
	 */
	
	optname = SO_KEEPALIVE;
	optval = 1;
	optlen = sizeof (int);

	if(-1 == setsockopt(server_s,SOL_SOCKET,optname,&optval,optlen))
	  {
	    timestamp(fplog);
	    fprintf(stderr,"ccd_dc: cannot set SO_KEEPALIVE socket option.\n");
	    fprintf(fplog,"ccd_dc: cannot set SO_KEEPALIVE socket option.\n");
	    fflush(fplog);
	    perror("ccd_dc: setting SO_KEEPALIVE");
	    cleanexit(BAD_STATUS);
	  }

	optname = SO_REUSEADDR;
	optval = 1;
	optlen = sizeof (int);

	if(-1 == setsockopt(server_s,SOL_SOCKET,optname,&optval,optlen))
	  {
	    timestamp(fplog);
	    fprintf(stderr,"ccd_dc: cannot set SO_REUSEADDR socket option.\n");
	    fprintf(fplog,"ccd_dc: cannot set SO_REUSEADDR socket option.\n");
	    fflush(fplog);
	    perror("ccd_dc: setting SO_REUSEADDR");
	    cleanexit(BAD_STATUS);
	  }

	from.sin_family = AF_INET;
	from.sin_addr.s_addr = htonl(INADDR_ANY);
	from.sin_port = htons(dcserver.sl_port);

	if(bind(server_s, (struct sockaddr *) &from,sizeof from))
	  {
		fprintf(stdout,"ccd_dc_server: ERR!:          cannot BIND socket.\n");
		fprintf(stdout,"\n\nNote well:  This is almost always an indication that\n");
		fprintf(stdout,"there is another copy of ccd_dc_api running.\n\n\n");
		timestamp(fplog);
		fprintf(fplog,"ccd_dc: cannot bind socket\n");
		fflush(fplog);
		cleanexit(BAD_STATUS);
	  }
	
	listen(server_s,5);
  }

check_other_servers()
  {
	if(fddetcmd == -1)
	  {
	    if(-1 != connect_to_host_api(&fddetcmd,dtserver.sl_hrname,dtserver.sl_port,NULL))
	      {
		fprintf(stdout,
		"ccd_dc_server: CONN:          connection established with hostname: %s with fd: %d for detector control\n",
			dtserver.sl_hrname,fddetcmd);
		timestamp(fplog);
		fprintf(fplog,"ccd_dc: connection established with hostname: %s with fd: %d for detector control\n",
			dtserver.sl_hrname,fddetcmd);
		fflush(fplog);
		enqueue_fcn(ccd_check_alive_server,fddetcmd,1.0);
	      }
	  }
	if(fdxfcm == -1)
	  {
	    if(-1 != connect_to_host_api(&fdxfcm,xfserver.sl_hrname,xfserver.sl_port,NULL))
	      {
		fprintf(stdout,
		"ccd_dc_server: CONN:          connection established with hostname: %s with fd: %d for transform control\n",
			xfserver.sl_hrname,fdxfcm);
		timestamp(fplog);
		fprintf(fplog,"ccd_dc: connection established with hostname: %s with fd: %d for transform control\n",
			xfserver.sl_hrname,fdxfcm);
		fflush(fplog);
		enqueue_fcn(ccd_check_alive_server,fdxfcm,1.0);
	      }
	  }
	if(bl_is_server)
	{
	if(fdblcmd == -1)
	  {
	    if(-1 != connect_to_host_api(&fdblcmd,blserver.sl_hrname,blserver.sl_port,NULL))
	      {
		fprintf(stdout,
		"ccd_dc_server: CONN:          connection established with hostname: %s with fd: %d for goniostat control\n",
			blserver.sl_hrname,fdblcmd);
		timestamp(fplog);
		fprintf(fplog,
		"ccd_dc_server: CONN:          connection established with hostname: %s with fd: %d for goniostat control\n",
			blserver.sl_hrname,fdblcmd);
		fflush(fplog);
		enqueue_fcn(ccd_check_alive_server,fdblcmd,1.0);
	      }
	  }
	if(fdblstatus == -1)
	  {
	    if(-1 != connect_to_host_api(&fdblstatus, blserver.sl_hrname, atoi(getenv("CCD_BLSTATPORT")), NULL))
	      {
		fprintf(stdout,
		"ccd_dc_server: CONN:          connection established with hostname: %s with fd: %d for goniostat status\n",
			blserver.sl_hrname,fdblcmd);
		timestamp(fplog);
		fprintf(fplog,
		"ccd_dc_server: CONN:          connection established with hostname: %s with fd: %d for goniostat status\n",
			blserver.sl_hrname,fdblcmd);
		fflush(fplog);
		enqueue_fcn(ccd_check_alive_server,fdblstatus,1.0);
	      }
	  }
	}
  }

notify_server_eof(fd)
int	fd;
{
    char message[80];

    int	i;

	for(i = 0; i < MAX_FDS; i++)
	  if(valid_fds[i] == fd)
		break;
	if(i == MAX_FDS)
	{
	    if(fd == fdxfcm)
	    {
		close(fdxfcm);
		fdxfcm = -1;
		fprintf(stdout,"ccd_dc_server: DROP:          connection for transform command closed\n");
		timestamp(fplog);
		fprintf(fplog,"ccd_dc: connection for transform command closed\n");
		fflush(fplog);
                dc_stop = 1;
		sprintf(message," MAX_FDS in notify_server_eof");
		issue_signal("CCD_SIG_XFORM_DROPPED",message);
                set_alert_msg(
                 "ERROR: Connection to transform program or CCD lost.");
		return;
	     }
	     else if(fd == fddetcmd)
	     {
		close(fddetcmd);
		fddetcmd = -1;
		fprintf(stdout,"ccd_dc_server: DROP:          connection for ccd_det command closed\n");
		timestamp(fplog);
		fprintf(fplog,"ccd_dc: connection for ccd_det command closed\n");
		fflush(fplog);
                dc_stop = 1;
		message[0] = '\0';
		issue_signal("CCD_SIG_DET_DROPPED",message);
                set_alert_msg("ERROR: Lost contact with CCD controller.");
		return;
	     }
	     else if(bl_is_server && fd == fdblcmd)
	     {
		close(fdblcmd);
		fdblcmd = -1;
		fprintf(stdout,"ccd_dc_server: DROP:          connection for goniostat command closed\n");
		timestamp(fplog);
		fprintf(fplog,"ccd_dc: connection for goniostat command closed\n");
		fflush(fplog);
                dc_stop = 1;
		message[0] = '\0';
		issue_signal("CCD_SIG_DET_DROPPED",message);
                set_alert_msg("ERROR: Lost contact with goniostat controller.");
		return;
	     }
	     else if(bl_is_server && fd == fdblstatus)
	     {
		close(fdblstatus);
		fdblstatus = -1;
		fprintf(stdout,"ccd_dc_server: DROP:          connection for goniostat status closed\n");
		timestamp(fplog);
		fprintf(fplog,"ccd_dc: connection for goniostat status closed\n");
		fflush(fplog);
                dc_stop = 1;
		message[0] = '\0';
		issue_signal("CCD_SIG_DET_DROPPED",message);
		return;
	     }
	     else
	     {
	        fprintf(stderr,"ccd_dc_server: WARN:          weird descriptor (%d) given to notify_server_eof\n",fd);
	        fprintf(stderr,"ccd_dc_server: INFO:          This usually occurs when a client disconnects\n");
	        fprintf(stderr,"ccd_dc_server: INFO:          or terminates abnormally.\n");
	        timestamp(fplog);
	        fprintf(fplog,"ccd_dc: weird descriptor (%d) given to notify_server_eof\n",fd);
	        fflush(fplog);
	        return;
	     }
	}
	switch(i)
	{
	    case COMMAND_ID:
		close(fdcom);
		fdcom = -1;
		fprintf(stdout,"ccd_dc_server: DROP:          connection for command closed\n");
		timestamp(fplog);
		fprintf(fplog,"ccd_dc: connection for command closed\n");
		fflush(fplog);
		break;
	    case OUTPUT_ID:
		close(fdout);
		fclose(fpout);
		fdout = -1;
		fpout = NULL;
		fprintf(stdout,"ccd_dc_server: DROP:          connection for output closed\n");
		timestamp(fplog);
		fprintf(fplog,"ccd_dc: connection for output closed\n");
		fflush(fplog);
		break;
	    case STATUS_ID:
		close(fdstat);
		fdstat = -1;
		fprintf(stdout,"ccd_dc_server: DROP:          connection for status closed\n");
		timestamp(fplog);
		fprintf(fplog,"ccd_dc: connection for status closed\n");
		fflush(fplog);
		break;
	    case XFORM_ID:
		close(fdxfcm);
		fdxfcm = -1;
		fprintf(stdout,"ccd_dc_server: DROP:          connection for transform command closed\n");
		timestamp(fplog);
		fprintf(fplog,"ccd_dc: connection for transform command closed\n");
		fflush(fplog);
                dc_stop = 1;
		sprintf(message," XFORM_ID");
		issue_signal("CCD_SIG_XFORM_DROPPED",message);
                set_alert_msg(
                 "ERROR: Connection to transform program or CCD lost.");
/* printf ("drop transform 2\n"); */
		break;
	    case DET_CMD_ID:
		close(fddetcmd);
		fddetcmd = -1;
		fprintf(stdout,"ccd_dc_server: DROP:          connection for detector command closed\n");
		timestamp(fplog);
		fprintf(fplog,"ccd_dc: connection for ccd_det command closed\n");
		fflush(fplog);
                dc_stop = 1;
		sprintf(message," DET_CMD_ID");
		issue_signal("CCD_SIG_DET_DROPPED",message);
                set_alert_msg("ERROR: Lost contact with CCD controller.");
/* printf ("drop CCD 2\n"); */
		break;
	    case DET_STATUS_ID:
		close(fddetstatus);
		fddetstatus = -1;
		fprintf(stdout,"ccd_dc_server: DROP:          connection for ccd_det status closed\n");
		timestamp(fplog);
		fprintf(fplog,"ccd_dc: connection for ccd_det status closed\n");
		fflush(fplog);
		break;
	    case BL_CMD_ID:
		close(fdblcmd);
		fdblcmd = -1;
		dc_stop = 1;
		fprintf(stdout,"ccd_dc_server: DROP:          connection for ccd_bl command closed\n");
		timestamp(fplog);
		fprintf(fplog,"ccd_dc: connection for ccd_bl command closed\n");
		fflush(fplog);
		break;
	    case BL_STATUS_ID:
		close(fdblstatus);
		fdblstatus = -1;
		fprintf(stdout,"ccd_dc_server: DROP:          connection for ccd_bl status closed\n");
		timestamp(fplog);
		fprintf(fplog,"ccd_dc: connection for ccd_bl status closed\n");
		fflush(fplog);
                dc_stop = 1;
                set_alert_msg("Beam Line Control connection DROPPED.");
		sprintf(message," BL_STATUS_ID");
		issue_signal("CCD_SIG_DET_DROPPED",message);
		break;
	    default:
		break;
	}
	valid_fds[i] = -1;
	for(i = 0; i < MAX_FDS; i++)
	  if(connected_fds[i] == fd)
	    {
		connected_fds[i] = -1;
		break;
	    }
}

ccd_server_update(arg)
int	arg;
  {
	struct	sockaddr_in	from;
	int 	g;
	int	len;
	char	buf[512],string1[512],string2[512];
	int	nb;
	int	i,j,k;
	fd_set	readmask, writemask, exceptmask;
	struct	timeval	timeout;
	int	got_valid;
	int	bufindex,ii,jj;
	int	connection_terminated;

	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	got_valid = -1;

	/*
	 *	Select for read the server socket + any
	 *	file descriptors which have been accepted
	 *	for connection but NOT validated.
	 */

	FD_ZERO(&readmask);
	FD_SET(server_s,&readmask);
	for(i = 0; i < MAX_FDS; i++)
	  if(connected_fds[i] != -1)
	    {
	      for(j = 0, k = -1; j < MAX_FDS; j++)
		if(connected_fds[i] == valid_fds[j])
		  {
		    k = j;
		    break;
		  }
	      if(k == -1)	/* connected but not validated */
		FD_SET(connected_fds[i],&readmask);
	    }
	nb = select(FD_SETSIZE, &readmask, (fd_set *) 0, (fd_set *) 0, &timeout);
	if(nb == -1)
	  {
		fprintf(stderr,"ccd_dc: select error.  Should never happen.\n");
		timestamp(fplog);
		fprintf(fplog,"ccd_dc: select error.  Should never happen.\n");
		fflush(fplog);
		perror("ccd_dc: select");
		cleanexit(BAD_STATUS);
	  }
	if(nb == 0)
	  {
		check_other_servers();
		enqueue_fcn(ccd_server_update,0,1.0);
		return;		/* timed out */
	  }
	    
	/*
	 *	There is something to do.  If the listener socket is ready for read,
	 *	perform an accept on it.  If one of the others is ready to read, get
	 *	the data and output it to the screen.
	 */
	if(FD_ISSET(server_s,&readmask))
	  {
	    len = sizeof from;
	    g = accept(server_s, (struct sockaddr *) &from, &len);

	    if(g < 0)
	      {
	        if(errno != EINTR)
	          {
		    fprintf(stderr,"ccd_dc: accept error for network connection\n");
		    timestamp(fplog);
		    fprintf(fplog,"ccd_dc: accept error for network connection\n");
		    fflush(fplog);
		    perror("accept");
		    cleanexit(BAD_STATUS);
	          }
	      }
	     else
	      {
		fprintf(stdout,"ccd_dc_server: CONN:          connection accepted from: %s with fd: %d\n",
				inet_ntoa(from.sin_addr), g);
		timestamp(fplog);
		fprintf(fplog,"ccd_dc: connection accepted from: %s with fd:\n",inet_ntoa(from.sin_addr), g);
		fflush(fplog);
		for(i = 0, j = -1; i < MAX_FDS; i++)
		  if(connected_fds[i] == -1)
		    {
			connected_fds[i] = g;
			j = 0;
			break;
		    }
		if(j == -1)
		  {
		    fprintf(stdout,"ccd_dc_server: ERR!:          all %d connection slots used up.\n",MAX_FDS);
		    timestamp(fplog);
		    fprintf(fplog,"ccd_dc: all %d connection slots used up.\n",MAX_FDS);
		    fflush(fplog);
		    cleanexit(BAD_STATUS);
		  }
	      }
	  }
	for(i = 0; i < MAX_FDS; i++)
	  if(connected_fds[i] != -1)
	    {
	      if(FD_ISSET(connected_fds[i],&readmask))
	        {
		  connection_terminated = 0;
		  for(bufindex = 0; ;)
		    {
		      nb = read(connected_fds[i],&buf[bufindex],sizeof buf);
		      if(nb <= 0)
		        {
			  if(errno == EINTR)
				continue;
		          fprintf(stdout,"ccd_dc: unvalidated connection terminated by client on fd: %d\n",connected_fds[i]);
			  timestamp(fplog);
		          fprintf(fplog,"ccd_dc: unvalidated connection terminated by client on fd: %d\n",connected_fds[i]);
			  fflush(fplog);
			  if(nb < 0)
			    {
			      fprintf(stdout,"ccd_dc: error returned on unvalidated connection termination:\n");
			      perror("ccd_dc: unvalidated connection termination:");
			    }
			   else
			      fprintf(stdout,"ccd_dc: EOF on unvalidated connection termination.\n");
		          close(connected_fds[i]);
		          connected_fds[i] = -1;
			  connection_terminated = 1;
		          break;
		        }
		      bufindex += nb;
		      for(ii = 0, jj = 0; ii < bufindex; ii++)
			if(buf[ii] == '\0')
				jj = 1;
		      if(jj == 1)
			break;
		    }
		  if(connection_terminated == 1)
			continue;
		  string1[0] = '\0'; string2[0] = '\0';
		  sscanf(buf,"%s%s",string1,string2);
		  if(0 != strcmp(string1,"connect"))
		    {
		      fprintf(stderr,"stdout: incorrect connection protocal on fd %d: %s\n",connected_fds[i],buf);
		      timestamp(fplog);
		      fprintf(fplog,"stdout: incorrect connection protocal on fd %d: %s\n",connected_fds[i],buf);
		      fflush(fplog);
		      close(connected_fds[i]);
		      connected_fds[i] = -1;
		      continue;
		    }
		  for(j = 0; connect_types[j] != NULL; j++)
		    if(0 == strcmp(string2,connect_types[j]))
		        break;
		  if(connect_types[j] == NULL)
		    {
		      fprintf(stderr,"stdout: incorrect connection protocal on fd %d: %s\n",connected_fds[i],buf);
		      timestamp(fplog);
		      fprintf(fplog,"stdout: incorrect connection protocal on fd %d: %s\n",connected_fds[i],buf);
		      fflush(fplog);
		      close(connected_fds[i]);
		      connected_fds[i] = -1;
		      continue;
		     }
		   if(valid_fds[connect_ids[j]] != -1)
		     {
		       fprintf(stdout,"ccd_dc: This is a second connection for purpose %s which is not allowed:\n",string2);
		       fprintf(stdout,"        The original connection must be terminated first.\n");
		       fprintf(stdout,"        Connection on fd %d is refused.\n",connected_fds[i]);
		       fprintf(fplog,"ccd_dc: This is a second connection for purpose %s which is not allowed:\n",string2);
		       fprintf(fplog,"        The original connection must be terminated first.\n");
		       fprintf(fplog,"        Connection on fd %d is refused.\n",connected_fds[i]);
		       fflush(fplog);
		       close(connected_fds[i]);
		       connected_fds[i] = -1;
		       continue;
		     }
		   valid_fds[connect_ids[j]] = connected_fds[i];
		   got_valid = connect_ids[j];
		   fprintf(stdout,"ccd_dc_server: CONN:          connection validated for %s on fd %d\n",
			   	string2,connected_fds[i]);
		   timestamp(fplog);
		   fprintf(fplog,"ccd_dc: connection validated for %s on fd %d\n",string2,connected_fds[i]);
		   fflush(fplog);
		}
	    }
	/*
	 *	Clean up some details in case we got a new valid
	 *	connection.
	 */

	switch(got_valid)
	  {
	    case COMMAND_ID:
		fdcom = valid_fds[got_valid];
		break;
	    case OUTPUT_ID:
		fdout = valid_fds[got_valid];
		fpout = fdopen(fdout,"r+");
		break;
	    case STATUS_ID:
		fdstat = valid_fds[got_valid];
		break;
	    case XFORM_ID:
		fdxfcm = valid_fds[got_valid];
		enqueue_fcn(ccd_check_alive,XFORM_ID,1.0);
		break;
	    case DET_CMD_ID:
		fddetcmd = valid_fds[got_valid];
		enqueue_fcn(ccd_check_alive,DET_CMD_ID,1.0);
		break;
	    case DET_STATUS_ID:
		fddetstatus = valid_fds[got_valid];
		enqueue_fcn(ccd_check_alive,DET_STATUS_ID,1.0);
		break;
	    case BL_CMD_ID:
		fdblcmd = valid_fds[got_valid];
		enqueue_fcn(ccd_check_alive,BL_CMD_ID,1.0);
		break;
	    case BL_STATUS_ID:
		fdblstatus = valid_fds[got_valid];
		enqueue_fcn(ccd_check_alive,BL_STATUS_ID,1.0);
		break;
	    default:
		break;
	  }
	check_other_servers();
	enqueue_fcn(ccd_server_update,0,1.0);
	return;		/* timed out */
  }

cleanexit(int status)
  {
    char message[80];

    fprintf(stdout,"ccd_dc       : EXIT:          shutting down with status %d\n",status);
    if(server_s != -1)
      {
	shutdown(server_s,2);
	close(server_s);
      }
    message[0] = '\0';
    if (status == GOOD_STATUS)
      {
        issue_signal("CCD_SIG_DC_STOP",message);
      }
    else
      {
        issue_signal("CCD_SIG_DC_DIED",message);
      }
    exit(status);
  }
