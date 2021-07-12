<!DOCTYPE html>
<html>
<head>
<base href="https://sqlite.org/althttpd/doc/tip/althttpd.c" />
<meta charset="UTF-8">
<meta http-equiv="Content-Security-Policy" content="default-src 'self' data:; script-src 'self' 'nonce-1f98b6c0f84b06e6eb04cd0e4b1318f5124cd24ebaab43aa'; style-src 'self' 'unsafe-inline'; img-src * data:" />
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Althttpd: althttpd.c</title>
<link rel="alternate" type="application/rss+xml" title="RSS Feed"  href="/althttpd/timeline.rss" />
<link rel="stylesheet" href="/althttpd/style.css?id=a7a8ce4d" type="text/css" />
</head>
<body class="artifact">
<div class="header">
  <div class="title"><h1>Althttpd</h1>althttpd.c</div>
    <div class="status"><a href='/althttpd/login'>Login</a>
</div>
</div>
<div class="mainmenu">
<a id='hbbtn' href='/althttpd/sitemap' aria-label='Site Map'>&#9776;</a><a href='/althttpd/home' class=''>Home</a>
<a href='/althttpd/timeline' class=''>Timeline</a>
<a href='/althttpd/brlist' class='wideonly'>Branches</a>
<a href='/althttpd/taglist' class='wideonly'>Tags</a>
<a href='/althttpd/forum' class='desktoponly'>Forum</a>
<a href='/althttpd/ticket' class='wideonly'>Tickets</a>
<a href='/althttpd/wiki' class='wideonly'>Wiki</a>
<a href='/althttpd/login' class='wideonly'>Login</a>
</div>
<div id='hbdrop'></div>
<form id='f01' method='GET' action='/althttpd/file'>
<input type='hidden' name='udc' value='1'>
<div class="submenu">
<a class="label" href="/althttpd/annotate?filename=althttpd.c&amp;checkin=tip">Annotate</a>
<a class="label" href="/althttpd/artifact/001b7cc47f">Artifact</a>
<a class="label" href="/althttpd/blame?filename=althttpd.c&amp;checkin=tip">Blame</a>
<a class="label" href="/althttpd/timeline?uf=001b7cc47f3f2cbc7899ecb3dd16cc359baec3e1672c32414354c499d37c17ce">Check-ins Using</a>
<a class="label" href="/althttpd/raw/001b7cc47f3f2cbc7899ecb3dd16cc359baec3e1672c32414354c499d37c17ce?at=althttpd.c">Download</a>
<a class="label" href="/althttpd/hexdump?name=001b7cc47f3f2cbc7899ecb3dd16cc359baec3e1672c32414354c499d37c17ce">Hex</a>
<label class='submenuctrl submenuckbox'><input type='checkbox' name='ln' id='submenuctrl-0' >Line Numbers</label>
</div>
<input type="hidden" name="name" value="althttpd.c">
</form>
<div class="content"><span id="debugMsg"></span>
<h2>File althttpd.c</a>
from the latest check-in</a></h2>
<hr />
<blockquote class="file-content">
<pre>
/*
** 2001-09-15
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
**
** This source code file implements a small, simple, stand-alone HTTP
** server.  
**
** Features:
**
**     * Launched from inetd/xinetd/stunnel4, or as a stand-alone server
**     * One process per request
**     * Deliver static content or run CGI or SCGI
**     * Virtual sites based on the &quot;Host:&quot; property of the HTTP header
**     * Runs in a chroot jail
**     * Unified log file in a CSV format
**     * Small code base (this 1 file) to facilitate security auditing
**     * Simple setup - no configuration files to misconfigure
** 
** This file implements a small and simple but secure and effective web
** server.  There are no frills.  Anything that could be reasonably
** omitted has been.
**
** Setup rules:
**
**    (1) Launch as root from inetd like this:
**
**            httpd -logfile logfile -root /home/www -user nobody
**
**        It will automatically chroot to /home/www and become user &quot;nobody&quot;.
**        The logfile name should be relative to the chroot jail.
**
**    (2) Directories of the form &quot;*.website&quot; (ex: www_sqlite_org.website)
**        contain content.  The directory is chosen based on the HTTP_HOST
**        request header.  If there is no HTTP_HOST header or if the
**        corresponding host directory does not exist, then the
**        &quot;default.website&quot; is used.  If the HTTP_HOST header contains any
**        charaters other than [a-zA-Z0-9_.,*~/] then a 403 error is
**        generated.
**
**    (3) Any file or directory whose name begins with &quot;.&quot; or &quot;-&quot; is ignored,
**        except if the URL begins with &quot;/.well-known/&quot; then initial &quot;.&quot; and
**        &quot;-&quot; characters are allowed, but not initial &quot;..&quot;.  The exception is
**        for RFC-5785 to allow letsencrypt or certbot to generate a TLS cert
**        using webroot.
**
**    (4) Characters other than [0-9a-zA-Z,-./:_~] and any %HH characters
**        escapes in the filename are all translated into &quot;_&quot;.  This is
**        a defense against cross-site scripting attacks and other mischief.
**
**    (5) Executable files are run as CGI.  Files whose name ends with &quot;.scgi&quot;
**        trigger and SCGI request (see item 10 below).  All other files
**        are delivered as is.
**
**    (6) For SSL support use stunnel and add the -https 1 option on the
**        httpd command-line.
**
**    (7) If a file named &quot;-auth&quot; exists in the same directory as the file to
**        be run as CGI or to be delivered, then it contains information
**        for HTTP Basic authorization.  See file format details below.
**
**    (8) To run as a stand-alone server, simply add the &quot;-port N&quot; command-line
**        option to define which TCP port to listen on.
**
**    (9) For static content, the mimetype is determined by the file suffix
**        using a table built into the source code below.  If you have
**        unusual content files, you might need to extend this table.
**
**   (10) Content files that end with &quot;.scgi&quot; and that contain text of the
**        form &quot;SCGI hostname port&quot; will format an SCGI request and send it
**        to hostname:port, the relay back the reply.  Error behavior is
**        determined by subsequent lines of the .scgi file.  See SCGI below
**        for details.
**
** Command-line Options:
**
**  --root DIR       Defines the directory that contains the various
**                   $HOST.website subdirectories, each containing web content 
**                   for a single virtual host.  If launched as root and if
**                   &quot;--user USER&quot; also appears on the command-line and if
**                   &quot;--jail 0&quot; is omitted, then the process runs in a chroot
**                   jail rooted at this directory and under the userid USER.
**                   This option is required for xinetd launch but defaults
**                   to &quot;.&quot; for a stand-alone web server.
**
**  --port N         Run in standalone mode listening on TCP port N
**
**  --user USER      Define the user under which the process should run if
**                   originally launched as root.  This process will refuse to
**                   run as root (for security).  If this option is omitted and
**                   the process is launched as root, it will abort without
**                   processing any HTTP requests.
**
**  --logfile FILE   Append a single-line, CSV-format, log file entry to FILE
**                   for each HTTP request.  FILE should be a full pathname.
**                   The FILE name is interpreted inside the chroot jail.  The
**                   FILE name is expanded using strftime() if it contains
**                   at least one &#39;%&#39; and is not too long.
**
**  --https          Indicates that input is coming over SSL and is being
**                   decoded upstream, perhaps by stunnel.  (This program
**                   only understands plaintext.)
**
**  --family ipv4    Only accept input from IPV4 or IPV6, respectively.
**  --family ipv6    These options are only meaningful if althttpd is run
**                   as a stand-alone server.
**
**  --jail BOOLEAN   Indicates whether or not to form a chroot jail if 
**                   initially run as root.  The default is true, so the only
**                   useful variant of this option is &quot;--jail 0&quot; which prevents
**                   the formation of the chroot jail.
**
**  --max-age SEC    The value for &quot;Cache-Control: max-age=%d&quot;.  Defaults to
**                   120 seconds.
**
**  --max-cpu SEC    Maximum number of seconds of CPU time allowed per
**                   HTTP connection.  Default 30.  0 means no limit.
**
**  --debug          Disables input timeouts.  This is useful for debugging
**                   when inputs is being typed in manually.
**
** Command-line options can take either one or two initial &quot;-&quot; characters.
** So &quot;--debug&quot; and &quot;-debug&quot; mean the same thing, for example.
**
**
** Security Features:
**
** (1)  This program automatically puts itself inside a chroot jail if
**      it can and if not specifically prohibited by the &quot;--jail 0&quot;
**      command-line option.  The root of the jail is the directory that
**      contains the various $HOST.website content subdirectories.
**
** (2)  No input is read while this process has root privileges.  Root
**      privileges are dropped prior to reading any input (but after entering
**      the chroot jail, of course).  If root privileges cannot be dropped
**      (for example because the --user command-line option was omitted or
**      because the user specified by the --user option does not exist), 
**      then the process aborts with an error prior to reading any input.
**
** (3)  The length of an HTTP request is limited to MAX_CONTENT_LENGTH bytes
**      (default: 250 million).  Any HTTP request longer than this fails
**      with an error.
**
** (4)  There are hard-coded time-outs on each HTTP request.  If this process
**      waits longer than the timeout for the complete request, or for CGI
**      to finish running, then this process aborts.  (The timeout feature
**      can be disabled using the --debug command-line option.)
**
** (5)  If the HTTP_HOST request header contains characters other than
**      [0-9a-zA-Z,-./:_~] then the entire request is rejected.
**
** (6)  Any characters in the URI pathname other than [0-9a-zA-Z,-./:_~]
**      are converted into &quot;_&quot;.  This applies to the pathname only, not
**      to the query parameters or fragment.
**
** (7)  If the first character of any URI pathname component is &quot;.&quot; or &quot;-&quot;
**      then a 404 Not Found reply is generated.  This prevents attacks
**      such as including &quot;..&quot; or &quot;.&quot; directory elements in the pathname
**      and allows placing files and directories in the content subdirectory
**      that are invisible to all HTTP requests, by making the first 
**      character of the file or subdirectory name &quot;-&quot; or &quot;.&quot;.
**
** (8)  The request URI must begin with &quot;/&quot; or else a 404 error is generated.
**
** (9)  This program never sets the value of an environment variable to a
**      string that begins with &quot;() {&quot;.
**
** Security Auditing:
**
** This webserver mostly only serves static content.  Any security risk will
** come from CGI and SCGI.  To check an installation for security, then, it
** makes sense to focus on the CGI and SCGI scripts.
**
** To local all CGI files:
**
**          find *.website -executable -type f -print
**     OR:  find *.website -perm +0111 -type f -print
**
** The first form of the &quot;find&quot; command is preferred, but is only supported
** by GNU find.  On a Mac, you&#39;ll have to use the second form.
**
** To find all SCGI files:
**
**          find *.website -name &#39;*.scgi&#39; -type f -print
**
** If any file is a security concern, it can be disabled on a live
** installation by turning off read permissions:
**
**          chmod 0000 file-of-concern
**
** SCGI Specification Files:
**
** Content files (files without the execute bit set) that end with &quot;.scgi&quot;
** specify a connection to an SCGI server.  The format of the .scgi file
** follows this template:
**
**      SCGI hostname port
**      fallback: fallback-filename
**      relight: relight-command
**
** The first line specifies the location and TCP/IP port of the SCGI server
** that will handle the request.  Subsequent lines determine what to do if
** the SCGI server cannot be contacted.  If the &quot;relight:&quot; line is present,
** then the relight-command is run using system() and the connection is
** retried after a 1-second delay.  Use &quot;&amp;&quot; at the end of the relight-command
** to run it in the background.  Make sure the relight-command does not
** send generate output, or that output will become part of the SCGI reply.
** Add a &quot;&gt;/dev/null&quot; suffix (before the &quot;&amp;&quot;) to the relight-command if
** necessary to suppress output.  If there is no relight-command, or if the
** relight is attempted but the SCGI server still cannot be contacted, then
** the content of the fallback-filename file is returned as a substitute for
** the SCGI request.  The mimetype is determined by the suffix on the
** fallback-filename.  The fallback-filename would typically be an error
** message indicating that the service is temporarily unavailable.
**
** Basic Authorization:
**
** If the file &quot;-auth&quot; exists in the same directory as the content file
** (for both static content and CGI) then it contains the information used
** for basic authorization.  The file format is as follows:
**
**    *  Blank lines and lines that begin with &#39;#&#39; are ignored
**    *  &quot;http-redirect&quot; forces a redirect to HTTPS if not there already
**    *  &quot;https-only&quot; disallows operation in HTTP
**    *  &quot;user NAME LOGIN:PASSWORD&quot; checks to see if LOGIN:PASSWORD 
**       authorization credentials are provided, and if so sets the
**       REMOTE_USER to NAME.
**    *  &quot;realm TEXT&quot; sets the realm to TEXT.
**
** There can be multiple &quot;user&quot; lines.  If no &quot;user&quot; line matches, the
** request fails with a 401 error.
**
** Because of security rule (7), there is no way for the content of the &quot;-auth&quot;
** file to leak out via HTTP request.
*/
#include &lt;stdio.h&gt;
#include &lt;ctype.h&gt;
#include &lt;syslog.h&gt;
#include &lt;stdlib.h&gt;
#include &lt;sys/stat.h&gt;
#include &lt;unistd.h&gt;
#include &lt;fcntl.h&gt;
#include &lt;string.h&gt;
#include &lt;pwd.h&gt;
#include &lt;sys/time.h&gt;
#include &lt;sys/types.h&gt;
#include &lt;sys/resource.h&gt;
#include &lt;sys/socket.h&gt;
#include &lt;sys/wait.h&gt;
#include &lt;netinet/in.h&gt;
#include &lt;arpa/inet.h&gt;
#include &lt;stdarg.h&gt;
#include &lt;time.h&gt;
#include &lt;sys/times.h&gt;
#include &lt;netdb.h&gt;
#include &lt;errno.h&gt;
#include &lt;sys/resource.h&gt;
#include &lt;signal.h&gt;
#ifdef linux
#include &lt;sys/sendfile.h&gt;
#endif
#include &lt;assert.h&gt;

/*
** Configure the server by setting the following macros and recompiling.
*/
#ifndef DEFAULT_PORT
#define DEFAULT_PORT &quot;80&quot;             /* Default TCP port for HTTP */
#endif
#ifndef MAX_CONTENT_LENGTH
#define MAX_CONTENT_LENGTH 250000000  /* Max length of HTTP request content */
#endif
#ifndef MAX_CPU
#define MAX_CPU 30                /* Max CPU cycles in seconds */
#endif

/*
** We record most of the state information as global variables.  This
** saves having to pass information to subroutines as parameters, and
** makes the executable smaller...
*/
static char *zRoot = 0;          /* Root directory of the website */
static char *zTmpNam = 0;        /* Name of a temporary file */
static char zTmpNamBuf[500];     /* Space to hold the temporary filename */
static char *zProtocol = 0;      /* The protocol being using by the browser */
static char *zMethod = 0;        /* The method.  Must be GET */
static char *zScript = 0;        /* The object to retrieve */
static char *zRealScript = 0;    /* The object to retrieve.  Same as zScript
                                 ** except might have &quot;/index.html&quot; appended */
