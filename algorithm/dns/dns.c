#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

 

static void printmessage(unsigned char *buf);
static unsigned char *printnamestring(unsigned char *p,unsigned char *buf);

 

#define GETWORD(__w,__p) do{__w=*(__p++)<<8;__w|=*(p++);}while(0)
#define GETLONG(__l,__p) do{__l=*(__p++)<<24;__l|=*(__p++)<<16;__l|=*(__p++)<<8;__l|=*(p++);}while(0)

 

int main(int argc,char* argv[])
{
    if(argc != 2)
    {
       printf("usage: dnsclient <host_name>\n");
       return -1;
    }

 

    time_t ident;
    int fd;
    int rc;
    int serveraddrlent;
    char *q;
    unsigned char *p;
    unsigned char *countp;
    unsigned char reqBuf[512] = {0};
    unsigned char rplBuf[512] = {0};
    struct sockaddr_in serveraddr;
 

    //udp
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(fd == -1)
    {
       perror("error create udp socket");
       return -1;
    }

   

    time(&ident);
    //copy
    p = reqBuf;
    //Transaction ID
    *(p++) = ident;
    *(p++) = ident>>8;
    //Header section
    //flag word = 0x0100
    *(p++) = 0x01;
    *(p++) = 0x00;
    //Questions = 0x0001
    //just one query
    *(p++) = 0x00;
    *(p++) = 0x01;
    //Answer RRs = 0x0000
    //no answers in this message
    *(p++) = 0x00;
    *(p++) = 0x00;
    //Authority RRs = 0x0000
    *(p++) = 0x00;
    *(p++) = 0x00;
    //Additional RRs = 0x0000
    *(p++) = 0x00;
    *(p++) = 0x00;
    //Query section
    countp = p;  
    *(p++) = 0;

    for(q=argv[1]; *q!=0; q++)
    {
       if(*q != '.')
       {
           (*countp)++;
           *(p++) = *q;
       }
       else if(*countp != 0)
       {
           countp = p;
           *(p++) = 0;
       }
    }

    if(*countp != 0)
       *(p++) = 0;

 

    //Type=1(A):host address
    *(p++)=0;
    *(p++)=1;
    //Class=1(IN):internet
    *(p++)=0;
    *(p++)=1;

 

    printf("\nRequest:\n");
    printmessage(reqBuf);

 

    //fill
    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(53);
    //serveraddr.sin_addr.s_addr = inet_addr("192.168.1.102");
    //serveraddr.sin_addr.s_addr = inet_addr("220.181.57.216");
    serveraddr.sin_addr.s_addr = inet_addr("8.8.8.8");

 

    //send to DNS Serv
    if(sendto(fd,reqBuf,p-reqBuf,0,(void *)&serveraddr,sizeof(serveraddr)) < 0)
    {
       perror("error sending request");
       return -1;
    }

 

    //recev the reply
    bzero(&serveraddr,sizeof(serveraddr));
    serveraddrlent = sizeof(serveraddr);
    rc = recvfrom(fd,&rplBuf,sizeof(rplBuf),0,(void *)&serveraddr,&serveraddrlent);

    if(rc < 0)
    {
       perror("error receiving request\n");
       return -1;
    }  

 

    //print out results
    printf("\nReply:\n");
    printmessage(rplBuf);

 

    //exit
    printf("Program Exit\n");
    return 0; 
}

 

static void printmessage(unsigned char *buf)
{
    unsigned char *p;
    unsigned int ident,flags,qdcount,ancount,nscount,arcount;
    unsigned int i,j,type,class,ttl,rdlength;

 

    p = buf;
    GETWORD(ident,p);
    printf("ident=%#x\n",ident);

 

    GETWORD(flags,p);
    printf("flags=%#x\n",flags);

    //printf("qr=%u\n",(flags>>15)&1);
    printf("qr=%u\n",flags>>15);

 

    printf("opcode=%u\n",(flags>>11)&15);
    printf("aa=%u\n",(flags>>10)&1);
    printf("tc=%u\n",(flags>>9)&1);
    printf("rd=%u\n",(flags>>8)&1);
    printf("ra=%u\n",(flags>>7)&1);
    printf("z=%u\n",(flags>>4)&7);
    printf("rcode=%u\n",flags&15); 

 

    GETWORD(qdcount,p);
    printf("qdcount=%u\n",qdcount);

 

    GETWORD(ancount,p);
    printf("ancount=%u\n",ancount);

 

    GETWORD(nscount,p);
    printf("nscount=%u\n",nscount);

 

    GETWORD(arcount,p);
    printf("arcount=%u\n",arcount);

 

    for(i=0; i<qdcount; i++)
    {
       printf("qd[%u]:\n",i);
       while(*p!=0)
       {
           p = printnamestring(p,buf);
           if(*p != 0)
              printf(".");
       }

       p++;
       printf("\n");
       GETWORD(type,p);
       printf("type=%u\n",type);
       GETWORD(class,p);
       printf("class=%u\n",class);
    }

 

    for(i=0; i<ancount; i++)
    {
       printf("an[%u]:\n",i);
       p = printnamestring(p,buf);
       printf("\n");
       GETWORD(type,p);
       printf("type=%u\n",type);
       GETWORD(class,p);
       printf("class=%u\n",class);
       GETLONG(ttl,p);
       printf("ttl=%u\n",ttl);
       GETWORD(rdlength,p);
       printf("rdlength=%u\n",rdlength);
       printf("rd=");

       for(j=0; j<rdlength; j++)
       {
           printf("%2.2x(%u)",*p,*p);
           p++;
       }

       printf("\n");
    }
}

 

static unsigned char *printnamestring(unsigned char *p,unsigned char *buf)
{
    unsigned int nchars,offset;
 

    nchars = *(p++);
    if((nchars & 0xc0) == 0xc0)
    {
       offset = (nchars & 0x3f) << 8;
       offset |= *(p++);
       nchars = buf[offset++];
       printf("%*.*s",nchars,nchars,buf+offset);
    }
    else
    {
       printf("%*.*s",nchars,nchars,p);
       p += nchars;
    }

 
    return (p);
}
