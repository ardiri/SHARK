/*************************************************************************
 *
 * Copyright 2001+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-socket.inc
 */

/*************************************************************************
 * Configuration
 *************************************************************************/

#define HTTP_HEADER_SIZE   128

/*************************************************************************
 * Component Variables
 *************************************************************************/

typedef struct _sock
{
  char      host[MAX_HOSTNAME_LENGTH];
  uint16    port;
  uint32    mode;

  boolean   open;

  sock     *proxy;
  char     *proxy_header;

#if defined(DAL_SOCK)
  DAL_SOCK
#endif

} _sock;

typedef struct SockGlobals
{
  void *struct_prefix;

#if defined(DAL_SOCK_GLOBALS)
  DAL_SOCK_GLOBALS
#endif

} SockGlobals;

GLOBALS_DEFINITION;

/*************************************************************************
 * Platform Dependent Interface
 *************************************************************************/

static boolean  DAL_SocketInitialize();
static void     DAL_SocketClose(_sock *s);
static _sock   *DAL_SocketGetHandle(char *host, uint16 port, uint32 mode);
static boolean  DAL_SocketConnect(_sock *s);
static error    DAL_SocketRead(_sock *s, void *buf, uint32 len, uint32 *cnt);
static void     DAL_SocketReleaseHandle(_sock *s);
static error    DAL_SocketWrite(_sock *s, void *buf, uint32 len, uint32 *cnt);
static void     DAL_SocketTerminate();

/*************************************************************************
 * Implementation
 *************************************************************************/

static boolean
_SocketInitialize()
{
  boolean init_ok;

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _SHARKLogCreateEntry(">> _SocketInitialize");
#endif

  // allocate memory for globals
  SOCK_GLOBALS = malloc(sizeof(SockGlobals));
  if (GLOBALS_UNINITIALIZED) return false;

  // assume everything is ok
  init_ok =  true;
  init_ok &= DAL_SocketInitialize();

  return init_ok;
}

void
_SocketClose(sock *s)
{
  _sock *_s = (_sock *)s;
  _sock *w;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // entry requirements
  if ((_s == NULL) || (!_s -> open)) return;

  // make sure the right socket is used
  w = _s;
  if (_s -> proxy != NULL) w = (_sock *)_s -> proxy;

  // pass through to underlying hardware layer
  DAL_SocketClose(w);

  _s -> open = false;
}

sock *
_SocketGetHandle(char *host, uint16 port, uint32 mode)
{
  _sock *result;

  // default return value
  result = NULL;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return (sock *)result;

  // entry requirement
  if ((host == NULL)      ||
      (port == 0)         ||
      ((mode & (SOCKET_TCP | SOCKET_UDP)) == 0) ||
      ((mode & (SOCKET_TCP | SOCKET_UDP)) == (SOCKET_TCP | SOCKET_UDP)))
    return (sock *)result;

  // query the underlying hardware platform
  result = DAL_SocketGetHandle(host, port, mode);

  // we have no proxy header for now
  result -> proxy_header = NULL;

  return (sock *)result;
}

boolean
_SocketOpen(sock *s)
{
  boolean  result;
  _sock   *_s = (_sock *)s;
  _sock   *w;
  uint32   len, cnt;
  error    err;
  char    *p, buf[HTTP_HEADER_SIZE+1];
  boolean  success;

  // default return value
  result = false;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return result;

  // entry requirements
  if ((_s == NULL) || (_s -> open)) return result;

  // make sure the right socket is used
  w = _s;
  if (_s -> proxy != NULL) w = (_sock *)_s -> proxy;

  // connect to the socket
  if (DAL_SocketConnect(w))
  {
    w -> open = true;

    // simple socket?
    if (_s -> proxy == NULL) goto SOCKET_ESTABLISHED;

    // build the HTTP tunnel connection string
    success = false;
    sprintf(buf, "CONNECT %s:%d HTTP/1.0\n", _s -> host, _s -> port);
    if (w -> proxy_header != NULL) strcat(buf, w -> proxy_header);
    strcat(buf, "\n");
    len = strlen(buf);

    // write to the socmket
    err = _SocketWrite((sock *)w, buf, len, &cnt);
    if ((err == errNone) && (cnt == len))
    {
      // lets wait quarter of second for server to respond
      _SysDelay(250);

      // lets process the HTTP response
      err = _SocketReadLine((sock *)w, buf, HTTP_HEADER_SIZE, &cnt);
      if (err == errNone)
      {
        p = buf; while (*p != ' ') p++; p++; // skip HTTP/1.x
        if (strncmp(p, "200", 3) == 0)
        {
          // skip over the remainding HTTP headers
          while ((err == errNone) && (cnt != 0))
          { 
            err = _SocketReadLine((sock *)w, buf, HTTP_HEADER_SIZE, &cnt); 
            if (*buf == 0) break;
          } 

          // we have successfully connected
          success = true;
        }
      }
    }

    // did we receive 200 OK
    if (!success) goto SOCKET_OPEN_ABORT;

SOCKET_ESTABLISHED:;

    _s -> open = true;
    result     = true;
  }

SOCKET_OPEN_ABORT:;

  return result;
}

error
_SocketRead(sock *s, void *buf, uint32 len, uint32 *cnt)
{
  error  result;
  _sock *_s = (_sock *)s;
  _sock *w;

  // default return value
  result = -1;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return result;

  // entry requirements
  if ((_s == NULL) || (!_s -> open)) return result;

  // make sure the right socket is used
  w = _s;
  if (_s -> proxy != NULL) w = (_sock *)_s -> proxy;

  // receive the information from the package
  result = DAL_SocketRead(w, buf, len, cnt);

  return result;
}