static char *zHome = 0;          /* The directory containing content */
static char *zQueryString = 0;   /* The query string on the end of the name */
static char *zFile = 0;          /* The filename of the object to retrieve */
static int lenFile = 0;          /* Length of the zFile name */
static char *zDir = 0;           /* Name of the directory holding zFile */
static char *zPathInfo = 0;      /* Part of the pathname past the file */
static char *zAgent = 0;         /* What type if browser is making this query */
static char *zServerName = 0;    /* The name after the http:// */
static char *zServerPort = 0;    /* The port number */
static char *zCookie = 0;        /* Cookies reported with the request */
static char *zHttpHost = 0;      /* Name according to the web browser */
static char *zRealPort = 0;      /* The real TCP port when running as daemon */
static char *zRemoteAddr = 0;    /* IP address of the request */
static char *zReferer = 0;       /* Name of the page that refered to us */
static char *zAccept = 0;        /* What formats will be accepted */
static char *zAcceptEncoding =0; /* gzip or default */
static char *zContentLength = 0; /* Content length reported in the header */
static char *zContentType = 0;   /* Content type reported in the header */
static char *zQuerySuffix = 0;   /* The part of the URL after the first ? */
static char *zAuthType = 0;      /* Authorization type (basic or digest) */
static char *zAuthArg = 0;       /* Authorization values */
static char *zRemoteUser = 0;    /* REMOTE_USER set by authorization module */
static char *zIfNoneMatch= 0;    /* The If-None-Match header value */
static char *zIfModifiedSince=0; /* The If-Modified-Since header value */
static int nIn = 0;              /* Number of bytes of input */
static int nOut = 0;             /* Number of bytes of output */
static char zReplyStatus[4];     /* Reply status code */
static int statusSent = 0;       /* True after status line is sent */
static char *zLogFile = 0;       /* Log to this file */
static int debugFlag = 0;        /* True if being debugged */
static struct timeval beginTime; /* Time when this process starts */
static int closeConnection = 0;  /* True to send Connection: close in reply */
static int nRequest = 0;         /* Number of requests processed */
static int omitLog = 0;          /* Do not make logfile entries if true */
static int useHttps = 0;         /* True to use HTTPS: instead of HTTP: */
static char *zHttp = &quot;http&quot;;     /* http or https */
static int useTimeout = 1;       /* True to use times */
static int standalone = 0;       /* Run as a standalone server (no inetd) */
static int ipv6Only = 0;         /* Use IPv6 only */
static int ipv4Only = 0;         /* Use IPv4 only */
static struct rusage priorSelf;  /* Previously report SELF time */
static struct rusage priorChild; /* Previously report CHILD time */
static int mxAge = 120;          /* Cache-control max-age */
static char *default_path = &quot;/bin:/usr/bin&quot;;  /* Default PATH variable */
static char *zScgi = 0;          /* Value of the SCGI env variable */
static int rangeStart = 0;       /* Start of a Range: request */
static int rangeEnd = 0;         /* End of a Range: request */
static int maxCpu = MAX_CPU;     /* Maximum CPU time per process */

/*
** Mapping between CGI variable names and values stored in
** global variables.
*/
static struct {
  char *zEnvName;
  char **pzEnvValue;
} cgienv[] = {
  { &quot;CONTENT_LENGTH&quot;,          &amp;zContentLength }, /* Must be first for SCGI */
  { &quot;AUTH_TYPE&quot;,                   &amp;zAuthType },
  { &quot;AUTH_CONTENT&quot;,                &amp;zAuthArg },
  { &quot;CONTENT_TYPE&quot;,                &amp;zContentType },
  { &quot;DOCUMENT_ROOT&quot;,               &amp;zHome },
  { &quot;HTTP_ACCEPT&quot;,                 &amp;zAccept },
  { &quot;HTTP_ACCEPT_ENCODING&quot;,        &amp;zAcceptEncoding },
  { &quot;HTTP_COOKIE&quot;,                 &amp;zCookie },
  { &quot;HTTP_HOST&quot;,                   &amp;zHttpHost },
  { &quot;HTTP_IF_MODIFIED_SINCE&quot;,      &amp;zIfModifiedSince },
  { &quot;HTTP_IF_NONE_MATCH&quot;,          &amp;zIfNoneMatch },
  { &quot;HTTP_REFERER&quot;,                &amp;zReferer },
  { &quot;HTTP_USER_AGENT&quot;,             &amp;zAgent },
  { &quot;PATH&quot;,                        &amp;default_path },
  { &quot;PATH_INFO&quot;,                   &amp;zPathInfo },
  { &quot;QUERY_STRING&quot;,                &amp;zQueryString },
  { &quot;REMOTE_ADDR&quot;,                 &amp;zRemoteAddr },
  { &quot;REQUEST_METHOD&quot;,              &amp;zMethod },
  { &quot;REQUEST_URI&quot;,                 &amp;zScript },
  { &quot;REMOTE_USER&quot;,                 &amp;zRemoteUser },
  { &quot;SCGI&quot;,                        &amp;zScgi },
  { &quot;SCRIPT_DIRECTORY&quot;,            &amp;zDir },
  { &quot;SCRIPT_FILENAME&quot;,             &amp;zFile },
  { &quot;SCRIPT_NAME&quot;,                 &amp;zRealScript },
  { &quot;SERVER_NAME&quot;,                 &amp;zServerName },
  { &quot;SERVER_PORT&quot;,                 &amp;zServerPort },
  { &quot;SERVER_PROTOCOL&quot;,             &amp;zProtocol },
};


