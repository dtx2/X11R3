/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.
Copyright 2021 Edward Halferty

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.*/
/*****************
 * WaitForSomething: - Make the server suspend until there is
 *	1. data from clients or
 *	2. input events available or
 *	3. ddx notices something of interest (graphics queue ready, etc.) or
 *	4. clients that have buffered replies/events are ready
 * If the time between INPUT events is greater than Scree nSaverTime, the display is turned off (or
 * saved, depending on the hardware).  So, WaitForSomething() has to handle this also (that's why the select() has a timeout.
 * For more info on ClientsWithInput, see ReadRequestFromClient(). pClientsReady is a mask, the bits set are
 * indices into the o.s. depedent table of available clients. (In this case, there is no table -- the index is the socket
 * file descriptor.)*/
void WaitForSomething(ClientPtr *pClientsReady, int *nready, ClientPtr *pNewClients, int *nnew) {
    int i, selecterr;
    struct timeval waittime, *wt;
    long timeout, curclient;
    long clientsReadable[mskcnt];
    long clientsWritable[mskcnt];
    if (! (ANYSET(ClientsWithInput))) {
        // We need a while loop here to handle crashed connections and the screen saver timeout
        while (1) {
            wt = NULL;
            if (AnyClientsWriteBlocked) { i = select (MAXSOCKS, LastSelectMask, clientsWritable, (int *) NULL, wt); }
            else { i = select (MAXSOCKS, LastSelectMask, (int *) NULL, (int *) NULL, wt); }
            selecterr = errno;
            if (i <= 0) { /* An error or timeout occurred */
                if (i < 0) {
                    if (selecterr == EBADF && !ANYSET (AllClients)) { return; } // Some client disconnected
                    else if (selecterr != EINTR) { ErrorF("WaitForSomething(): select: errno=%d\n", selecterr); }
                }
            } else {
                if (AnyClientsWriteBlocked && ANYSET (clientsWritable)) {
                    NewOutputPending = TRUE;
                    ORBITS(OutputPending, clientsWritable, OutputPending);
                    UNSETBITS(ClientsWriteBlocked, clientsWritable);
                    if (! ANYSET(ClientsWriteBlocked)) { AnyClientsWriteBlocked = FALSE; }
                }
                MASKANDSETBITS(clientsReadable, LastSelectMask, AllClients);
                if (LastSelectMask[0]) { EstablishNewConnections(pNewClients, nnew); }
                if (*nnew || (LastSelectMask[0] & EnabledDevices) || (ANYSET (clientsReadable))) { break; }
            }
        }
    } else { COPYBITS(ClientsWithInput, clientsReadable); }
    if (ANYSET(clientsReadable)) {
        for (i=0; i<mskcnt; i++) {
            while (clientsReadable[i]) {
                curclient = ffs (clientsReadable[i]) - 1;
                pClientsReady[(*nready)++] = ConnectionTranslation[curclient + (32 * i)];
                clientsReadable[i] &= ~(1 << curclient);
            }
        }
    }
}
int main(int argc, char *argv[]) {
    CreateUnixSocket();
    while(1) {
        while (1) {
            WaitForSomething(clientReady, &nready, newClients, &nnew);
            while (nnew--) { // Establish any new connections
                client = newClients[nnew];
                client->requestLogIndex = 0;
                WriteToClient(client, sizeof(xConnSetupPrefix), (char *) &connSetupPrefix);
                WriteToClient(client, connSetupPrefix.length << 2, ConnectionInfo);
                nClients++;
            }
            while ((nready--) > 0) { // Handle events in round robin fashion, doing input between each round
                client = clientReady[nready];
                if (!client) { continue; } // KillClient can cause this to happen
                isItTimeToYield = FALSE;
                requestingClient = client;
                while (!isItTimeToYield) {
                    ProcessInputEvents();
                    // now, finally, deal with client requests
                    request = (xReq *)ReadRequestFromClient(client, &result, (char *) request);
                    if (result < 0) { break; }
                    else if (result == 0) { continue; }
                    client->sequence++;
                    client->requestBuffer = (pointer) request;
                    if (client->requestLogIndex == MAX_REQUEST_LOG) { client->requestLogIndex = 0; }
                    client->requestLog[client->requestLogIndex] = request->reqType;
                    client->requestLogIndex++;
                }
            }
        }
        Dispatch();
    }
}
