// Transfer Protocol classes
// TJW970621

// include FAX ???

#include "..\misc\typesize.h"

// begin dummies ...
typedef unsigned int size_t;
typedef unsigned int time_t;
typedef unsigned int FILE;
typedef int COMHANDLE;
#define MAXPATH 260
#define NULL 0
void strcpy(char *, char *){}
// end dummies

// all transfer protocols (Hydra,Janus,ZModem,XModem) are derived from this
// base class:

class TransferProtocol {
private:
  COMHANDLE handle;               // I/O stream's handle
                                  
  CHAR   txpathfname[MAXPATH];    // full path and (long) filename
  CHAR   rxpathfname[MAXPATH];
  CHAR   txpathfname8p3[MAXPATH]; // full path and 8.3 filename
  CHAR   rxpathfname8p3[MAXPATH];
  CHAR   txpath[MAXPATH];         // only path
  CHAR   rxpath[MAXPATH];
  CHAR   txfname[MAXPATH];        // only (long) filename
  CHAR   rxfname[MAXPATH];
  CHAR   txfname8p3[MAXPATH];     // only 8.3 filename
  CHAR   rxfname8p3[MAXPATH];

  time_t txtimestamp;             // UTC timestamp of files
  time_t rxtimestamp;
  FILE   *txfp;                   // file pointers (*fopen) - or maybe we 
  FILE   *rxfp;                   // better should use iostream lib !?
  UINT32 txpos;                   // current file xfer position
  UINT32 rxpos;
  UINT32 txstartpos;              // position we started (0) or resumed (!=0)
  UINT32 rxstartpos;              // from
  UINT32 txfilelen;               // total file length
  UINT32 rxfilelen;
  
  UINT32 txdiskfree;              // free disk space
  UINT32 rxdiskfree;

  UINT32 txblocklen;              // current protocol block length
  UINT32 rxblocklen;
  UINT32 txminblocklen;           // minimum protocol block length
  UINT32 rxminblocklen;
  UINT32 txmaxblocklen;           // maximum protocol block length
  UINT32 rxmaxblocklen;

  UINT32 txerrors;                // error counters
  UINT32 rxerrors;

  INT    debug;                   // debug flag
  
  CHAR   *buf;                    // bidi: Tx buffer, else: Rx/Tx buffer
  CHAR   *buf2;                   // bidi: Rx buffer
  size_t bufsize;                 // bidi: Tx buffer size, else Rx/Tx buf sz
  size_t buf2size;                // bidi: Rx buffer size

  VOID   send(UCHAR c);           // send byte to stream
  VOID   send(CHAR *s);           // send ASCIIZ string to stream
  VOID   send(CHAR *p, INT32 l);  // send buffer from p, length l
  INT    get(VOID);               // get byte from stream
    
public:
  TransferProtocol(COMHANDLE Handle =-1, INT Debug =0,
                   size_t TxBufSize =0, size_t RxBufSize =0);
  virtual ~TransferProtocol();
  TransferProtocol& operator=(const TransferProtocol& rhs); // no = operator!!!
  
  virtual INT BeginTransfer(VOID) =0;
  virtual INT Transfer(CHAR *TxFilename, CHAR *RxFilename) =0;
  virtual INT EndTransfer(VOID) =0;
};

class Hydra : public TransferProtocol {
private:

public:
  Hydra(COMHANDLE Handle, INT Debug);
  virtual ~Hydra();
  INT BeginTransfer(VOID);
  INT Transfer(CHAR *TxFilename, CHAR *RxFilename);
  INT EndTransfer(VOID);
};

class Janus : public TransferProtocol {
private:

public:
  Janus(COMHANDLE Handle, INT Debug);
  virtual ~Janus();
  INT BeginTransfer(VOID);
  INT Transfer(CHAR *TxFilename, CHAR *RxFilename);
  INT EndTransfer(VOID);

};

class ZModem : public TransferProtocol {
private:

public:
  ZModem(COMHANDLE Handle, INT Debug);
  virtual ~ZModem();
  INT BeginTransfer(VOID);
  INT Transfer(CHAR *TxFilename, CHAR *RxFilename);
  INT EndTransfer(VOID);

};

class XModem : public TransferProtocol {
private:

public:
  XModem(COMHANDLE Handle, INT Debug);
  virtual ~XModem();
  INT BeginTransfer(VOID);
  INT Transfer(CHAR *TxFilename, CHAR *RxFilename);
  INT EndTransfer(VOID);

};

// implementation ---------------------------------------------------------

TransferProtocol::TransferProtocol(COMHANDLE Handle, INT Debug,
                                   size_t TxBufSize, size_t RxBufSize){
  handle = Handle;
  debug = Debug;
  if(TxBufSize!=0){
    bufsize  = TxBufSize;
    buf      = new char[bufsize];
  }
  if(RxBufSize!=0){
    buf2size = RxBufSize;
    buf2     = new char[buf2size];
  }
  *txpathfname = *txpath = *txfname = '\0';
  *rxpathfname = *rxpath = *rxfname = '\0';
  txfp = rxfp = NULL;
  txpos = rxpos = txstartpos = rxstartpos = txfilelen = rxfilelen = 0;
  txerrors = rxerrors = 0;
}

TransferProtocol::~TransferProtocol(){
  if(bufsize!=0)
    delete[] buf;
  if(buf2size!=0)
    delete[] buf2;
}

VOID TransferProtocol::send(UCHAR c){

}

VOID TransferProtocol::send(CHAR *s){
  UCHAR *p;
  for(p=(UCHAR *)s;*p;p++)
    send(*p);
}

VOID TransferProtocol::send(CHAR *buf, INT32 len){
  UCHAR *p;
  INT32 l;
  for(p=(UCHAR *)buf,l=0;l<len;p++,l++)
    send(*p);
}

INT TransferProtocol::get(VOID){

}

    
Hydra::Hydra(COMHANDLE Handle, INT Debug) :
       TransferProtocol(Handle, Debug, 4200, 4200) {
}

Hydra::~Hydra() {
}

INT Hydra::BeginTransfer(VOID){
}

INT Hydra::Transfer(CHAR *TxFilename, CHAR *RxFilename){
}

INT Hydra::EndTransfer(VOID){
}

Janus::Janus(COMHANDLE Handle, INT Debug) :
       TransferProtocol(Handle, Debug, 4200, 4200) {
}

Janus::~Janus() {
}

INT Janus::BeginTransfer(VOID){
}

INT Janus::Transfer(CHAR *TxFilename, CHAR *RxFilename){
}

INT Janus::EndTransfer(VOID){
}

ZModem::ZModem(COMHANDLE Handle, INT Debug) :
       TransferProtocol(Handle, Debug, 8200, 0) {
}

ZModem::~ZModem() {
}

INT ZModem::BeginTransfer(VOID){
}

INT ZModem::Transfer(CHAR *TxFilename, CHAR *RxFilename){
}

INT ZModem::EndTransfer(VOID){
}

XModem::XModem(COMHANDLE Handle, INT Debug) :
       TransferProtocol(Handle, Debug, 1100, 0) {
}

XModem::~XModem() {
}

INT XModem::BeginTransfer(VOID){
}

INT XModem::Transfer(CHAR *TxFilename, CHAR *RxFilename){
}

INT XModem::EndTransfer(VOID){
}


int main(void){

 XModem x(1,0);
 ZModem z(1,0);
 Hydra  h(1,0);
 Janus  j(1,0);


}