/*
** Double any double-quote characters in a string.
*/
static char *Escape(char *z){
  size_t i, j;
  size_t n;
  char c;
  char *zOut;
  for(i=0; (c=z[i])!=0 &amp;&amp; c!=&#39;&quot;&#39;; i++){}
  if( c==0 ) return z;
  n = 1;
  for(i++; (c=z[i])!=0; i++){ if( c==&#39;&quot;&#39; ) n++; }
  zOut = malloc( i+n+1 );
  if( zOut==0 ) return &quot;&quot;;
  for(i=j=0; (c=z[i])!=0; i++){
    zOut[j++] = c;
    if( c==&#39;&quot;&#39; ) zOut[j++] = c;
  }
  zOut[j] = 0;
  return zOut;
}

/*
** Convert a struct timeval into an integer number of microseconds
*/
static long long int tvms(struct timeval *p){
  return ((long long int)p-&gt;tv_sec)*1000000 + (long long int)p-&gt;tv_usec;
}

/*
** Make an entry in the log file.  If the HTTP connection should be
** closed, then terminate this process.  Otherwise return.
*/
static void MakeLogEntry(int exitCode, int lineNum){
  FILE *log;
  if( zTmpNam ){
    unlink(zTmpNam);
  }
  if( zLogFile &amp;&amp; !omitLog ){
    struct timeval now;
    struct tm *pTm;
    struct rusage self, children;
    int waitStatus;
    char *zRM = zRemoteUser ? zRemoteUser : &quot;&quot;;
    char *zFilename;
    size_t sz;
    char zDate[200];
    char zExpLogFile[500];

    if( zScript==0 ) zScript = &quot;&quot;;
    if( zRealScript==0 ) zRealScript = &quot;&quot;;
    if( zRemoteAddr==0 ) zRemoteAddr = &quot;&quot;;
    if( zHttpHost==0 ) zHttpHost = &quot;&quot;;
    if( zReferer==0 ) zReferer = &quot;&quot;;
    if( zAgent==0 ) zAgent = &quot;&quot;;
    gettimeofday(&amp;now, 0);
    pTm = localtime(&amp;now.tv_sec);
    strftime(zDate, sizeof(zDate), &quot;%Y-%m-%d %H:%M:%S&quot;, pTm);
    sz = strftime(zExpLogFile, sizeof(zExpLogFile), zLogFile, pTm);
    if( sz&gt;0 &amp;&amp; sz&lt;sizeof(zExpLogFile)-2 ){
      zFilename = zExpLogFile;
    }else{
      zFilename = zLogFile;
    }
    waitpid(-1, &amp;waitStatus, WNOHANG);
    getrusage(RUSAGE_SELF, &amp;self);
    getrusage(RUSAGE_CHILDREN, &amp;children);
    if( (log = fopen(zFilename,&quot;a&quot;))!=0 ){
#ifdef COMBINED_LOG_FORMAT
      strftime(zDate, sizeof(zDate), &quot;%d/%b/%Y:%H:%M:%S %Z&quot;, pTm);
      fprintf(log, &quot;%s - - [%s] \&quot;%s %s %s\&quot; %s %d \&quot;%s\&quot; \&quot;%s\&quot;\n&quot;,
              zRemoteAddr, zDate, zMethod, zScript, zProtocol,
              zReplyStatus, nOut, zReferer, zAgent);
#else
      strftime(zDate, sizeof(zDate), &quot;%Y-%m-%d %H:%M:%S&quot;, pTm);
      /* Log record files:
      **  (1) Date and time
      **  (2) IP address
      **  (3) URL being accessed
      **  (4) Referer
      **  (5) Reply status
      **  (6) Bytes received
      **  (7) Bytes sent
      **  (8) Self user time
      **  (9) Self system time
      ** (10) Children user time
      ** (11) Children system time
      ** (12) Total wall-clock time
      ** (13) Request number for same TCP/IP connection
      ** (14) User agent
      ** (15) Remote user
      ** (16) Bytes of URL that correspond to the SCRIPT_NAME
      ** (17) Line number in source file
      */
      fprintf(log,
        &quot;%s,%s,\&quot;%s://%s%s\&quot;,\&quot;%s\&quot;,&quot;
           &quot;%s,%d,%d,%lld,%lld,%lld,%lld,%lld,%d,\&quot;%s\&quot;,\&quot;%s\&quot;,%d,%d\n&quot;,
        zDate, zRemoteAddr, zHttp, Escape(zHttpHost), Escape(zScript),
        Escape(zReferer), zReplyStatus, nIn, nOut,
        tvms(&amp;self.ru_utime) - tvms(&amp;priorSelf.ru_utime),
        tvms(&amp;self.ru_stime) - tvms(&amp;priorSelf.ru_stime),
        tvms(&amp;children.ru_utime) - tvms(&amp;priorChild.ru_utime),
        tvms(&amp;children.ru_stime) - tvms(&amp;priorChild.ru_stime),
        tvms(&amp;now) - tvms(&amp;beginTime),
        nRequest, Escape(zAgent), Escape(zRM),
        (int)(strlen(zHttp)+strlen(zHttpHost)+strlen(zRealScript)+3),
        lineNum
      );
      priorSelf = self;
      priorChild = children;
#endif
      fclose(log);
      nIn = nOut = 0;
    }
  }
  if( closeConnection ){
    exit(exitCode);
  }
  statusSent = 0;
}

/*
** Allocate memory safely
*/
static char *SafeMalloc( size_t size ){
  char *p;

  p = (char*)malloc(size);
  if( p==0 ){
    strcpy(zReplyStatus, &quot;998&quot;);
    MakeLogEntry(1,100);  /* LOG: Malloc() failed */
    exit(1);
  }
  return p;
}

/*
** Set the value of environment variable zVar to zValue.
*/
static void SetEnv(const char *zVar, const char *zValue){
  char *z;
  size_t len;
  if( zValue==0 ) zValue=&quot;&quot;;
  /* Disable an attempted bashdoor attack */
  if( strncmp(zValue,&quot;() {&quot;,4)==0 ) zValue = &quot;&quot;;
  len = strlen(zVar) + strlen(zValue) + 2;
  z = SafeMalloc(len);
  sprintf(z,&quot;%s=%s&quot;,zVar,zValue);
  putenv(z);
}

/*
** Remove the first space-delimited token from a string and return
** a pointer to it.  Add a NULL to the string to terminate the token.
** Make *zLeftOver point to the start of the next token.
*/
static char *GetFirstElement(char *zInput, char **zLeftOver){
  char *zResult = 0;
  if( zInput==0 ){
    if( zLeftOver ) *zLeftOver = 0;
    return 0;
  }
  while( isspace(*(unsigned char*)zInput) ){ zInput++; }
  zResult = zInput;
  while( *zInput &amp;&amp; !isspace(*(unsigned char*)zInput) ){ zInput++; }
  if( *zInput ){
    *zInput = 0;
    zInput++;
    while( isspace(*(unsigned char*)zInput) ){ zInput++; }
  }
  if( zLeftOver ){ *zLeftOver = zInput; }
  return zResult;
}

/*
** Make a copy of a string into memory obtained from malloc.
*/
static char *StrDup(const char *zSrc){
  char *zDest;
  size_t size;

  if( zSrc==0 ) return 0;
  size = strlen(zSrc) + 1;
  zDest = (char*)SafeMalloc( size );
  strcpy(zDest,zSrc);
  return zDest;
}
static char *StrAppend(char *zPrior, const char *zSep, const char *zSrc){
  char *zDest;
  size_t size;
  size_t n0, n1, n2;

  if( zSrc==0 ) return 0;
  if( zPrior==0 ) return StrDup(zSrc);
  n0 = strlen(zPrior);
  n1 = strlen(zSep);
  n2 = strlen(zSrc);
  size = n0+n1+n2+1;
  zDest = (char*)SafeMalloc( size );
  memcpy(zDest, zPrior, n0);
  free(zPrior);
  memcpy(&amp;zDest[n0],zSep,n1);
  memcpy(&amp;zDest[n0+n1],zSrc,n2+1);
  return zDest;
}

/*
** Compare two ETag values. Return 0 if they match and non-zero if they differ.
**
** The one on the left might be a NULL pointer and it might be quoted.
*/
static int CompareEtags(const char *zA, const char *zB){
  if( zA==0 ) return 1;
  if( zA[0]==&#39;&quot;&#39; ){
    int lenB = (int)strlen(zB);
    if( strncmp(zA+1, zB, lenB)==0 &amp;&amp; zA[lenB+1]==&#39;&quot;&#39; ) return 0;
  }
  return strcmp(zA, zB);
}

/*
** Break a line at the first \n or \r character seen.
*/
static void RemoveNewline(char *z){
  if( z==0 ) return;
  while( *z &amp;&amp; *z!=&#39;\n&#39; &amp;&amp; *z!=&#39;\r&#39; ){ z++; }
  *z = 0;
}

/* Render seconds since 1970 as an RFC822 date string.  Return
** a pointer to that string in a static buffer.
*/
static char *Rfc822Date(time_t t){
  struct tm *tm;
  static char zDate[100];
  tm = gmtime(&amp;t);
  strftime(zDate, sizeof(zDate), &quot;%a, %d %b %Y %H:%M:%S %Z&quot;, tm);
  return zDate;
}

/*
** Print a date tag in the header.  The name of the tag is zTag.
** The date is determined from the unix timestamp given.
*/
static int DateTag(const char *zTag, time_t t){
  return printf(&quot;%s: %s\r\n&quot;, zTag, Rfc822Date(t));
}

/*
** Parse an RFC822-formatted timestamp as we&#39;d expect from HTTP and return
** a Unix epoch time. &lt;= zero is returned on failure.
*/
time_t ParseRfc822Date(const char *zDate){
  int mday, mon, year, yday, hour, min, sec;
  char zIgnore[4];
  char zMonth[4];
  static const char *const azMonths[] =
    {&quot;Jan&quot;, &quot;Feb&quot;, &quot;Mar&quot;, &quot;Apr&quot;, &quot;May&quot;, &quot;Jun&quot;,
     &quot;Jul&quot;, &quot;Aug&quot;, &quot;Sep&quot;, &quot;Oct&quot;, &quot;Nov&quot;, &quot;Dec&quot;};
  if( 7==sscanf(zDate, &quot;%3[A-Za-z], %d %3[A-Za-z] %d %d:%d:%d&quot;, zIgnore,
                       &amp;mday, zMonth, &amp;year, &amp;hour, &amp;min, &amp;sec)){
    if( year &gt; 1900 ) year -= 1900;
    for(mon=0; mon&lt;12; mon++){
      if( !strncmp( azMonths[mon], zMonth, 3 )){
        int nDay;
        int isLeapYr;
        static int priorDays[] =
         {  0, 31, 59, 90,120,151,181,212,243,273,304,334 };
        isLeapYr = year%4==0 &amp;&amp; (year%100!=0 || (year+300)%400==0);
        yday = priorDays[mon] + mday - 1;
        if( isLeapYr &amp;&amp; mon&gt;1 ) yday++;
        nDay = (year-70)*365 + (year-69)/4 - year/100 + (year+300)/400 + yday;
        return ((time_t)(nDay*24 + hour)*60 + min)*60 + sec;
      }
    }
  }
  return 0;
}

/*
** Test procedure for ParseRfc822Date
*/
void TestParseRfc822Date(void){
  time_t t1, t2;
  for(t1=0; t1&lt;0x7fffffff; t1 += 127){
    t2 = ParseRfc822Date(Rfc822Date(t1));
    assert( t1==t2 );
  }
}

/*
** Print the first line of a response followed by the server type.
*/
static void StartResponse(const char *zResultCode){
  time_t now;
  time(&amp;now);
  if( statusSent ) return;
  nOut += printf(&quot;%s %s\r\n&quot;, zProtocol, zResultCode);
  strncpy(zReplyStatus, zResultCode, 3);
  zReplyStatus[3] = 0;
  if( zReplyStatus[0]&gt;=&#39;4&#39; ){
    closeConnection = 1;
  }
  if( closeConnection ){
    nOut += printf(&quot;Connection: close\r\n&quot;);
  }else{
    nOut += printf(&quot;Connection: keep-alive\r\n&quot;);
  }
  nOut += DateTag(&quot;Date&quot;, now);
  statusSent = 1;
}

/*
** Tell the client that there is no such document
*/
static void NotFound(int lineno){
  StartResponse(&quot;404 Not Found&quot;);
  nOut += printf(
    &quot;Content-type: text/html; charset=utf-8\r\n&quot;
    &quot;\r\n&quot;
    &quot;&lt;head&gt;&lt;title lineno=\&quot;%d\&quot;&gt;Not Found&lt;/title&gt;&lt;/head&gt;\n&quot;
    &quot;&lt;body&gt;&lt;h1&gt;Document Not Found&lt;/h1&gt;\n&quot;
    &quot;The document %s is not available on this server\n&quot;
    &quot;&lt;/body&gt;\n&quot;, lineno, zScript);
  MakeLogEntry(0, lineno);
  exit(0);
}

/*
** Tell the client that they are not welcomed here.
*/
static void Forbidden(int lineno){
  StartResponse(&quot;403 Forbidden&quot;);
  nOut += printf(
    &quot;Content-type: text/plain; charset=utf-8\r\n&quot;
    &quot;\r\n&quot;
    &quot;Access denied\n&quot;
  );
  closeConnection = 1;
  MakeLogEntry(0, lineno);
  exit(0);
}

/*
** Tell the client that authorization is required to access the
** document.
*/
static void NotAuthorized(const char *zRealm){
  StartResponse(&quot;401 Authorization Required&quot;);
  nOut += printf(
    &quot;WWW-Authenticate: Basic realm=\&quot;%s\&quot;\r\n&quot;
    &quot;Content-type: text/html; charset=utf-8\r\n&quot;
    &quot;\r\n&quot;
    &quot;&lt;head&gt;&lt;title&gt;Not Authorized&lt;/title&gt;&lt;/head&gt;\n&quot;
    &quot;&lt;body&gt;&lt;h1&gt;401 Not Authorized&lt;/h1&gt;\n&quot;
    &quot;A login and password are required for this document\n&quot;
    &quot;&lt;/body&gt;\n&quot;, zRealm);
  MakeLogEntry(0, 110);  /* LOG: Not authorized */
}

/*
** Tell the client that there is an error in the script.
*/
static void CgiError(void){
  StartResponse(&quot;500 Error&quot;);
  nOut += printf(
    &quot;Content-type: text/html; charset=utf-8\r\n&quot;
    &quot;\r\n&quot;
    &quot;&lt;head&gt;&lt;title&gt;CGI Program Error&lt;/title&gt;&lt;/head&gt;\n&quot;
    &quot;&lt;body&gt;&lt;h1&gt;CGI Program Error&lt;/h1&gt;\n&quot;
    &quot;The CGI program %s generated an error\n&quot;
    &quot;&lt;/body&gt;\n&quot;, zScript);
  MakeLogEntry(0, 120);  /* LOG: CGI Error */
  exit(0);
}

/*
** This is called if we timeout or catch some other kind of signal.
** Log an error code which is 900+iSig and then quit.
*/
static void Timeout(int iSig){
  if( !debugFlag ){
    if( zScript &amp;&amp; zScript[0] ){
      char zBuf[10];
      zBuf[0] = &#39;9&#39;;
      zBuf[1] = &#39;0&#39; + (iSig/10)%10;
      zBuf[2] = &#39;0&#39; + iSig%10;
      zBuf[3] = 0;
      strcpy(zReplyStatus, zBuf);
      MakeLogEntry(0, 130);  /* LOG: Timeout */
    }
    exit(0);
  }
}

/*
** Tell the client that there is an error in the script.
*/
static void CgiScriptWritable(void){
  StartResponse(&quot;500 CGI Configuration Error&quot;);
  nOut += printf(
    &quot;Content-type: text/plain; charset=utf-8\r\n&quot;
    &quot;\r\n&quot;
    &quot;The CGI program %s is writable by users other than its owner.\n&quot;,
    zRealScript);
  MakeLogEntry(0, 140);  /* LOG: CGI script is writable */
  exit(0);       
}

/*
** Tell the client that the server malfunctioned.
*/
static void Malfunction(int linenum, const char *zFormat, ...){
  va_list ap;
  va_start(ap, zFormat);
  StartResponse(&quot;500 Server Malfunction&quot;);
  nOut += printf(
    &quot;Content-type: text/plain; charset=utf-8\r\n&quot;
    &quot;\r\n&quot;
    &quot;Web server malfunctioned; error number %d\n\n&quot;, linenum);
  if( zFormat ){
    nOut += vprintf(zFormat, ap);
    printf(&quot;\n&quot;);
    nOut++;
  }
  MakeLogEntry(0, linenum);
  exit(0);       
}

/*
** Do a server redirect to the document specified.  The document
** name not contain scheme or network location or the query string.
** It will be just the path.
*/
static void Redirect(const char *zPath, int iStatus, int finish, int lineno){
  switch( iStatus ){
    case 301:
      StartResponse(&quot;301 Permanent Redirect&quot;);
      break;
    case 308:
      StartResponse(&quot;308 Permanent Redirect&quot;);
      break;
    default:
      StartResponse(&quot;302 Temporary Redirect&quot;);
      break;
  }
  if( zServerPort==0 || zServerPort[0]==0 || strcmp(zServerPort,&quot;80&quot;)==0 ){
    nOut += printf(&quot;Location: %s://%s%s%s\r\n&quot;,
                   zHttp, zServerName, zPath, zQuerySuffix);
  }else{
    nOut += printf(&quot;Location: %s://%s:%s%s%s\r\n&quot;,
                   zHttp, zServerName, zServerPort, zPath, zQuerySuffix);
  }
  if( finish ){
    nOut += printf(&quot;Content-length: 0\r\n&quot;);
    nOut += printf(&quot;\r\n&quot;);
    MakeLogEntry(0, lineno);
  }
  fflush(stdout);
}

/*
** This function treats its input as a base-64 string and returns the
** decoded value of that string.  Characters of input that are not
** valid base-64 characters (such as spaces and newlines) are ignored.
*/
void Decode64(char *z64){
  char *zData;
  int n64;
  int i, j;
  int a, b, c, d;
  static int isInit = 0;
  static int trans[128];
  static unsigned char zBase[] = 
    &quot;ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/&quot;;

  if( !isInit ){
    for(i=0; i&lt;128; i++){ trans[i] = 0; }
    for(i=0; zBase[i]; i++){ trans[zBase[i] &amp; 0x7f] = i; }
    isInit = 1;
  }
  n64 = strlen(z64);
  while( n64&gt;0 &amp;&amp; z64[n64-1]==&#39;=&#39; ) n64--;
  zData = z64;
  for(i=j=0; i+3&lt;n64; i+=4){
    a = trans[z64[i] &amp; 0x7f];
    b = trans[z64[i+1] &amp; 0x7f];
    c = trans[z64[i+2] &amp; 0x7f];
    d = trans[z64[i+3] &amp; 0x7f];
    zData[j++] = ((a&lt;&lt;2) &amp; 0xfc) | ((b&gt;&gt;4) &amp; 0x03);
    zData[j++] = ((b&lt;&lt;4) &amp; 0xf0) | ((c&gt;&gt;2) &amp; 0x0f);
    zData[j++] = ((c&lt;&lt;6) &amp; 0xc0) | (d &amp; 0x3f);
  }
  if( i+2&lt;n64 ){
    a = trans[z64[i] &amp; 0x7f];
    b = trans[z64[i+1] &amp; 0x7f];
    c = trans[z64[i+2] &amp; 0x7f];
    zData[j++] = ((a&lt;&lt;2) &amp; 0xfc) | ((b&gt;&gt;4) &amp; 0x03);
    zData[j++] = ((b&lt;&lt;4) &amp; 0xf0) | ((c&gt;&gt;2) &amp; 0x0f);
  }else if( i+1&lt;n64 ){
    a = trans[z64[i] &amp; 0x7f];
    b = trans[z64[i+1] &amp; 0x7f];
    zData[j++] = ((a&lt;&lt;2) &amp; 0xfc) | ((b&gt;&gt;4) &amp; 0x03);
  }
  zData[j] = 0;
}

/*
** Check to see if basic authorization credentials are provided for
** the user according to the information in zAuthFile.  Return true
** if authorized.  Return false if not authorized.
**
** File format:
**
**    *  Blank lines and lines that begin with &#39;#&#39; are ignored
**    *  &quot;http-redirect&quot; forces a redirect to HTTPS if not there already
**    *  &quot;https-only&quot; disallows operation in HTTP
**    *  &quot;user NAME LOGIN:PASSWORD&quot; checks to see if LOGIN:PASSWORD 
**       authorization credentials are provided, and if so sets the
**       REMOTE_USER to NAME.
**    *  &quot;realm TEXT&quot; sets the realm to TEXT.
**    *  &quot;anyone&quot; bypasses authentication and allows anyone to see the
**       files.  Useful in combination with &quot;http-redirect&quot;
*/
static int CheckBasicAuthorization(const char *zAuthFile){
  FILE *in;
  char *zRealm = &quot;unknown realm&quot;;
  char *zLoginPswd;
  char *zName;
  char zLine[2000];

  in = fopen(zAuthFile, &quot;rb&quot;);
  if( in==0 ){
    NotFound(150);  /* LOG: Cannot open -auth file */
    return 0;
  }
  if( zAuthArg ) Decode64(zAuthArg);
  while( fgets(zLine, sizeof(zLine), in) ){
    char *zFieldName;
    char *zVal;

    zFieldName = GetFirstElement(zLine,&amp;zVal);
    if( zFieldName==0 || *zFieldName==0 ) continue;
    if( zFieldName[0]==&#39;#&#39; ) continue;
    RemoveNewline(zVal);
    if( strcmp(zFieldName, &quot;realm&quot;)==0 ){
      zRealm = StrDup(zVal);
    }else if( strcmp(zFieldName,&quot;user&quot;)==0 ){
      if( zAuthArg==0 ) continue;
      zName = GetFirstElement(zVal, &amp;zVal);
      zLoginPswd = GetFirstElement(zVal, &amp;zVal);
      if( zLoginPswd==0 ) continue;
      if( zAuthArg &amp;&amp; strcmp(zAuthArg,zLoginPswd)==0 ){
        zRemoteUser = StrDup(zName);
        fclose(in);
        return 1;
      }
    }else if( strcmp(zFieldName,&quot;https-only&quot;)==0 ){
      if( !useHttps ){
        NotFound(160);  /* LOG:  http request on https-only page */
        fclose(in);
        return 0;
      }
    }else if( strcmp(zFieldName,&quot;http-redirect&quot;)==0 ){
      if( !useHttps ){
        zHttp = &quot;https&quot;;
        Redirect(zScript, 301, 1, 170); /* LOG: -auth redirect */
        fclose(in);
        return 0;
      }
    }else if( strcmp(zFieldName,&quot;anyone&quot;)==0 ){
      fclose(in);
      return 1;
    }else{
      NotFound(180);  /* LOG:  malformed entry in -auth file */
      fclose(in);
      return 0;
    }
  }
  fclose(in);
  NotAuthorized(zRealm);
  return 0;
}

/*
** Guess the mime-type of a document based on its name.
*/
const char *GetMimeType(const char *zName, int nName){
  const char *z;
  int i;
  int first, last;
  int len;
  char zSuffix[20];

  /* A table of mimetypes based on file suffixes. 
  ** Suffixes must be in sorted order so that we can do a binary
  ** search to find the mime-type
  */
  static const struct {
    const char *zSuffix;       /* The file suffix */
    int size;                  /* Length of the suffix */
    const char *zMimetype;     /* The corresponding mimetype */
  } aMime[] = {
    { &quot;ai&quot;,         2, &quot;application/postscript&quot;            },
    { &quot;aif&quot;,        3, &quot;audio/x-aiff&quot;                      },
    { &quot;aifc&quot;,       4, &quot;audio/x-aiff&quot;                      },
    { &quot;aiff&quot;,       4, &quot;audio/x-aiff&quot;                      },
    { &quot;arj&quot;,        3, &quot;application/x-arj-compressed&quot;      },
    { &quot;asc&quot;,        3, &quot;text/plain&quot;                        },
    { &quot;asf&quot;,        3, &quot;video/x-ms-asf&quot;                    },
    { &quot;asx&quot;,        3, &quot;video/x-ms-asx&quot;                    },
    { &quot;au&quot;,         2, &quot;audio/ulaw&quot;                        },
    { &quot;avi&quot;,        3, &quot;video/x-msvideo&quot;                   },
    { &quot;bat&quot;,        3, &quot;application/x-msdos-program&quot;       },
    { &quot;bcpio&quot;,      5, &quot;application/x-bcpio&quot;               },
    { &quot;bin&quot;,        3, &quot;application/octet-stream&quot;          },
    { &quot;c&quot;,          1, &quot;text/plain&quot;                        },
    { &quot;cc&quot;,         2, &quot;text/plain&quot;                        },
    { &quot;ccad&quot;,       4, &quot;application/clariscad&quot;             },
    { &quot;cdf&quot;,        3, &quot;application/x-netcdf&quot;              },
    { &quot;class&quot;,      5, &quot;application/octet-stream&quot;          },
    { &quot;cod&quot;,        3, &quot;application/vnd.rim.cod&quot;           },
    { &quot;com&quot;,        3, &quot;application/x-msdos-program&quot;       },
    { &quot;cpio&quot;,       4, &quot;application/x-cpio&quot;                },
    { &quot;cpt&quot;,        3, &quot;application/mac-compactpro&quot;        },
    { &quot;csh&quot;,        3, &quot;application/x-csh&quot;                 },
    { &quot;css&quot;,        3, &quot;text/css&quot;                          },
    { &quot;dcr&quot;,        3, &quot;application/x-director&quot;            },
    { &quot;deb&quot;,        3, &quot;application/x-debian-package&quot;      },
    { &quot;dir&quot;,        3, &quot;application/x-director&quot;            },
    { &quot;dl&quot;,         2, &quot;video/dl&quot;                          },
    { &quot;dms&quot;,        3, &quot;application/octet-stream&quot;          },
    { &quot;doc&quot;,        3, &quot;application/msword&quot;                },
    { &quot;drw&quot;,        3, &quot;application/drafting&quot;              },
    { &quot;dvi&quot;,        3, &quot;application/x-dvi&quot;                 },
    { &quot;dwg&quot;,        3, &quot;application/acad&quot;                  },
    { &quot;dxf&quot;,        3, &quot;application/dxf&quot;                   },
    { &quot;dxr&quot;,        3, &quot;application/x-director&quot;            },
    { &quot;eps&quot;,        3, &quot;application/postscript&quot;            },
    { &quot;etx&quot;,        3, &quot;text/x-setext&quot;                     },
    { &quot;exe&quot;,        3, &quot;application/octet-stream&quot;          },
    { &quot;ez&quot;,         2, &quot;application/andrew-inset&quot;          },
    { &quot;f&quot;,          1, &quot;text/plain&quot;                        },
    { &quot;f90&quot;,        3, &quot;text/plain&quot;                        },
    { &quot;fli&quot;,        3, &quot;video/fli&quot;                         },
    { &quot;flv&quot;,        3, &quot;video/flv&quot;                         },
    { &quot;gif&quot;,        3, &quot;image/gif&quot;                         },
    { &quot;gl&quot;,         2, &quot;video/gl&quot;                          },
    { &quot;gtar&quot;,       4, &quot;application/x-gtar&quot;                },
    { &quot;gz&quot;,         2, &quot;application/x-gzip&quot;                },
    { &quot;hdf&quot;,        3, &quot;application/x-hdf&quot;                 },
    { &quot;hh&quot;,         2, &quot;text/plain&quot;                        },
    { &quot;hqx&quot;,        3, &quot;application/mac-binhex40&quot;          },
    { &quot;h&quot;,          1, &quot;text/plain&quot;                        },
    { &quot;htm&quot;,        3, &quot;text/html; charset=utf-8&quot;          },
    { &quot;html&quot;,       4, &quot;text/html; charset=utf-8&quot;          },
    { &quot;ice&quot;,        3, &quot;x-conference/x-cooltalk&quot;           },
    { &quot;ief&quot;,        3, &quot;image/ief&quot;                         },
    { &quot;iges&quot;,       4, &quot;model/iges&quot;                        },
    { &quot;igs&quot;,        3, &quot;model/iges&quot;                        },
    { &quot;ips&quot;,        3, &quot;application/x-ipscript&quot;            },
    { &quot;ipx&quot;,        3, &quot;application/x-ipix&quot;                },
    { &quot;jad&quot;,        3, &quot;text/vnd.sun.j2me.app-descriptor&quot;  },
    { &quot;jar&quot;,        3, &quot;application/java-archive&quot;          },
    { &quot;jpeg&quot;,       4, &quot;image/jpeg&quot;                        },
    { &quot;jpe&quot;,        3, &quot;image/jpeg&quot;                        },
    { &quot;jpg&quot;,        3, &quot;image/jpeg&quot;                        },
    { &quot;js&quot;,         2, &quot;application/x-javascript&quot;          },
    { &quot;kar&quot;,        3, &quot;audio/midi&quot;                        },
    { &quot;latex&quot;,      5, &quot;application/x-latex&quot;               },
    { &quot;lha&quot;,        3, &quot;application/octet-stream&quot;          },
    { &quot;lsp&quot;,        3, &quot;application/x-lisp&quot;                },
    { &quot;lzh&quot;,        3, &quot;application/octet-stream&quot;          },
    { &quot;m&quot;,          1, &quot;text/plain&quot;                        },
    { &quot;m3u&quot;,        3, &quot;audio/x-mpegurl&quot;                   },
    { &quot;man&quot;,        3, &quot;application/x-troff-man&quot;           },
    { &quot;me&quot;,         2, &quot;application/x-troff-me&quot;            },
    { &quot;mesh&quot;,       4, &quot;model/mesh&quot;                        },
    { &quot;mid&quot;,        3, &quot;audio/midi&quot;                        },
    { &quot;midi&quot;,       4, &quot;audio/midi&quot;                        },
    { &quot;mif&quot;,        3, &quot;application/x-mif&quot;                 },
    { &quot;mime&quot;,       4, &quot;www/mime&quot;                          },
    { &quot;movie&quot;,      5, &quot;video/x-sgi-movie&quot;                 },
    { &quot;mov&quot;,        3, &quot;video/quicktime&quot;                   },
    { &quot;mp2&quot;,        3, &quot;audio/mpeg&quot;                        },
    { &quot;mp2&quot;,        3, &quot;video/mpeg&quot;                        },
    { &quot;mp3&quot;,        3, &quot;audio/mpeg&quot;                        },
    { &quot;mpeg&quot;,       4, &quot;video/mpeg&quot;                        },
    { &quot;mpe&quot;,        3, &quot;video/mpeg&quot;                        },
    { &quot;mpga&quot;,       4, &quot;audio/mpeg&quot;                        },
    { &quot;mpg&quot;,        3, &quot;video/mpeg&quot;                        },
    { &quot;ms&quot;,         2, &quot;application/x-troff-ms&quot;            },
    { &quot;msh&quot;,        3, &quot;model/mesh&quot;                        },
    { &quot;nc&quot;,         2, &quot;application/x-netcdf&quot;              },
    { &quot;oda&quot;,        3, &quot;application/oda&quot;                   },
    { &quot;ogg&quot;,        3, &quot;application/ogg&quot;                   },
    { &quot;ogm&quot;,        3, &quot;application/ogg&quot;                   },
    { &quot;pbm&quot;,        3, &quot;image/x-portable-bitmap&quot;           },
    { &quot;pdb&quot;,        3, &quot;chemical/x-pdb&quot;                    },
    { &quot;pdf&quot;,        3, &quot;application/pdf&quot;                   },
    { &quot;pgm&quot;,        3, &quot;image/x-portable-graymap&quot;          },
    { &quot;pgn&quot;,        3, &quot;application/x-chess-pgn&quot;           },
    { &quot;pgp&quot;,        3, &quot;application/pgp&quot;                   },
    { &quot;pl&quot;,         2, &quot;application/x-perl&quot;                },
    { &quot;pm&quot;,         2, &quot;application/x-perl&quot;                },
    { &quot;png&quot;,        3, &quot;image/png&quot;                         },
    { &quot;pnm&quot;,        3, &quot;image/x-portable-anymap&quot;           },
    { &quot;pot&quot;,        3, &quot;application/mspowerpoint&quot;          },
    { &quot;ppm&quot;,        3, &quot;image/x-portable-pixmap&quot;           },
    { &quot;pps&quot;,        3, &quot;application/mspowerpoint&quot;          },
    { &quot;ppt&quot;,        3, &quot;application/mspowerpoint&quot;          },
    { &quot;ppz&quot;,        3, &quot;application/mspowerpoint&quot;          },
    { &quot;pre&quot;,        3, &quot;application/x-freelance&quot;           },
    { &quot;prt&quot;,        3, &quot;application/pro_eng&quot;               },
    { &quot;ps&quot;,         2, &quot;application/postscript&quot;            },
    { &quot;qt&quot;,         2, &quot;video/quicktime&quot;                   },
    { &quot;ra&quot;,         2, &quot;audio/x-realaudio&quot;                 },
    { &quot;ram&quot;,        3, &quot;audio/x-pn-realaudio&quot;              },
    { &quot;rar&quot;,        3, &quot;application/x-rar-compressed&quot;      },
    { &quot;ras&quot;,        3, &quot;image/cmu-raster&quot;                  },
    { &quot;ras&quot;,        3, &quot;image/x-cmu-raster&quot;                },
    { &quot;rgb&quot;,        3, &quot;image/x-rgb&quot;                       },
    { &quot;rm&quot;,         2, &quot;audio/x-pn-realaudio&quot;              },
    { &quot;roff&quot;,       4, &quot;application/x-troff&quot;               },
    { &quot;rpm&quot;,        3, &quot;audio/x-pn-realaudio-plugin&quot;       },
    { &quot;rtf&quot;,        3, &quot;application/rtf&quot;                   },
    { &quot;rtf&quot;,        3, &quot;text/rtf&quot;                          },
    { &quot;rtx&quot;,        3, &quot;text/richtext&quot;                     },
    { &quot;scm&quot;,        3, &quot;application/x-lotusscreencam&quot;      },
    { &quot;set&quot;,        3, &quot;application/set&quot;                   },
    { &quot;sgml&quot;,       4, &quot;text/sgml&quot;                         },
    { &quot;sgm&quot;,        3, &quot;text/sgml&quot;                         },
    { &quot;sh&quot;,         2, &quot;application/x-sh&quot;                  },
    { &quot;shar&quot;,       4, &quot;application/x-shar&quot;                },
    { &quot;silo&quot;,       4, &quot;model/mesh&quot;                        },
    { &quot;sit&quot;,        3, &quot;application/x-stuffit&quot;             },
    { &quot;skd&quot;,        3, &quot;application/x-koan&quot;                },
    { &quot;skm&quot;,        3, &quot;application/x-koan&quot;                },
    { &quot;skp&quot;,        3, &quot;application/x-koan&quot;                },
    { &quot;skt&quot;,        3, &quot;application/x-koan&quot;                },
    { &quot;smi&quot;,        3, &quot;application/smil&quot;                  },
    { &quot;smil&quot;,       4, &quot;application/smil&quot;                  },
    { &quot;snd&quot;,        3, &quot;audio/basic&quot;                       },
    { &quot;sol&quot;,        3, &quot;application/solids&quot;                },
    { &quot;spl&quot;,        3, &quot;application/x-futuresplash&quot;        },
    { &quot;src&quot;,        3, &quot;application/x-wais-source&quot;         },
    { &quot;step&quot;,       4, &quot;application/STEP&quot;                  },
    { &quot;stl&quot;,        3, &quot;application/SLA&quot;                   },
    { &quot;stp&quot;,        3, &quot;application/STEP&quot;                  },
    { &quot;sv4cpio&quot;,    7, &quot;application/x-sv4cpio&quot;             },
    { &quot;sv4crc&quot;,     6, &quot;application/x-sv4crc&quot;              },
    { &quot;svg&quot;,        3, &quot;image/svg+xml&quot;                     },
    { &quot;swf&quot;,        3, &quot;application/x-shockwave-flash&quot;     },
    { &quot;t&quot;,          1, &quot;application/x-troff&quot;               },
    { &quot;tar&quot;,        3, &quot;application/x-tar&quot;                 },
    { &quot;tcl&quot;,        3, &quot;application/x-tcl&quot;                 },
    { &quot;tex&quot;,        3, &quot;application/x-tex&quot;                 },
    { &quot;texi&quot;,       4, &quot;application/x-texinfo&quot;             },
    { &quot;texinfo&quot;,    7, &quot;application/x-texinfo&quot;             },
    { &quot;tgz&quot;,        3, &quot;application/x-tar-gz&quot;              },
    { &quot;tiff&quot;,       4, &quot;image/tiff&quot;                        },
    { &quot;tif&quot;,        3, &quot;image/tiff&quot;                        },
    { &quot;tr&quot;,         2, &quot;application/x-troff&quot;               },
    { &quot;tsi&quot;,        3, &quot;audio/TSP-audio&quot;                   },
    { &quot;tsp&quot;,        3, &quot;application/dsptype&quot;               },
    { &quot;tsv&quot;,        3, &quot;text/tab-separated-values&quot;         },
    { &quot;txt&quot;,        3, &quot;text/plain&quot;                        },
    { &quot;unv&quot;,        3, &quot;application/i-deas&quot;                },
    { &quot;ustar&quot;,      5, &quot;application/x-ustar&quot;               },
    { &quot;vcd&quot;,        3, &quot;application/x-cdlink&quot;              },
    { &quot;vda&quot;,        3, &quot;application/vda&quot;                   },
    { &quot;viv&quot;,        3, &quot;video/vnd.vivo&quot;                    },
    { &quot;vivo&quot;,       4, &quot;video/vnd.vivo&quot;                    },
    { &quot;vrml&quot;,       4, &quot;model/vrml&quot;                        },
    { &quot;vsix&quot;,       4, &quot;application/vsix&quot;                  },
    { &quot;wav&quot;,        3, &quot;audio/x-wav&quot;                       },
    { &quot;wax&quot;,        3, &quot;audio/x-ms-wax&quot;                    },
    { &quot;wiki&quot;,       4, &quot;application/x-fossil-wiki&quot;         },
    { &quot;wma&quot;,        3, &quot;audio/x-ms-wma&quot;                    },
    { &quot;wmv&quot;,        3, &quot;video/x-ms-wmv&quot;                    },
    { &quot;wmx&quot;,        3, &quot;video/x-ms-wmx&quot;                    },
    { &quot;wrl&quot;,        3, &quot;model/vrml&quot;                        },
    { &quot;wvx&quot;,        3, &quot;video/x-ms-wvx&quot;                    },
    { &quot;xbm&quot;,        3, &quot;image/x-xbitmap&quot;                   },
    { &quot;xlc&quot;,        3, &quot;application/vnd.ms-excel&quot;          },
    { &quot;xll&quot;,        3, &quot;application/vnd.ms-excel&quot;          },
    { &quot;xlm&quot;,        3, &quot;application/vnd.ms-excel&quot;          },
    { &quot;xls&quot;,        3, &quot;application/vnd.ms-excel&quot;          },
    { &quot;xlw&quot;,        3, &quot;application/vnd.ms-excel&quot;          },
    { &quot;xml&quot;,        3, &quot;text/xml&quot;                          },
    { &quot;xpm&quot;,        3, &quot;image/x-xpixmap&quot;                   },
    { &quot;xwd&quot;,        3, &quot;image/x-xwindowdump&quot;               },
    { &quot;xyz&quot;,        3, &quot;chemical/x-pdb&quot;                    },
    { &quot;zip&quot;,        3, &quot;application/zip&quot;                   },
  };

  for(i=nName-1; i&gt;0 &amp;&amp; zName[i]!=&#39;.&#39;; i--){}
  z = &amp;zName[i+1];
  len = nName - i;
  if( len&lt;(int)sizeof(zSuffix)-1 ){
    strcpy(zSuffix, z);
    for(i=0; zSuffix[i]; i++) zSuffix[i] = tolower(zSuffix[i]);
    first = 0;
    last = sizeof(aMime)/sizeof(aMime[0]);
    while( first&lt;=last ){
      int c;
      i = (first+last)/2;
      c = strcmp(zSuffix, aMime[i].zSuffix);
      if( c==0 ) return aMime[i].zMimetype;
      if( c&lt;0 ){
        last = i-1;
      }else{
        first = i+1;
      }
    }
  }
  return &quot;application/octet-stream&quot;;
}

/*
** The following table contains 1 for all characters that are permitted in
** the part of the URL before the query parameters and fragment.
**
** Allowed characters:  0-9a-zA-Z,-./:_~
**
** Disallowed characters include:  !&quot;#$%&amp;&#39;()*+;&lt;=&gt;?[\]^{|}
*/
static const char allowedInName[] = {
      /*  x0  x1  x2  x3  x4  x5  x6  x7  x8  x9  xa  xb  xc  xd  xe  xf */
/* 0x */   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
/* 1x */   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
/* 2x */   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
/* 3x */   1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  0,
/* 4x */   0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
/* 5x */   1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  1,
/* 6x */   0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
/* 7x */   1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  1,  0,
/* 8x */   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
/* 9x */   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
/* Ax */   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
/* Bx */   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
/* Cx */   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
/* Dx */   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
/* Ex */   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
/* Fx */   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
};

/*
** Remove all disallowed characters in the input string z[].  Convert any
** disallowed characters into &quot;_&quot;.
**
** Not that the three character sequence &quot;%XX&quot; where X is any byte is
** converted into a single &quot;_&quot; character.
**
** Return the number of characters converted.  An &quot;%XX&quot; -&gt; &quot;_&quot; conversion
** counts as a single character.
*/
static int sanitizeString(char *z){
  int nChange = 0;
  while( *z ){
    if( !allowedInName[*(unsigned char*)z] ){
      if( *z==&#39;%&#39; &amp;&amp; z[1]!=0 &amp;&amp; z[2]!=0 ){
        int i;
        for(i=3; (z[i-2] = z[i])!=0; i++){}
      }
      *z = &#39;_&#39;;
      nChange++;
    }
    z++;
  }
  return nChange;
}

/*
** Count the number of &quot;/&quot; characters in a string.
*/
static int countSlashes(const char *z){
  int n = 0;
  while( *z ) if( *(z++)==&#39;/&#39; ) n++;
  return n;
}

/*
** Transfer nXfer bytes from in to out, after first discarding
** nSkip bytes from in.  Increment the nOut global variable
** according to the number of bytes transferred.
*/
static void xferBytes(FILE *in, FILE *out, int nXfer, int nSkip){
  size_t n;
  size_t got;
  char zBuf[16384];
  while( nSkip&gt;0 ){
    n = nSkip;
    if( n&gt;sizeof(zBuf) ) n = sizeof(zBuf);
    got = fread(zBuf, 1, n, in);
    if( got==0 ) break;
    nSkip -= got;
  }
  while( nXfer&gt;0 ){
    n = nXfer;
    if( n&gt;sizeof(zBuf) ) n = sizeof(zBuf);
    got = fread(zBuf, 1, n, in);
    if( got==0 ) break;
    fwrite(zBuf, got, 1, out);
    nOut += got;
    nXfer -= got;
  }
}

/*
** Send the text of the file named by zFile as the reply.  Use the
** suffix on the end of the zFile name to determine the mimetype.
**
** Return 1 to omit making a log entry for the reply.
*/
static int SendFile(
  const char *zFile,      /* Name of the file to send */
  int lenFile,            /* Length of the zFile name in bytes */
  struct stat *pStat      /* Result of a stat() against zFile */
){
  const char *zContentType;
  time_t t;
  FILE *in;
  char zETag[100];

  zContentType = GetMimeType(zFile, lenFile);
  if( zTmpNam ) unlink(zTmpNam);
  sprintf(zETag, &quot;m%xs%x&quot;, (int)pStat-&gt;st_mtime, (int)pStat-&gt;st_size);
  if( CompareEtags(zIfNoneMatch,zETag)==0
   || (zIfModifiedSince!=0
        &amp;&amp; (t = ParseRfc822Date(zIfModifiedSince))&gt;0
        &amp;&amp; t&gt;=pStat-&gt;st_mtime)
  ){
    StartResponse(&quot;304 Not Modified&quot;);
    nOut += DateTag(&quot;Last-Modified&quot;, pStat-&gt;st_mtime);
    nOut += printf(&quot;Cache-Control: max-age=%d\r\n&quot;, mxAge);
    nOut += printf(&quot;ETag: \&quot;%s\&quot;\r\n&quot;, zETag);
    nOut += printf(&quot;\r\n&quot;);
    fflush(stdout);
    MakeLogEntry(0, 470);  /* LOG: ETag Cache Hit */
    return 1;
  }
  in = fopen(zFile,&quot;rb&quot;);
  if( in==0 ) NotFound(480); /* LOG: fopen() failed for static content */
  if( rangeEnd&gt;0 &amp;&amp; rangeStart&lt;pStat-&gt;st_size ){
    StartResponse(&quot;206 Partial Content&quot;);
    if( rangeEnd&gt;=pStat-&gt;st_size ){
      rangeEnd = pStat-&gt;st_size-1;
    }
    nOut += printf(&quot;Content-Range: bytes %d-%d/%d\r\n&quot;,
                    rangeStart, rangeEnd, (int)pStat-&gt;st_size);
    pStat-&gt;st_size = rangeEnd + 1 - rangeStart;
  }else{
    StartResponse(&quot;200 OK&quot;);
    rangeStart = 0;
  }
  nOut += DateTag(&quot;Last-Modified&quot;, pStat-&gt;st_mtime);
  nOut += printf(&quot;Cache-Control: max-age=%d\r\n&quot;, mxAge);
  nOut += printf(&quot;ETag: \&quot;%s\&quot;\r\n&quot;, zETag);
  nOut += printf(&quot;Content-type: %s; charset=utf-8\r\n&quot;,zContentType);
  nOut += printf(&quot;Content-length: %d\r\n\r\n&quot;,(int)pStat-&gt;st_size);
  fflush(stdout);
  if( strcmp(zMethod,&quot;HEAD&quot;)==0 ){
    MakeLogEntry(0, 2); /* LOG: Normal HEAD reply */
    fclose(in);
    fflush(stdout);
    return 1;
  }
  if( useTimeout ) alarm(30 + pStat-&gt;st_size/1000);
#ifdef linux
  {
    off_t offset = rangeStart;
    nOut += sendfile(fileno(stdout), fileno(in), &amp;offset, pStat-&gt;st_size);
  }
#else
  xferBytes(in, stdout, (int)pStat-&gt;st_size, rangeStart);
#endif
  fclose(in);
  return 0;
}

/*
** A CGI or SCGI script has run and is sending its reply back across
** the channel &quot;in&quot;.  Process this reply into an appropriate HTTP reply.
** Close the &quot;in&quot; channel when done.
*/
static void CgiHandleReply(FILE *in){
  int seenContentLength = 0;   /* True if Content-length: header seen */
  int contentLength = 0;       /* The content length */
  size_t nRes = 0;             /* Bytes of payload */
  size_t nMalloc = 0;          /* Bytes of space allocated to aRes */
  char *aRes = 0;              /* Payload */
  int c;                       /* Next character from in */
  char *z;                     /* Pointer to something inside of zLine */
  int iStatus = 0;             /* Reply status code */
  char zLine[1000];            /* One line of reply from the CGI script */

  if( useTimeout ){
    /* Disable the timeout, so that we can implement Hanging-GET or
    ** long-poll style CGIs.  The RLIMIT_CPU will serve as a safety
    ** to help prevent a run-away CGI */
    alarm(0);
  }
  while( fgets(zLine,sizeof(zLine),in) &amp;&amp; !isspace((unsigned char)zLine[0]) ){
    if( strncasecmp(zLine,&quot;Location:&quot;,9)==0 ){
      StartResponse(&quot;302 Redirect&quot;);
      RemoveNewline(zLine);
      z = &amp;zLine[10];
      while( isspace(*(unsigned char*)z) ){ z++; }
      nOut += printf(&quot;Location: %s\r\n&quot;,z);
      rangeEnd = 0;
    }else if( strncasecmp(zLine,&quot;Status:&quot;,7)==0 ){
      int i;
      for(i=7; isspace((unsigned char)zLine[i]); i++){}
      nOut += printf(&quot;%s %s&quot;, zProtocol, &amp;zLine[i]);
      strncpy(zReplyStatus, &amp;zLine[i], 3);
      zReplyStatus[3] = 0;
      iStatus = atoi(zReplyStatus);
      if( iStatus!=200 ) rangeEnd = 0;
      statusSent = 1;
    }else if( strncasecmp(zLine, &quot;Content-length:&quot;, 15)==0 ){
      seenContentLength = 1;
      contentLength = atoi(zLine+15);
    }else{
      size_t nLine = strlen(zLine);
      if( nRes+nLine &gt;= nMalloc ){
        nMalloc += nMalloc + nLine*2;
        aRes = realloc(aRes, nMalloc+1);
        if( aRes==0 ){
          Malfunction(600, &quot;Out of memory: %d bytes&quot;, nMalloc);
        }
      }
      memcpy(aRes+nRes, zLine, nLine);
      nRes += nLine;
    }
  }

  /* Copy everything else thru without change or analysis.
  */
  if( rangeEnd&gt;0 &amp;&amp; seenContentLength &amp;&amp; rangeStart&lt;contentLength ){
    StartResponse(&quot;206 Partial Content&quot;);
    if( rangeEnd&gt;=contentLength ){
      rangeEnd = contentLength-1;
    }
    nOut += printf(&quot;Content-Range: bytes %d-%d/%d\r\n&quot;,
                    rangeStart, rangeEnd, contentLength);
    contentLength = rangeEnd + 1 - rangeStart;
  }else{
    StartResponse(&quot;200 OK&quot;);
  }
  if( nRes&gt;0 ){
    aRes[nRes] = 0;
    printf(&quot;%s&quot;, aRes);
    nOut += nRes;
    nRes = 0;
  }
  if( iStatus==304 ){
    nOut += printf(&quot;\r\n\r\n&quot;);
  }else if( seenContentLength ){
    nOut += printf(&quot;Content-length: %d\r\n\r\n&quot;, contentLength);
    xferBytes(in, stdout, contentLength, rangeStart);
  }else{
    while( (c = getc(in))!=EOF ){
      if( nRes&gt;=nMalloc ){
        nMalloc = nMalloc*2 + 1000;
        aRes = realloc(aRes, nMalloc+1);
        if( aRes==0 ){
           Malfunction(610, &quot;Out of memory: %d bytes&quot;, nMalloc);
        }
      }
      aRes[nRes++] = c;
    }
    if( nRes ){
      aRes[nRes] = 0;
      nOut += printf(&quot;Content-length: %d\r\n\r\n%s&quot;, (int)nRes, aRes);
    }else{
      nOut += printf(&quot;Content-length: 0\r\n\r\n&quot;);
    }
  }
  free(aRes);
  fclose(in);
}

/*
** Send an SCGI request to a host identified by zFile and process the
** reply.
*/
static void SendScgiRequest(const char *zFile, const char *zScript){
  FILE *in;
  FILE *s;
  char *z;
  char *zHost;
  char *zPort = 0;
  char *zRelight = 0;
  char *zFallback = 0;
  int rc;
  int iSocket = -1;
  struct addrinfo hints;
  struct addrinfo *ai = 0;
  struct addrinfo *p;
  char *zHdr;
  size_t nHdr = 0;
  size_t nHdrAlloc;
  int i;
  char zLine[1000];
  char zExtra[1000];
  in = fopen(zFile, &quot;rb&quot;);
  if( in==0 ){
    Malfunction(700, &quot;cannot open \&quot;%s\&quot;\n&quot;, zFile);
  }
  if( fgets(zLine, sizeof(zLine)-1, in)==0 ){
    Malfunction(701, &quot;cannot read \&quot;%s\&quot;\n&quot;, zFile);
  }
  if( strncmp(zLine,&quot;SCGI &quot;,5)!=0 ){
    Malfunction(702, &quot;misformatted SCGI spec \&quot;%s\&quot;\n&quot;, zFile);
  }
  z = zLine+5;
  zHost = GetFirstElement(z,&amp;z);
  zPort = GetFirstElement(z,0);
  if( zHost==0 || zHost[0]==0 || zPort==0 || zPort[0]==0 ){
    Malfunction(703, &quot;misformatted SCGI spec \&quot;%s\&quot;\n&quot;, zFile);
  }
  while( fgets(zExtra, sizeof(zExtra)-1, in) ){
    char *zCmd = GetFirstElement(zExtra,&amp;z);
    if( zCmd==0 ) continue;
    if( zCmd[0]==&#39;#&#39; ) continue;
    RemoveNewline(z);
    if( strcmp(zCmd, &quot;relight:&quot;)==0 ){
      free(zRelight);
      zRelight = StrDup(z);
      continue;
    }
    if( strcmp(zCmd, &quot;fallback:&quot;)==0 ){
      free(zFallback);
      zFallback = StrDup(z);
      continue;
    }
    Malfunction(704, &quot;unrecognized line in SCGI spec: \&quot;%s %s\&quot;\n&quot;,
                zCmd, z ? z : &quot;&quot;);
  }
  fclose(in);
  memset(&amp;hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  rc = getaddrinfo(zHost,zPort,&amp;hints,&amp;ai);
  if( rc ){
    Malfunction(704, &quot;cannot resolve SCGI server name %s:%s\n%s\n&quot;,
                zHost, zPort, gai_strerror(rc));
  }
  while(1){  /* Exit via break */
    for(p=ai; p; p=p-&gt;ai_next){
      iSocket = socket(p-&gt;ai_family, p-&gt;ai_socktype, p-&gt;ai_protocol);
      if( iSocket&lt;0 ) continue;
      if( connect(iSocket,p-&gt;ai_addr,p-&gt;ai_addrlen)&gt;=0 ) break;
      close(iSocket);
    }
    if( iSocket&lt;0 || (s = fdopen(iSocket,&quot;r+&quot;))==0 ){
      if( iSocket&gt;=0 ) close(iSocket);
      if( zRelight ){
        rc = system(zRelight);
        if( rc ){
          Malfunction(721,&quot;Relight failed with %d: \&quot;%s\&quot;\n&quot;,
                      rc, zRelight);
        }
        free(zRelight);
        zRelight = 0;
        sleep(1);
        continue;
      }
      if( zFallback ){
        struct stat statbuf;
        int rc;
        memset(&amp;statbuf, 0, sizeof(statbuf));
        if( chdir(zDir) ){
          char zBuf[1000];
          Malfunction(720, /* LOG: chdir() failed */
               &quot;cannot chdir to [%s] from [%s]&quot;, 
               zDir, getcwd(zBuf,999));
        }
        rc = stat(zFallback, &amp;statbuf);
        if( rc==0 &amp;&amp; S_ISREG(statbuf.st_mode) &amp;&amp; access(zFallback,R_OK)==0 ){
          closeConnection = 1;
          rc = SendFile(zFallback, (int)strlen(zFallback), &amp;statbuf);
          free(zFallback);
          exit(0);
        }else{
          Malfunction(706, &quot;bad fallback file: \&quot;%s\&quot;\n&quot;, zFallback);
        }
      }
      Malfunction(707, &quot;cannot open socket to SCGI server %s\n&quot;,
                  zScript);
    }
    break;
  }

  nHdrAlloc = 0;
  zHdr = 0;
  if( zContentLength==0 ) zContentLength = &quot;0&quot;;
  zScgi = &quot;1&quot;;
  for(i=0; i&lt;(int)(sizeof(cgienv)/sizeof(cgienv[0])); i++){
    int n1, n2;
    if( cgienv[i].pzEnvValue[0]==0 ) continue;
    n1 = (int)strlen(cgienv[i].zEnvName);
    n2 = (int)strlen(*cgienv[i].pzEnvValue);
    if( n1+n2+2+nHdr &gt;= nHdrAlloc ){
      nHdrAlloc = nHdr + n1 + n2 + 1000;
      zHdr = realloc(zHdr, nHdrAlloc);
      if( zHdr==0 ){
        Malfunction(706, &quot;out of memory&quot;);
      }
    }
    memcpy(zHdr+nHdr, cgienv[i].zEnvName, n1);
    nHdr += n1;
    zHdr[nHdr++] = 0;
    memcpy(zHdr+nHdr, *cgienv[i].pzEnvValue, n2);
    nHdr += n2;
    zHdr[nHdr++] = 0;
  }
  zScgi = 0;
  fprintf(s,&quot;%d:&quot;,(int)nHdr);
  fwrite(zHdr, 1, nHdr, s);
  fprintf(s,&quot;,&quot;);
  free(zHdr);
  if( zMethod[0]==&#39;P&#39;
   &amp;&amp; atoi(zContentLength)&gt;0 
   &amp;&amp; (in = fopen(zTmpNam,&quot;r&quot;))!=0 ){
    size_t n;
    while( (n = fread(zLine,1,sizeof(zLine),in))&gt;0 ){
      fwrite(zLine, 1, n, s);
    }
    fclose(in);
  }
  fflush(s);
  CgiHandleReply(s);
}

/*
** This routine processes a single HTTP request on standard input and
** sends the reply to standard output.  If the argument is 1 it means
** that we are should close the socket without processing additional
** HTTP requests after the current request finishes.  0 means we are
** allowed to keep the connection open and to process additional requests.
** This routine may choose to close the connection even if the argument
** is 0.
** 
** If the connection should be closed, this routine calls exit() and
** thus never returns.  If this routine does return it means that another
** HTTP request may appear on the wire.
*/
void ProcessOneRequest(int forceClose){
  int i, j, j0;
  char *z;                  /* Used to parse up a string */
  struct stat statbuf;      /* Information about the file to be retrieved */
  FILE *in;                 /* For reading from CGI scripts */
#ifdef LOG_HEADER
  FILE *hdrLog = 0;         /* Log file for complete header content */
#endif
  char zLine[1000];         /* A buffer for input lines or forming names */

  /* Change directories to the root of the HTTP filesystem
  */
  if( chdir(zRoot[0] ? zRoot : &quot;/&quot;)!=0 ){
    char zBuf[1000];
    Malfunction(190,   /* LOG: chdir() failed */
         &quot;cannot chdir to [%s] from [%s]&quot;,
         zRoot, getcwd(zBuf,999));
  }
  nRequest++;

  /*
  ** We must receive a complete header within 15 seconds
  */
  signal(SIGALRM, Timeout);
  signal(SIGSEGV, Timeout);
  signal(SIGPIPE, Timeout);
  signal(SIGXCPU, Timeout);
  if( useTimeout ) alarm(15);

  /* Get the first line of the request and parse out the
  ** method, the script and the protocol.
  */
  if( fgets(zLine,sizeof(zLine),stdin)==0 ){
    exit(0);
  }
  gettimeofday(&amp;beginTime, 0);
  omitLog = 0;
  nIn += strlen(zLine);

  /* Parse the first line of the HTTP request */
  zMethod = StrDup(GetFirstElement(zLine,&amp;z));
  zRealScript = zScript = StrDup(GetFirstElement(z,&amp;z));
  zProtocol = StrDup(GetFirstElement(z,&amp;z));
  if( zProtocol==0 || strncmp(zProtocol,&quot;HTTP/&quot;,5)!=0 || strlen(zProtocol)!=8 ){
    StartResponse(&quot;400 Bad Request&quot;);
    nOut += printf(
      &quot;Content-type: text/plain; charset=utf-8\r\n&quot;
      &quot;\r\n&quot;
      &quot;This server does not understand the requested protocol\n&quot;
    );
    MakeLogEntry(0, 200); /* LOG: bad protocol in HTTP header */
    exit(0);
  }
  if( zScript[0]!=&#39;/&#39; ) NotFound(210); /* LOG: Empty request URI */
  while( zScript[1]==&#39;/&#39; ){
    zScript++;
    zRealScript++;
  }
  if( forceClose ){
    closeConnection = 1;
  }else if( zProtocol[5]&lt;&#39;1&#39; || zProtocol[7]&lt;&#39;1&#39; ){
    closeConnection = 1;
  }

  /* This very simple server only understands the GET, POST
  ** and HEAD methods
  */
  if( strcmp(zMethod,&quot;GET&quot;)!=0 &amp;&amp; strcmp(zMethod,&quot;POST&quot;)!=0
       &amp;&amp; strcmp(zMethod,&quot;HEAD&quot;)!=0 ){
    StartResponse(&quot;501 Not Implemented&quot;);
    nOut += printf(
      &quot;Content-type: text/plain; charset=utf-8\r\n&quot;
      &quot;\r\n&quot;
      &quot;The %s method is not implemented on this server.\n&quot;,
      zMethod);
    MakeLogEntry(0, 220); /* LOG: Unknown request method */
    exit(0);
  }

  /* If there is a log file (if zLogFile!=0) and if the pathname in
  ** the first line of the http request contains the magic string
  ** &quot;FullHeaderLog&quot; then write the complete header text into the
  ** file %s(zLogFile)-hdr.  Overwrite the file.  This is for protocol
  ** debugging only and is only enabled if althttpd is compiled with
  ** the -DLOG_HEADER=1 option.
  */
#ifdef LOG_HEADER
  if( zLogFile
   &amp;&amp; strstr(zScript,&quot;FullHeaderLog&quot;)!=0
   &amp;&amp; strlen(zLogFile)&lt;sizeof(zLine)-50
  ){
    sprintf(zLine, &quot;%s-hdr&quot;, zLogFile);
    hdrLog = fopen(zLine, &quot;wb&quot;);
  }
#endif


  /* Get all the optional fields that follow the first line.
  */
  zCookie = 0;
  zAuthType = 0;
  zRemoteUser = 0;
  zReferer = 0;
  zIfNoneMatch = 0;
  zIfModifiedSince = 0;
  rangeEnd = 0;
  while( fgets(zLine,sizeof(zLine),stdin) ){
    char *zFieldName;
    char *zVal;

#ifdef LOG_HEADER
    if( hdrLog ) fprintf(hdrLog, &quot;%s&quot;, zLine);
#endif
    nIn += strlen(zLine);
    zFieldName = GetFirstElement(zLine,&amp;zVal);
    if( zFieldName==0 || *zFieldName==0 ) break;
    RemoveNewline(zVal);
    if( strcasecmp(zFieldName,&quot;User-Agent:&quot;)==0 ){
      zAgent = StrDup(zVal);
    }else if( strcasecmp(zFieldName,&quot;Accept:&quot;)==0 ){
      zAccept = StrDup(zVal);
    }else if( strcasecmp(zFieldName,&quot;Accept-Encoding:&quot;)==0 ){
      zAcceptEncoding = StrDup(zVal);
    }else if( strcasecmp(zFieldName,&quot;Content-length:&quot;)==0 ){
      zContentLength = StrDup(zVal);
    }else if( strcasecmp(zFieldName,&quot;Content-type:&quot;)==0 ){
      zContentType = StrDup(zVal);
    }else if( strcasecmp(zFieldName,&quot;Referer:&quot;)==0 ){
      zReferer = StrDup(zVal);
      if( strstr(zVal, &quot;devids.net/&quot;)!=0 ){ zReferer = &quot;devids.net.smut&quot;;
        Forbidden(230); /* LOG: Referrer is devids.net */
      }
    }else if( strcasecmp(zFieldName,&quot;Cookie:&quot;)==0 ){
      zCookie = StrAppend(zCookie,&quot;; &quot;,zVal);
    }else if( strcasecmp(zFieldName,&quot;Connection:&quot;)==0 ){
      if( strcasecmp(zVal,&quot;close&quot;)==0 ){
        closeConnection = 1;
      }else if( !forceClose &amp;&amp; strcasecmp(zVal, &quot;keep-alive&quot;)==0 ){
        closeConnection = 0;
      }
    }else if( strcasecmp(zFieldName,&quot;Host:&quot;)==0 ){
      int inSquare = 0;
      char c;
      if( sanitizeString(zVal) ){
        Forbidden(240);  /* LOG: Illegal content in HOST: parameter */
      }
      zHttpHost = StrDup(zVal);
      zServerPort = zServerName = StrDup(zHttpHost);
      while( zServerPort &amp;&amp; (c = *zServerPort)!=0
              &amp;&amp; (c!=&#39;:&#39; || inSquare) ){
        if( c==&#39;[&#39; ) inSquare = 1;
        if( c==&#39;]&#39; ) inSquare = 0;
        zServerPort++;
      }
      if( zServerPort &amp;&amp; *zServerPort ){
        *zServerPort = 0;
        zServerPort++;
      }
      if( zRealPort ){
        zServerPort = StrDup(zRealPort);
      }
    }else if( strcasecmp(zFieldName,&quot;Authorization:&quot;)==0 ){
      zAuthType = GetFirstElement(StrDup(zVal), &amp;zAuthArg);
    }else if( strcasecmp(zFieldName,&quot;If-None-Match:&quot;)==0 ){
      zIfNoneMatch = StrDup(zVal);
    }else if( strcasecmp(zFieldName,&quot;If-Modified-Since:&quot;)==0 ){
      zIfModifiedSince = StrDup(zVal);
    }else if( strcasecmp(zFieldName,&quot;Range:&quot;)==0
           &amp;&amp; strcmp(zMethod,&quot;GET&quot;)==0 ){
      int x1 = 0, x2 = 0;
      int n = sscanf(zVal, &quot;bytes=%d-%d&quot;, &amp;x1, &amp;x2);
      if( n==2 &amp;&amp; x1&gt;=0 &amp;&amp; x2&gt;=x1 ){
        rangeStart = x1;
        rangeEnd = x2;
      }else if( n==1 &amp;&amp; x1&gt;0 ){
        rangeStart = x1;
        rangeEnd = 0x7fffffff;
      }
    }
  }
#ifdef LOG_HEADER
  if( hdrLog ) fclose(hdrLog);
#endif

  /* Disallow requests from certain clients */
  if( zAgent ){
    const char *azDisallow[] = {
      &quot;Windows 9&quot;,
      &quot;Download Master&quot;,
      &quot;Ezooms/&quot;,
      &quot;HTTrace&quot;,
      &quot;AhrefsBot&quot;,
      &quot;MicroMessenger&quot;,
      &quot;OPPO A33 Build&quot;,
      &quot;SemrushBot&quot;,
      &quot;MegaIndex.ru&quot;,
      &quot;MJ12bot&quot;,
      &quot;Chrome/0.A.B.C&quot;,
      &quot;Neevabot/&quot;,
      &quot;BLEXBot/&quot;,
    };
    size_t ii;
    for(ii=0; ii&lt;sizeof(azDisallow)/sizeof(azDisallow[0]); ii++){
      if( strstr(zAgent,azDisallow[ii])!=0 ){
        Forbidden(250);  /* LOG: Disallowed user agent */
      }
    }
#if 0
    /* Spider attack from 2019-04-24 */
    if( strcmp(zAgent,
            &quot;Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 &quot;
            &quot;(KHTML, like Gecko) Chrome/50.0.2661.102 Safari/537.36&quot;)==0 ){
      Forbidden(251);  /* LOG: Disallowed user agent (20190424) */
    }
#endif
  }
#if 0
  if( zReferer ){
    static const char *azDisallow[] = {
      &quot;skidrowcrack.com&quot;,
      &quot;hoshiyuugi.tistory.com&quot;,
      &quot;skidrowgames.net&quot;,
    };
    int i;
    for(i=0; i&lt;sizeof(azDisallow)/sizeof(azDisallow[0]); i++){
      if( strstr(zReferer, azDisallow[i])!=0 ){
        NotFound(260);  /* LOG: Disallowed referrer */
      }
    }
  }
#endif

  /* Make an extra effort to get a valid server name and port number.
  ** Only Netscape provides this information.  If the browser is
  ** Internet Explorer, then we have to find out the information for
  ** ourselves.
  */
  if( zServerName==0 ){
    zServerName = SafeMalloc( 100 );
    gethostname(zServerName,100);
  }
  if( zServerPort==0 || *zServerPort==0 ){
    zServerPort = DEFAULT_PORT;
  }

  /* Remove the query string from the end of the requested file.
  */
  for(z=zScript; *z &amp;&amp; *z!=&#39;?&#39;; z++){}
  if( *z==&#39;?&#39; ){
    zQuerySuffix = StrDup(z);
    *z = 0;
  }else{
    zQuerySuffix = &quot;&quot;;
  }
  zQueryString = *zQuerySuffix ? &amp;zQuerySuffix[1] : zQuerySuffix;

  /* Create a file to hold the POST query data, if any.  We have to
  ** do it this way.  We can&#39;t just pass the file descriptor down to
  ** the child process because the fgets() function may have already
  ** read part of the POST data into its internal buffer.
  */
  if( zMethod[0]==&#39;P&#39; &amp;&amp; zContentLength!=0 ){
    size_t len = atoi(zContentLength);
    FILE *out;
    char *zBuf;
    int n;

    if( len&gt;MAX_CONTENT_LENGTH ){
      StartResponse(&quot;500 Request too large&quot;);
      nOut += printf(
        &quot;Content-type: text/plain; charset=utf-8\r\n&quot;
        &quot;\r\n&quot;
        &quot;Too much POST data\n&quot;
      );
      MakeLogEntry(0, 270); /* LOG: Request too large */
      exit(0);
    }
    rangeEnd = 0;
    sprintf(zTmpNamBuf, &quot;/tmp/-post-data-XXXXXX&quot;);
    zTmpNam = zTmpNamBuf;
    if( mkstemp(zTmpNam)&lt;0 ){
      Malfunction(280,  /* LOG: mkstemp() failed */
               &quot;Cannot create a temp file in which to store POST data&quot;);
    }
    out = fopen(zTmpNam,&quot;wb&quot;);
    if( out==0 ){
      StartResponse(&quot;500 Cannot create /tmp file&quot;);
      nOut += printf(
        &quot;Content-type: text/plain; charset=utf-8\r\n&quot;
        &quot;\r\n&quot;
        &quot;Could not open \&quot;%s\&quot; for writing\n&quot;, zTmpNam
      );
      MakeLogEntry(0, 290); /* LOG: cannot create temp file for POST */
      exit(0);
    }
    zBuf = SafeMalloc( len+1 );
    if( useTimeout ) alarm(15 + len/2000);
    n = fread(zBuf,1,len,stdin);
    nIn += n;
    fwrite(zBuf,1,n,out);
    free(zBuf);
    fclose(out);
  }

  /* Make sure the running time is not too great */
  if( useTimeout ) alarm(10);

  /* Convert all unusual characters in the script name into &quot;_&quot;.
  **
  ** This is a defense against various attacks, XSS attacks in particular.
  */
  sanitizeString(zScript);

  /* Do not allow &quot;/.&quot; or &quot;/-&quot; to to occur anywhere in the entity name.
  ** This prevents attacks involving &quot;..&quot; and also allows us to create
  ** files and directories whose names begin with &quot;-&quot; or &quot;.&quot; which are
  ** invisible to the webserver.
  **
  ** Exception:  Allow the &quot;/.well-known/&quot; prefix in accordance with
  ** RFC-5785.
  */
  for(z=zScript; *z; z++){
    if( *z==&#39;/&#39; &amp;&amp; (z[1]==&#39;.&#39; || z[1]==&#39;-&#39;) ){
      if( strncmp(zScript,&quot;/.well-known/&quot;,13)==0 &amp;&amp; (z[1]!=&#39;.&#39; || z[2]!=&#39;.&#39;) ){
        /* Exception:  Allow &quot;/.&quot; and &quot;/-&quot; for URLs that being with
        ** &quot;/.well-known/&quot;.  But do not allow &quot;/..&quot;. */
        continue;
      }
      NotFound(300); /* LOG: Path element begins with &quot;.&quot; or &quot;-&quot; */
    }
  }

  /* Figure out what the root of the filesystem should be.  If the
  ** HTTP_HOST parameter exists (stored in zHttpHost) then remove the
  ** port number from the end (if any), convert all characters to lower
  ** case, and convert non-alphanumber characters (including &quot;.&quot;) to &quot;_&quot;.
  ** Then try to find a directory with that name and the extension .website.
  ** If not found, look for &quot;default.website&quot;.
  */
  if( zScript[0]!=&#39;/&#39; ){
    NotFound(310); /* LOG: URI does not start with &quot;/&quot; */
  }
  if( strlen(zRoot)+40 &gt;= sizeof(zLine) ){
    NotFound(320); /* LOG: URI too long */
  }
  if( zHttpHost==0 || zHttpHost[0]==0 ){
    NotFound(330);  /* LOG: Missing HOST: parameter */
  }else if( strlen(zHttpHost)+strlen(zRoot)+10 &gt;= sizeof(zLine) ){
    NotFound(340);  /* LOG: HOST parameter too long */
  }else{
    sprintf(zLine, &quot;%s/%s&quot;, zRoot, zHttpHost);
    for(i=strlen(zRoot)+1; zLine[i] &amp;&amp; zLine[i]!=&#39;:&#39;; i++){
      unsigned char c = (unsigned char)zLine[i];
      if( !isalnum(c) ){
        if( c==&#39;.&#39; &amp;&amp; (zLine[i+1]==0 || zLine[i+1]==&#39;:&#39;) ){
          /* If the client sent a FQDN with a &quot;.&quot; at the end
          ** (example: &quot;sqlite.org.&quot; instead of just &quot;sqlite.org&quot;) then
          ** omit the final &quot;.&quot; from the document root directory name */
          break;
        }
        zLine[i] = &#39;_&#39;;
      }else if( isupper(c) ){
        zLine[i] = tolower(c);
      }
    }
    strcpy(&amp;zLine[i], &quot;.website&quot;);
  }
  if( stat(zLine,&amp;statbuf) || !S_ISDIR(statbuf.st_mode) ){
    sprintf(zLine, &quot;%s/default.website&quot;, zRoot);
    if( stat(zLine,&amp;statbuf) || !S_ISDIR(statbuf.st_mode) ){
      if( standalone ){
        sprintf(zLine, &quot;%s&quot;, zRoot);
      }else{
        NotFound(350);  /* LOG: *.website permissions */
      }
    }
  }
  zHome = StrDup(zLine);

  /* Change directories to the root of the HTTP filesystem
  */
  if( chdir(zHome)!=0 ){
    char zBuf[1000];
    Malfunction(360,  /* LOG: chdir() failed */
         &quot;cannot chdir to [%s] from [%s]&quot;,
         zHome, getcwd(zBuf,999));
  }

  /* Locate the file in the filesystem.  We might have to append
  ** a name like &quot;/home&quot; or &quot;/index.html&quot; or &quot;/index.cgi&quot; in order
  ** to find it.  Any excess path information is put into the
  ** zPathInfo variable.
  */
  j = j0 = (int)strlen(zLine);
  i = 0;
  while( zScript[i] ){
    while( zScript[i] &amp;&amp; (i==0 || zScript[i]!=&#39;/&#39;) ){
      zLine[j] = zScript[i];
      i++; j++;
    }
    zLine[j] = 0;
    if( stat(zLine,&amp;statbuf)!=0 ){
      int stillSearching = 1;
      while( stillSearching &amp;&amp; i&gt;0 &amp;&amp; j&gt;j0 ){
        while( j&gt;j0 &amp;&amp; zLine[j-1]!=&#39;/&#39; ){ j--; }
        strcpy(&amp;zLine[j-1], &quot;/not-found.html&quot;);
        if( stat(zLine,&amp;statbuf)==0 &amp;&amp; S_ISREG(statbuf.st_mode)
            &amp;&amp; access(zLine,R_OK)==0 ){
          zRealScript = StrDup(&amp;zLine[j0]);
          Redirect(zRealScript, 302, 1, 370); /* LOG: redirect to not-found */
          return;
        }else{
          j--;
        }
      }
      if( stillSearching ) NotFound(380); /* LOG: URI not found */
      break;
    }
    if( S_ISREG(statbuf.st_mode) ){
      if( access(zLine,R_OK) ){
        NotFound(390);  /* LOG: File not readable */
      }
      zRealScript = StrDup(&amp;zLine[j0]);
      break;
    }
    if( zScript[i]==0 || zScript[i+1]==0 ){
      static const char *azIndex[] = { &quot;/home&quot;, &quot;/index.html&quot;, &quot;/index.cgi&quot; };
      int k = j&gt;0 &amp;&amp; zLine[j-1]==&#39;/&#39; ? j-1 : j;
      unsigned int jj;
      for(jj=0; jj&lt;sizeof(azIndex)/sizeof(azIndex[0]); jj++){
        strcpy(&amp;zLine[k],azIndex[jj]);
        if( stat(zLine,&amp;statbuf)!=0 ) continue;
        if( !S_ISREG(statbuf.st_mode) ) continue;
        if( access(zLine,R_OK) ) continue;
        break;
      }
      if( jj&gt;=sizeof(azIndex)/sizeof(azIndex[0]) ){
        NotFound(400); /* LOG: URI is a directory w/o index.html */
      }
      zRealScript = StrDup(&amp;zLine[j0]);
      if( zScript[i]==0 ){
        /* If the requested URL does not end with &quot;/&quot; but we had to
        ** append &quot;index.html&quot;, then a redirect is necessary.  Otherwise
        ** none of the relative URLs in the delivered document will be
        ** correct. */
        Redirect(zRealScript,301,1,410); /* LOG: redirect to add trailing / */
        return;
      }
      break;
    }
    zLine[j] = zScript[i];
    i++; j++;
  }
  zFile = StrDup(zLine);
  zPathInfo = StrDup(&amp;zScript[i]);
  lenFile = strlen(zFile);
  zDir = StrDup(zFile);
  for(i=strlen(zDir)-1; i&gt;0 &amp;&amp; zDir[i]!=&#39;/&#39;; i--){};
  if( i==0 ){
     strcpy(zDir,&quot;/&quot;);
  }else{
     zDir[i] = 0;
  }

  /* Check to see if there is an authorization file.  If there is,
  ** process it.
  */
  sprintf(zLine, &quot;%s/-auth&quot;, zDir);
  if( access(zLine,R_OK)==0 &amp;&amp; !CheckBasicAuthorization(zLine) ) return;

  /* Take appropriate action
  */
  if( (statbuf.st_mode &amp; 0100)==0100 &amp;&amp; access(zFile,X_OK)==0 ){
    char *zBaseFilename;         /* Filename without directory prefix */

    /*
    ** Abort with an error if the CGI script is writable by anyone other
    ** than its owner.
    */
    if( statbuf.st_mode &amp; 0022 ){
      CgiScriptWritable();
    }

    /* If its executable, it must be a CGI program.  Start by
    ** changing directories to the directory holding the program.
    */
    if( chdir(zDir) ){
      char zBuf[1000];
      Malfunction(420, /* LOG: chdir() failed */
           &quot;cannot chdir to [%s] from [%s]&quot;, 
           zDir, getcwd(zBuf,999));
    }

    /* Compute the base filename of the CGI script */
    for(i=strlen(zFile)-1; i&gt;=0 &amp;&amp; zFile[i]!=&#39;/&#39;; i--){}
    zBaseFilename = &amp;zFile[i+1];

    /* Setup the environment appropriately.
    */
    putenv(&quot;GATEWAY_INTERFACE=CGI/1.0&quot;);
    for(i=0; i&lt;(int)(sizeof(cgienv)/sizeof(cgienv[0])); i++){
      if( *cgienv[i].pzEnvValue ){
        SetEnv(cgienv[i].zEnvName,*cgienv[i].pzEnvValue);
      }
    }
    if( useHttps ){
      putenv(&quot;HTTPS=on&quot;);
      putenv(&quot;REQUEST_SCHEME=https&quot;);
    }else{
      putenv(&quot;REQUEST_SCHEME=http&quot;);
    }

    /* For the POST method all input has been written to a temporary file,
    ** so we have to redirect input to the CGI script from that file.
    */
    if( zMethod[0]==&#39;P&#39; ){
      if( dup(0)&lt;0 ){
        Malfunction(430,  /* LOG: dup(0) failed */
                    &quot;Unable to duplication file descriptor 0&quot;);
      }
      close(0);
      open(zTmpNam, O_RDONLY);
    }

    if( strncmp(zBaseFilename,&quot;nph-&quot;,4)==0 ){
      /* If the name of the CGI script begins with &quot;nph-&quot; then we are
      ** dealing with a &quot;non-parsed headers&quot; CGI script.  Just exec()
      ** it directly and let it handle all its own header generation.
      */
      execl(zBaseFilename,zBaseFilename,(char*)0);
      /* NOTE: No log entry written for nph- scripts */
      exit(0);
    }

    /* Fall thru to here only if this process (the server) is going
    ** to read and augment the header sent back by the CGI process.
    ** Open a pipe to receive the output from the CGI process.  Then
    ** fork the CGI process.  Once everything is done, we should be
    ** able to read the output of CGI on the &quot;in&quot; stream.
    */
    {
      int px[2];
      if( pipe(px) ){
        Malfunction(440, /* LOG: pipe() failed */
                    &quot;Unable to create a pipe for the CGI program&quot;);
      }
      if( fork()==0 ){
        close(px[0]);
        close(1);
        if( dup(px[1])!=1 ){
          Malfunction(450, /* LOG: dup(1) failed */
                 &quot;Unable to duplicate file descriptor %d to 1&quot;,
                 px[1]);
        }
        close(px[1]);
        for(i=3; close(i)==0; i++){}
        execl(zBaseFilename, zBaseFilename, (char*)0);
        exit(0);
      }
      close(px[1]);
      in = fdopen(px[0], &quot;rb&quot;);
    }
    if( in==0 ){
      CgiError();
    }else{
      CgiHandleReply(in);
    }
  }else if( lenFile&gt;5 &amp;&amp; strcmp(&amp;zFile[lenFile-5],&quot;.scgi&quot;)==0 ){
    /* Any file that ends with &quot;.scgi&quot; is assumed to be text of the
    ** form:
    **     SCGI hostname port
    ** Open a TCP/IP connection to that host and send it an SCGI request
    */
    SendScgiRequest(zFile, zScript);
  }else if( countSlashes(zRealScript)!=countSlashes(zScript) ){
    /* If the request URI for static content contains material past the
    ** actual content file name, report that as a 404 error. */
    NotFound(460); /* LOG: Excess URI content past static file name */
  }else{
    /* If it isn&#39;t executable then it
    ** must a simple file that needs to be copied to output.
    */
    if( SendFile(zFile, lenFile, &amp;statbuf) ) return;
  }
  fflush(stdout);
  MakeLogEntry(0, 0);  /* LOG: Normal reply */

  /* The next request must arrive within 30 seconds or we close the connection
  */
  omitLog = 1;
  if( useTimeout ) alarm(30);
}

#define MAX_PARALLEL 50  /* Number of simultaneous children */

/*
** All possible forms of an IP address.  Needed to work around GCC strict
** aliasing rules.
*/
typedef union {
  struct sockaddr sa;              /* Abstract superclass */
  struct sockaddr_in sa4;          /* IPv4 */
  struct sockaddr_in6 sa6;         /* IPv6 */
  struct sockaddr_storage sas;     /* Should be the maximum of the above 3 */
} address;

/*
** Implement an HTTP server daemon listening on port zPort.
**
** As new connections arrive, fork a child and let the child return
** out of this procedure call.  The child will handle the request.
** The parent never returns from this procedure.
**
** Return 0 to each child as it runs.  If unable to establish a
** listening socket, return non-zero.
*/
int http_server(const char *zPort, int localOnly){
  int listener[20];            /* The server sockets */
  int connection;              /* A socket for each individual connection */
  fd_set readfds;              /* Set of file descriptors for select() */
  address inaddr;              /* Remote address */
  socklen_t lenaddr;           /* Length of the inaddr structure */
  int child;                   /* PID of the child process */
  int nchildren = 0;           /* Number of child processes */
  struct timeval delay;        /* How long to wait inside select() */
  int opt = 1;                 /* setsockopt flag */
  struct addrinfo sHints;      /* Address hints */
  struct addrinfo *pAddrs, *p; /* */
  int rc;                      /* Result code */
  int i, n;
  int maxFd = -1;
  
  memset(&amp;sHints, 0, sizeof(sHints));
  if( ipv4Only ){
    sHints.ai_family = PF_INET;
    /*printf(&quot;ipv4 only\n&quot;);*/
  }else if( ipv6Only ){
    sHints.ai_family = PF_INET6;
    /*printf(&quot;ipv6 only\n&quot;);*/
  }else{
    sHints.ai_family = PF_UNSPEC;
  }
  sHints.ai_socktype = SOCK_STREAM;
  sHints.ai_flags = AI_PASSIVE;
  sHints.ai_protocol = 0;
  rc = getaddrinfo(localOnly ? &quot;localhost&quot;: 0, zPort, &amp;sHints, &amp;pAddrs);
  if( rc ){
    fprintf(stderr, &quot;could not get addr info: %s&quot;, 
            rc!=EAI_SYSTEM ? gai_strerror(rc) : strerror(errno));
    return 1;
  }
  for(n=0, p=pAddrs; n&lt;(int)(sizeof(listener)/sizeof(listener[0])) &amp;&amp; p!=0;
        p=p-&gt;ai_next){
    listener[n] = socket(p-&gt;ai_family, p-&gt;ai_socktype, p-&gt;ai_protocol);
    if( listener[n]&gt;=0 ){
      /* if we can&#39;t terminate nicely, at least allow the socket to be reused */
      setsockopt(listener[n], SOL_SOCKET, SO_REUSEADDR,&amp;opt, sizeof(opt));
      
#if defined(IPV6_V6ONLY)
      if( p-&gt;ai_family==AF_INET6 ){
        int v6only = 1;
        setsockopt(listener[n], IPPROTO_IPV6, IPV6_V6ONLY,
                    &amp;v6only, sizeof(v6only));
      }
#endif
      
      if( bind(listener[n], p-&gt;ai_addr, p-&gt;ai_addrlen)&lt;0 ){
        printf(&quot;bind failed: %s\n&quot;, strerror(errno));
        close(listener[n]);
        continue;
      }
      if( listen(listener[n], 20)&lt;0 ){
        printf(&quot;listen() failed: %s\n&quot;, strerror(errno));
        close(listener[n]);
        continue;
      }
      n++;
    }
  }
  if( n==0 ){
    fprintf(stderr, &quot;cannot open any sockets\n&quot;);
    return 1;
  }

  while( 1 ){
    if( nchildren&gt;MAX_PARALLEL ){
      /* Slow down if connections are arriving too fast */
      sleep( nchildren-MAX_PARALLEL );
    }
    delay.tv_sec = 60;
    delay.tv_usec = 0;
    FD_ZERO(&amp;readfds);
    for(i=0; i&lt;n; i++){
      assert( listener[i]&gt;=0 );
      FD_SET( listener[i], &amp;readfds);
      if( listener[i]&gt;maxFd ) maxFd = listener[i];
    }
    select( maxFd+1, &amp;readfds, 0, 0, &amp;delay);
    for(i=0; i&lt;n; i++){
      if( FD_ISSET(listener[i], &amp;readfds) ){
        lenaddr = sizeof(inaddr);
        connection = accept(listener[i], &amp;inaddr.sa, &amp;lenaddr);
        if( connection&gt;=0 ){
          child = fork();
          if( child!=0 ){
            if( child&gt;0 ) nchildren++;
            close(connection);
            /* printf(&quot;subprocess %d started...\n&quot;, child); fflush(stdout); */
          }else{
            int nErr = 0, fd;
            close(0);
            fd = dup(connection);
            if( fd!=0 ) nErr++;
            close(1);
            fd = dup(connection);
            if( fd!=1 ) nErr++;
            close(connection);
            return nErr;
          }
        }
      }
      /* Bury dead children */
      while( (child = waitpid(0, 0, WNOHANG))&gt;0 ){
        /* printf(&quot;process %d ends\n&quot;, child); fflush(stdout); */
        nchildren--;
      }
    }
  }
  /* NOT REACHED */  
  exit(1);
}


int main(int argc, char **argv){
  int i;                    /* Loop counter */
  char *zPermUser = 0;      /* Run daemon with this user&#39;s permissions */
  const char *zPort = 0;    /* Implement an HTTP server process */
  int useChrootJail = 1;    /* True to use a change-root jail */
  struct passwd *pwd = 0;   /* Information about the user */

  /* Record the time when processing begins.
  */
  gettimeofday(&amp;beginTime, 0);

  /* Parse command-line arguments
  */
  while( argc&gt;1 &amp;&amp; argv[1][0]==&#39;-&#39; ){
    char *z = argv[1];
    char *zArg = argc&gt;=3 ? argv[2] : &quot;0&quot;;
    if( z[0]==&#39;-&#39; &amp;&amp; z[1]==&#39;-&#39; ) z++;
    if( strcmp(z,&quot;-user&quot;)==0 ){
      zPermUser = zArg;
    }else if( strcmp(z,&quot;-root&quot;)==0 ){
      zRoot = zArg;
    }else if( strcmp(z,&quot;-logfile&quot;)==0 ){
      zLogFile = zArg;
    }else if( strcmp(z,&quot;-max-age&quot;)==0 ){
      mxAge = atoi(zArg);
    }else if( strcmp(z,&quot;-max-cpu&quot;)==0 ){
      maxCpu = atoi(zArg);
    }else if( strcmp(z,&quot;-https&quot;)==0 ){
      useHttps = atoi(zArg);
      zHttp = useHttps ? &quot;https&quot; : &quot;http&quot;;
      if( useHttps ) zRemoteAddr = getenv(&quot;REMOTE_HOST&quot;);
    }else if( strcmp(z, &quot;-port&quot;)==0 ){
      zPort = zArg;
      standalone = 1;
     
    }else if( strcmp(z, &quot;-family&quot;)==0 ){
      if( strcmp(zArg, &quot;ipv4&quot;)==0 ){
        ipv4Only = 1;
      }else if( strcmp(zArg, &quot;ipv6&quot;)==0 ){
        ipv6Only = 1;
      }else{
        Malfunction(500,  /* LOG: unknown IP protocol */
                    &quot;unknown IP protocol: [%s]\n&quot;, zArg);
      }
    }else if( strcmp(z, &quot;-jail&quot;)==0 ){
      if( atoi(zArg)==0 ){
        useChrootJail = 0;
      }
    }else if( strcmp(z, &quot;-debug&quot;)==0 ){
      if( atoi(zArg) ){
        useTimeout = 0;
      }
    }else if( strcmp(z, &quot;-input&quot;)==0 ){
      if( freopen(zArg, &quot;rb&quot;, stdin)==0 || stdin==0 ){
        Malfunction(501, /* LOG: cannot open --input file */
                    &quot;cannot open --input file \&quot;%s\&quot;\n&quot;, zArg);
      }
    }else if( strcmp(z, &quot;-datetest&quot;)==0 ){
      TestParseRfc822Date();
      printf(&quot;Ok\n&quot;);
      exit(0);
    }else{
      Malfunction(510, /* LOG: unknown command-line argument on launch */
                  &quot;unknown argument: [%s]\n&quot;, z);
    }
    argv += 2;
    argc -= 2;
  }
  if( zRoot==0 ){
    if( standalone ){
      zRoot = &quot;.&quot;;
    }else{
      Malfunction(520, /* LOG: --root argument missing */
                  &quot;no --root specified&quot;);
    }
  }
  
  /* Change directories to the root of the HTTP filesystem.  Then
  ** create a chroot jail there.
  */
  if( chdir(zRoot)!=0 ){
    Malfunction(530, /* LOG: chdir() failed */
                &quot;cannot change to directory [%s]&quot;, zRoot);
  }

  /* Get information about the user if available */
  if( zPermUser ) pwd = getpwnam(zPermUser);

  /* Enter the chroot jail if requested */  
  if( zPermUser &amp;&amp; useChrootJail &amp;&amp; getuid()==0 ){
    if( chroot(&quot;.&quot;)&lt;0 ){
      Malfunction(540, /* LOG: chroot() failed */
                  &quot;unable to create chroot jail&quot;);
    }else{
      zRoot = &quot;&quot;;
    }
  }

  /* Activate the server, if requested */
  if( zPort &amp;&amp; http_server(zPort, 0) ){
    Malfunction(550, /* LOG: server startup failed */
                &quot;failed to start server&quot;);
  }

#ifdef RLIMIT_CPU
  if( maxCpu&gt;0 ){
    struct rlimit rlim;
    rlim.rlim_cur = maxCpu;
    rlim.rlim_max = maxCpu;
    setrlimit(RLIMIT_CPU, &amp;rlim);
  }
#endif

  /* Drop root privileges.
  */
  if( zPermUser ){
    if( pwd ){
      if( setgid(pwd-&gt;pw_gid) ){
        Malfunction(560, /* LOG: setgid() failed */
                    &quot;cannot set group-id to %d&quot;, pwd-&gt;pw_gid);
      }
      if( setuid(pwd-&gt;pw_uid) ){
        Malfunction(570, /* LOG: setuid() failed */
                    &quot;cannot set user-id to %d&quot;, pwd-&gt;pw_uid);
      }
    }else{
      Malfunction(580, /* LOG: unknown user */
                  &quot;no such user [%s]&quot;, zPermUser);
    }
  }
  if( getuid()==0 ){
    Malfunction(590, /* LOG: cannot run as root */
                &quot;cannot run as root&quot;);
  }

  /* Get the IP address from whence the request originates
  */
  if( zRemoteAddr==0 ){
    address remoteAddr;
    unsigned int size = sizeof(remoteAddr);
    char zHost[NI_MAXHOST];
    if( getpeername(0, &amp;remoteAddr.sa, &amp;size)&gt;=0 ){
      getnameinfo(&amp;remoteAddr.sa, size, zHost, sizeof(zHost), 0, 0,
                  NI_NUMERICHOST);
      zRemoteAddr = StrDup(zHost);
    }
  }
  if( zRemoteAddr!=0
   &amp;&amp; strncmp(zRemoteAddr, &quot;::ffff:&quot;, 7)==0
   &amp;&amp; strchr(zRemoteAddr+7, &#39;:&#39;)==0
   &amp;&amp; strchr(zRemoteAddr+7, &#39;.&#39;)!=0
  ){
    zRemoteAddr += 7;
  }

  /* Process the input stream */
  for(i=0; i&lt;100; i++){
    ProcessOneRequest(0);
  }
  ProcessOneRequest(1);
  exit(0);
}

#if 0
/* Copy/paste the following text into SQLite to generate the xref
** table that describes all error codes.
*/
BEGIN;
CREATE TABLE IF NOT EXISTS xref(lineno INTEGER PRIMARY KEY, desc TEXT);
DELETE FROM Xref;
INSERT INTO xref VALUES(100,&#39;Malloc() failed&#39;);
INSERT INTO xref VALUES(110,&#39;Not authorized&#39;);
INSERT INTO xref VALUES(120,&#39;CGI Error&#39;);
INSERT INTO xref VALUES(130,&#39;Timeout&#39;);
INSERT INTO xref VALUES(140,&#39;CGI script is writable&#39;);
INSERT INTO xref VALUES(150,&#39;Cannot open -auth file&#39;);
INSERT INTO xref VALUES(160,&#39;http request on https-only page&#39;);
INSERT INTO xref VALUES(170,&#39;-auth redirect&#39;);
INSERT INTO xref VALUES(180,&#39;malformed entry in -auth file&#39;);
INSERT INTO xref VALUES(190,&#39;chdir() failed&#39;);
INSERT INTO xref VALUES(200,&#39;bad protocol in HTTP header&#39;);
INSERT INTO xref VALUES(210,&#39;Empty request URI&#39;);
INSERT INTO xref VALUES(220,&#39;Unknown request method&#39;);
INSERT INTO xref VALUES(230,&#39;Referrer is devids.net&#39;);
INSERT INTO xref VALUES(240,&#39;Illegal content in HOST: parameter&#39;);
INSERT INTO xref VALUES(250,&#39;Disallowed user agent&#39;);
INSERT INTO xref VALUES(260,&#39;Disallowed referrer&#39;);
INSERT INTO xref VALUES(270,&#39;Request too large&#39;);
INSERT INTO xref VALUES(280,&#39;mkstemp() failed&#39;);
INSERT INTO xref VALUES(290,&#39;cannot create temp file for POST content&#39;);
INSERT INTO xref VALUES(300,&#39;Path element begins with . or -&#39;);
INSERT INTO xref VALUES(310,&#39;URI does not start with /&#39;);
INSERT INTO xref VALUES(320,&#39;URI too long&#39;);
INSERT INTO xref VALUES(330,&#39;Missing HOST: parameter&#39;);
INSERT INTO xref VALUES(340,&#39;HOST parameter too long&#39;);
INSERT INTO xref VALUES(350,&#39;*.website permissions&#39;);
INSERT INTO xref VALUES(360,&#39;chdir() failed&#39;);
INSERT INTO xref VALUES(370,&#39;redirect to not-found page&#39;);
INSERT INTO xref VALUES(380,&#39;URI not found&#39;);
INSERT INTO xref VALUES(390,&#39;File not readable&#39;);
INSERT INTO xref VALUES(400,&#39;URI is a directory w/o index.html&#39;);
INSERT INTO xref VALUES(410,&#39;redirect to add trailing /&#39;);
INSERT INTO xref VALUES(420,&#39;chdir() failed&#39;);
INSERT INTO xref VALUES(430,&#39;dup(0) failed&#39;);
INSERT INTO xref VALUES(440,&#39;pipe() failed&#39;);
INSERT INTO xref VALUES(450,&#39;dup(1) failed&#39;);
INSERT INTO xref VALUES(460,&#39;Excess URI content past static file name&#39;);
INSERT INTO xref VALUES(470,&#39;ETag Cache Hit&#39;);
INSERT INTO xref VALUES(480,&#39;fopen() failed for static content&#39;);
INSERT INTO xref VALUES(2,&#39;Normal HEAD reply&#39;);
INSERT INTO xref VALUES(0,&#39;Normal reply&#39;);
INSERT INTO xref VALUES(500,&#39;unknown IP protocol&#39;);
INSERT INTO xref VALUES(501,&#39;cannot open --input file&#39;);
INSERT INTO xref VALUES(510,&#39;unknown command-line argument on launch&#39;);
INSERT INTO xref VALUES(520,&#39;--root argument missing&#39;);
INSERT INTO xref VALUES(530,&#39;chdir() failed&#39;);
INSERT INTO xref VALUES(540,&#39;chroot() failed&#39;);
INSERT INTO xref VALUES(550,&#39;server startup failed&#39;);
INSERT INTO xref VALUES(560,&#39;setgid() failed&#39;);
INSERT INTO xref VALUES(570,&#39;setuid() failed&#39;);
INSERT INTO xref VALUES(580,&#39;unknown user&#39;);
INSERT INTO xref VALUES(590,&#39;cannot run as root&#39;);
INSERT INTO xref VALUES(600,&#39;malloc() failed&#39;);
INSERT INTO xref VALUES(610,&#39;malloc() failed&#39;);
COMMIT;
#endif /* SQL */

</pre>
</blockquote>
</div>
<div class="footer">
This page was generated in about
0.007s by
Fossil 2.16 [255a28b37a] 2021-06-24 16:40:48
</div>
<script nonce="1f98b6c0f84b06e6eb04cd0e4b1318f5124cd24ebaab43aa">/* style.c:858 */
function debugMsg(msg){
var n = document.getElementById("debugMsg");
if(n){n.textContent=msg;}
}
</script>
<script nonce='1f98b6c0f84b06e6eb04cd0e4b1318f5124cd24ebaab43aa'>
/* hbmenu.js *************************************************************/
(function() {
var hbButton = document.getElementById("hbbtn");
if (!hbButton) return;
if (!document.addEventListener) return;
var panel = document.getElementById("hbdrop");
if (!panel) return;
if (!panel.style) return;
var panelBorder = panel.style.border;
var panelInitialized = false;
var panelResetBorderTimerID = 0;
var animate = panel.style.transition !== null && (typeof(panel.style.transition) == "string");
var animMS = panel.getAttribute("data-anim-ms");
if (animMS) {
animMS = parseInt(animMS);
if (isNaN(animMS) || animMS == 0)
animate = false;
else if (animMS < 0)
animMS = 400;
}
else
animMS = 400;
var panelHeight;
function calculatePanelHeight() {
panel.style.maxHeight = '';
var es   = window.getComputedStyle(panel),
edis = es.display,
epos = es.position,
evis = es.visibility;
panel.style.visibility = 'hidden';
panel.style.position   = 'absolute';
panel.style.display    = 'block';
panelHeight = panel.offsetHeight + 'px';
panel.style.display    = edis;
panel.style.position   = epos;
panel.style.visibility = evis;
}
function showPanel() {
if (panelResetBorderTimerID) {
clearTimeout(panelResetBorderTimerID);
panelResetBorderTimerID = 0;
}
if (animate) {
if (!panelInitialized) {
panelInitialized = true;
calculatePanelHeight();
panel.style.transition = 'max-height ' + animMS +
'ms ease-in-out';
panel.style.overflowY  = 'hidden';
panel.style.maxHeight  = '0';
}
setTimeout(function() {
panel.style.maxHeight = panelHeight;
panel.style.border    = panelBorder;
}, 40);
}
panel.style.display = 'block';
document.addEventListener('keydown',panelKeydown,true);
document.addEventListener('click',panelClick,false);
}
var panelKeydown = function(event) {
var key = event.which || event.keyCode;
if (key == 27) {
event.stopPropagation();
panelToggle(true);
}
};
var panelClick = function(event) {
if (!panel.contains(event.target)) {
panelToggle(true);
}
};
function panelShowing() {
if (animate) {
return panel.style.maxHeight == panelHeight;
}
else {
return panel.style.display == 'block';
}
}
function hasChildren(element) {
var childElement = element.firstChild;
while (childElement) {
if (childElement.nodeType == 1)
return true;
childElement = childElement.nextSibling;
}
return false;
}
window.addEventListener('resize',function(event) {
panelInitialized = false;
},false);
hbButton.addEventListener('click',function(event) {
event.stopPropagation();
event.preventDefault();
panelToggle(false);
},false);
function panelToggle(suppressAnimation) {
if (panelShowing()) {
document.removeEventListener('keydown',panelKeydown,true);
document.removeEventListener('click',panelClick,false);
if (animate) {
if (suppressAnimation) {
var transition = panel.style.transition;
panel.style.transition = '';
panel.style.maxHeight = '0';
panel.style.border = 'none';
setTimeout(function() {
panel.style.transition = transition;
}, 40);
}
else {
panel.style.maxHeight = '0';
panelResetBorderTimerID = setTimeout(function() {
panel.style.border = 'none';
panelResetBorderTimerID = 0;
}, animMS);
}
}
else {
panel.style.display = 'none';
}
}
else {
if (!hasChildren(panel)) {
var xhr = new XMLHttpRequest();
xhr.onload = function() {
var doc = xhr.responseXML;
if (doc) {
var sm = doc.querySelector("ul#sitemap");
if (sm && xhr.status == 200) {
panel.innerHTML = sm.outerHTML;
showPanel();
}
}
}
var url = hbButton.href + (hbButton.href.includes("?")?"&popup":"?popup")
xhr.open("GET", url);
xhr.responseType = "document";
xhr.send();
}
else {
showPanel();
}
}
}
})();
/* menu.js *************************************************************/
function toggle_annotation_log(){
var w = document.getElementById("annotation_log");
var x = document.forms["f01"].elements["log"].checked
w.style.display = x ? "block" : "none";
}
function submenu_onchange_submit(){
var w = document.getElementById("f01");
w.submit();
}
(function (){
for(var i=0; 1; i++){
var x = document.getElementById("submenuctrl-"+i);
if(!x) break;
if( !x.hasAttribute('data-ctrl') ){
x.onchange = submenu_onchange_submit;
}else{
var cx = x.getAttribute('data-ctrl');
if( cx=="toggle_annotation_log" ){
x.onchange = toggle_annotation_log;
}
}
}
})();
</script>
</body>
</html>
