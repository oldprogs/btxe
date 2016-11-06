// Fido Address classes
// TJW970622

#include "..\misc\typesize.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

class FTNAddress {
private:
  UINT16 zone;                    // FTN zone
  UINT16 net;                     // FTN net
  UINT16 node;                    // FTN node
  UINT16 point;                   // FTN point
  CHAR  *domain;                  // FTN domain
    
public:
  FTNAddress(const FTNAddress &);
  FTNAddress(UINT16 Zone =0, UINT16 Net =0, UINT16 Node =0, UINT16 Point =0,
             CHAR *Domain =NULL);
  FTNAddress(CHAR *Addr, FTNAddress& Default);
  ~FTNAddress();

  FTNAddress& operator=(const FTNAddress& rhs);
  INT operator==(const FTNAddress& rhs);

  UINT16 Zone() const;
  UINT16 Net() const;
  UINT16 Node() const;
  UINT16 Point() const;
  CHAR * Domain() const;
  print() const;
};

// implementation ---------------------------------------------------------

FTNAddress::FTNAddress(const FTNAddress &a){
  zone  = a.Zone();
  net   = a.Net();
  node  = a.Node();
  point = a.Point();
  domain = strdup(a.Domain()); 
}
  
FTNAddress::FTNAddress(UINT16 Zone, UINT16 Net, UINT16 Node, UINT16 Point,
                       CHAR *Domain){
  zone = Zone; net = Net; node = Node; point = Point;
  if(Domain==NULL)
    domain=NULL;
  else
    domain=strdup(Domain);
}

FTNAddress::FTNAddress(CHAR *Addr, FTNAddress& Default){
  INT ret;
  CHAR buf[256];
  
  zone   = Default.Zone();
  net    = Default.Net();
  node   = Default.Node();
  point  = 0;
  domain = strdup(Default.Domain());
  
  buf[0]='\0';  
  if((ret=sscanf(Addr,"%hd:%hd/%hd.%hd@%s",&zone,&net,&node,&point,buf))<3)
  {
    zone = Default.Zone();
    net  = Default.Net();
    if((ret=sscanf(Addr,"%hd/%hd.%hd@%s",&net,&node,&point,buf))<2)
    {
      net = Default.Net();
      if((ret=sscanf(Addr,"%hd.%hd@%s",&node,&point,buf))<1)
      {
        ret=sscanf(Addr,".%hd@%s",&point,buf);
      }
      else if(ret==1)
      {
        point = 0;
        sscanf(Addr,"%hd@%s",&node,buf);
      }
    }
    else if(ret==2)
    {
      point = 0;
      sscanf(Addr,"%hd/%hd@%s",&net,&node,buf);
    }
  }
  else if(ret==3)
  {
    point = 0;
    sscanf(Addr,"%hd:%hd/%hd@%s",&zone,&net,&node,buf);
  }
  if(buf[0]){
    free(domain);
    domain = strdup(buf);
  }
}

FTNAddress::~FTNAddress(){
  if(domain!=NULL)
    free(domain);
}

FTNAddress& FTNAddress::operator=(const FTNAddress& rhs){
  if(this!=&rhs){
    zone  = rhs.Zone();
    net   = rhs.Net();
    node  = rhs.Node();
    point = rhs.Point();
    free(domain);
    domain = strdup(rhs.Domain()); 
  }
  return *this;
}

INT FTNAddress::operator==(const FTNAddress& rhs){
  if(this!=&rhs){
    if( zone  == rhs.Zone() &&
        net   == rhs.Net()  &&
        node  == rhs.Node() &&
        point == rhs.Point() &&
        ( domain == NULL || rhs.Domain() == NULL ||
          strcmp(domain,rhs.Domain())==0))
      return 1;
    else
      return 0;
  }else
    return 1;
}

UINT16 FTNAddress::Zone()   const { return zone;   }
UINT16 FTNAddress::Net()    const { return net;    }
UINT16 FTNAddress::Node()   const { return node;   }
UINT16 FTNAddress::Point()  const { return point;  }
CHAR * FTNAddress::Domain() const { return domain; }

FTNAddress::print() const {
  printf("%hd:%hd/%hd.%hd@%s",zone,net,node,point,domain);
}


int main(void){

 FTNAddress x(2,2474,400,0,"fidonet");
 FTNAddress y("234.5",x);
 x.print();
 y.print();
 x=y;
 x.print();
 if(x==y) printf("x==y");
 else     printf("x!=y");
 
}