error
_SocketReadLine(sock *s, void *buf, uint32 len, uint32 *cnt)
{
  error   result, err;
  uint32  amt, total;
  uint8  *p;

  // default return value
  result = -1;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return result;

  // process until maximum number of bytes read in
  total = 0;
  p     = (uint8 *)buf;
  while (total < len)
  {
    // read in one byte at a time
    amt = 0;
    err = _SocketRead(s, p, 1, &amt);
    if ((err == errNone) || (amt == 0)) return result;

    total += amt;
    if (*p == '\015') continue; // ignore CR
    if (*p == '\012') break;    // LF is the separator
    p++;
  }
  *p = 0;

  // if we got this far, we actually read something
  result = errNone;

  // we need to tell the caller how many bytes were actually processed
  if (cnt != NULL) *cnt = total;

  return result;
}

error
_SocketReadWaitAll(sock *s, void *buf, uint32 len, uint32 *cnt)
{
  error   result, err;
  uint32  amt, total;
  uint8  *p;

  // default return value
  result  = -1;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return result;

  // process until maximum number of bytes read in
  total = 0;
  p = (uint8 *)buf;
  while (total < len)
  {
    // try to read in as many bytes as possible
    amt = 0;
    err = _SocketRead(s, p, (uint32)(len-total), &amt);
    if ((err == errNone) || (amt == 0)) return result;

    total += amt; 
    p     += amt;
  }
  *p = 0;

  // if we got this far, we actually read something
  result = errNone;

  // we need to tell the caller how many bytes were actually processed
  if (cnt != NULL) *cnt = total;

  return result;
}

void
_SocketReleaseHandle(sock *s)
{
  _sock *_s = (_sock *)s;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // entry requirements
  if (_s == NULL) return;

  // ensure socket if closed 
  _SocketClose(s);

  // make sure the right socket is used
  if (_s -> proxy != NULL) _SocketReleaseHandle(_s -> proxy);
  _s -> proxy = NULL;

  // do we have any additional headers?
  if (_s -> proxy_header != NULL) free(_s -> proxy_header);
  _s -> proxy_header = NULL;

  // pass through to underlying hardware layer
  DAL_SocketReleaseHandle(_s);
}

void
_SocketSetProxy(sock *s, char *host, uint16 port, char *user, char *pass)
{
  char   buf[128], auth[128], *pS, *pD;
  int    cnt, len;
  char   table[] =
           "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  _sock *_s = (_sock *)s;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // entry requirements
  if ((_s == NULL) || (_s -> open) || (_s -> mode != SOCKET_TCP)) return;

  // do we already have a socket defined?
  if (_s -> proxy != NULL) _SocketReleaseHandle(_s -> proxy);

  // lets try and define our proxy socket
  _s -> proxy = _SocketGetHandle(host, port, SOCKET_TCP);
  if (_s -> proxy != NULL) 
  {
    _s = (_sock *)_s -> proxy;

    // are we configuring proxy authentication?
    if ((user != NULL) || (pass != NULL))
    {
      strcpy(buf, ""); 
      if (user != NULL) strcat(buf, user);
      strcat(buf, ":"); 
      if (pass != NULL) strcat(buf, pass);

      // base64 encode the username and password string
      pS = buf; pD = auth; cnt = strlen(pS);
      while (cnt > 0)
      {
        switch (MIN(3, cnt))
        {
          case 3:
               *pD++ = table[  pS[0]         >> 2 ];
               *pD++ = table[((pS[0] & 0x03) << 4) | (pS[1] >> 4)];
               *pD++ = table[((pS[1] & 0x0F) << 2) | (pS[2] >> 6)];
               *pD++ = table[  pS[2] & 0x3F ];

               cnt -= 3; pS += 3;
               break;

          case 2:
               *pD++ = table[  pS[0]         >> 2 ];
               *pD++ = table[((pS[0] & 0x03) << 4) | (pS[1] >> 4)];
               *pD++ = table[((pS[1] & 0x0F) << 2)];
               *pD++ = '=';

               cnt -= 2; pS += 2;
               break;

          case 1:
               *pD++ = table[  pS[0]         >> 2 ];
               *pD++ = table[((pS[0] & 0x03) << 4)];
               *pD++ = '=';
               *pD++ = '=';

               cnt --; pS++;
               break;

          default:
               break;
        }
      }
      *pD++ = 0;

      // build the proxy header for authorization
      len = (strlen(buf)*8 + 5)/6;
      _s -> proxy_header = malloc(32+len);
      sprintf(_s -> proxy_header, "Proxy-Authorization: Basic %s\n", auth);
    }
  }
}

error
_SocketWrite(sock *s, void *buf, uint32 len, uint32 *cnt)
{
  int32  result;
  _sock *_s = (_sock *)s;
  _sock *w;

  // default return value
  result = -1;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return result;

  // entry requirements
  if ((_s == NULL) || (!_s -> open)) return result;

  // make sure the right socket is used
  w = _s;
  if (_s -> proxy != NULL) w = (_sock *)_s -> proxy;

  // send the data to the socket
  result = DAL_SocketWrite(w, buf, len, cnt);

  return result;
}

static void
_SocketTerminate()
{
#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _SHARKLogCreateEntry(">> _SocketTerminate");
#endif

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  DAL_SocketTerminate();

  // free memory for globals
  free(SOCK_GLOBALS); 
  SOCK_GLOBALS = NULL;
}

#undef HTTP_HEADER_SIZE

/********************************* EOF ***********************************/
