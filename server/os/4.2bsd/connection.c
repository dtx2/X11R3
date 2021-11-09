#define X_UNIX_DIR	"/tmp/.X11-unix"
#define X_UNIX_PATH	"/tmp/.X11-unix/X"
static struct sockaddr_un unsock;
static int open_unix_socket() {
    int oldUmask, request;
    unsock.sun_family = AF_UNIX;
    oldUmask = umask(0);
    mkdir(X_UNIX_DIR, 0777);
    strcpy(unsock.sun_path, X_UNIX_PATH);
    unlink(unsock.sun_path);
    if ((request = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) { Notice("Creating Unix socket"); }
    else {
        if(bind(request,(struct sockaddr *)&unsock, strlen(unsock.sun_path)+2)) { Error ("Binding Unix socket"); }
        if (listen (request, 5)) { Error("Unix Listening"); }
    }
    (void)umask(oldUmask);
    return request;
}
void CreateUnixSocket() { // At initialization, create the socket to listen on for new clients.
    int request, i, whichbyte, retry;
    for (i = 0; i < MAXSOCKS; i++) { ConnectionTranslation[i] = (ClientPtr)NULL; }
	lastfdesc = getdtablesize() - 1;
    if (lastfdesc > MAXSOCKS) { lastfdesc = MAXSOCKS; }
    whichbyte = 1;
    if (*(char *) &whichbyte) { whichByteIsFirst = 'l'; } else { whichByteIsFirst = 'B'; }
    if ((request = open_unix_socket ()) != -1) { unixDomainConnection = request; }
}
// EstablishNewConnections - If anyone is waiting on listened sockets, accept them. Returns a mask with indices of
// new clients.  Updates AllClients and AllSockets.
void EstablishNewConnections(ClientPtr *newclients, int *nnew) {
    if (readyconnections = (LastSelectMask[0])) {
        while (readyconnections) {
            curconn = ffs (readyconnections) - 1;
            if ((newconn = accept(curconn, (struct sockaddr *) NULL, (int *)NULL)) >= 0) {
                fcntl(newconn, F_SETFL, FNDELAY);
                if (next == (ClientPtr)NULL) {
                    xConnSetupPrefix c;
                    if (reason) {
                        c.success = xFalse;
                        c.lengthReason = strlen(reason);
                        c.length = (c.lengthReason + 3) >> 2;
                        c.majorVersion = X_PROTOCOL;
                        c.minorVersion = X_PROTOCOL_REVISION;
                        write(newconn, (char *)&c, sizeof(xConnSetupPrefix));
                        iov[0].iov_len = c.lengthReason;
                        iov[0].iov_base = reason;
                        iov[1].iov_len = padlength[c.lengthReason & 3];
                        iov[1].iov_base = p;
                        writev(newconn, iov, 2);
                    }
                }
            }
        }
    }
}