/*************************************************************************
 *
 * Copyright 2001+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL-socket.inc
 */

/*************************************************************************
 * Configuration
 *************************************************************************/

// none

/*************************************************************************
 * Component Variables
 *************************************************************************/

#define SOCK_GLOBALS          g_sock
#define GLOBALS_UNINITIALIZED (SOCK_GLOBALS == NULL)
#define GLOBALS_DEFINITION    SockGlobals *g_sock = NULL

// _sock:: platform additions
#define DAL_SOCK \
  struct hostent     *he;  \
  struct sockaddr_in  sin; \
  int32               fd;

/*************************************************************************
 * Platform Independent Implementation
 *************************************************************************/

#include "../common/SHARK-socket.inc"

/*************************************************************************
 * Implementation
 *************************************************************************/

static boolean
DAL_SocketInitialize()
{
  boolean init_ok;
  WSADATA wsaData;
  int32   nCode;

  // default return value
  init_ok = true;

  // clear memory used by memory globals
  memset(SOCK_GLOBALS, 0, sizeof(SockGlobals));

  // initialize the winsock library
  nCode = WSAStartup(0x0101, &wsaData);

  init_ok &= (nCode == 0);

  return init_ok;
}

static void
DAL_SocketClose(_sock *s)
{
  // close the socket
  closesocket(s -> fd);
}

static _sock *
DAL_SocketGetHandle(char *host, uint16 port, uint32 mode)
{
  _sock              *result;
  struct hostent     *he;
  struct sockaddr_in  sin;
  int32               fd;

  // default return value
  result = NULL;

  he = gethostbyname(host);
  if (he == NULL) return result;

  memset(&sin, 0, sizeof(struct sockaddr_in));
  sin.sin_family = AF_INET;
  sin.sin_port   = htons(port);
  memcpy(&sin.sin_addr, he -> h_addr, he -> h_length);

  // define our socket (TCP or UDP)
  fd = -1;
  if ((mode & SOCKET_TCP) == SOCKET_TCP) fd = socket(PF_INET, SOCK_STREAM, 0);
  if ((mode & SOCKET_UDP) == SOCKET_UDP) fd = socket(PF_INET, SOCK_DGRAM,  0);

  // did the socket get created successfully?
  if (fd != -1)
  {
    // basic properties
    result = (_sock *)malloc(sizeof(_sock));
    if (result != NULL)
    {
      memset(result, 0, sizeof(_sock));

      strcpy(result -> host, host);
      result -> port    = port;
      result -> mode    = mode;

      // we need to store this information to connect
      result -> he      = he;
      memcpy(&result -> sin, &sin, sizeof(struct sockaddr_in));
      result -> fd      = fd;
      result -> open    = false;
    }
  }

  return result;
}

static boolean
DAL_SocketConnect(_sock *s)
{
  boolean result;

  // try to open the socket
  result = (connect(s -> fd, 
                  (struct sockaddr *)&s -> sin, sizeof(struct sockaddr)) == 0);

  return result;
}

static error
DAL_SocketRead(_sock *s, void *buf, uint32 len, uint32 *cnt)
{
  error result;
  int   amt;

  // default return value
  result = errNone;

  // try to send data through the socket
  amt = recv(s -> fd, buf, len, 0);

  // we must tell caller how many bytes were written, and set error
  if (amt < 0) result = -1;
  if (cnt != NULL) *cnt = amt;

  return result;
}

static void
DAL_SocketReleaseHandle(_sock *s)
{
  // destroy the socket handle and free
  s -> fd = -1;
  free(s);
}

static error
DAL_SocketWrite(_sock *s, void *buf, uint32 len, uint32 *cnt)
{
  error result;
  int   amt;

  // default return value
  result = errNone;

  // try to send data through the socket
  amt = send(s -> fd, buf, len, 0);

  // we must tell caller how many bytes were written, and set error
  if (amt < 0) result = -1;
  if (cnt != NULL) *cnt = amt;

  return result;
}

void 
DAL_SocketTerminate()
{
  // clean up
  WSACleanup();
}

#undef GLOBALS_DEFINITION
#undef GLOBALS_UNINITIALIZED
#undef SOCK_GLOBALS

/********************************* EOF ***********************************/
