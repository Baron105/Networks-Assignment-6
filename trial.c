#include <stdio.h>

int main()
{
    printf("Enter the query: (getIP N <domain-1> <domain-2> <domain-3> … <domain-N>) or EXIT\n");
    char query[1000];

    scanf("%[^\n]", query);

    int n;
    sscanf(query, "getIP %d", &n);


    // getIP 2 www.google.com www.yahoo.com

    char simDNSquery[1000] = {'\0'};

    int id = 1 ;
    int j = 15;
    // fill the first 16 char with the id 
    while(j>=0)
    {
        simDNSquery[j] = id%2 + '0';
        id/=2;
        j--;
    }
    // it is a query message
    simDNSquery[16] = '0' ;

    j = 19;
    int temp = n;
    while(j>16)
    {
        simDNSquery[j] = '0' + temp%2;
        temp/=2;
        j--;
    }
    j=20;
    // find length of each domain and the domain 
    int t=7;
    while(n--)
    {
        j--;
        while(query[t]==' ' || query[t]=='\0')
        {
            t++;
        }
        int len = 0;
        int start = t;

        while(query[t]!=' ' && query[t]!='\0')
        {
            len++;
            t++;
        }

        // put the len in the 4 bits from j+1 to j+4
        int temp  = len;
        int m = 4;
        while(m)
        {
            simDNSquery[j+m] = '0' + temp%2;
            temp/=2;
            m--;
        }

        j = j+5;

        // put the domain in the message
        for(int i = start; i<start+len; i++)
        {
            simDNSquery[j] = query[i];
            j++;
        }
    }

    printf("%s\n", simDNSquery);

    n=2;
    long ip[2] = {3232235777, 3232235778};

    // making the response packet

    char simDNSresponse[1000] = {'\0'};

    j = 15;
    // fill the first 16 char with the id
    while(j>=0)
    {
        simDNSresponse[j] = id%2 + '0';
        id/=2;
        j--;
    }
    // it is a response message
    simDNSresponse[16] = '1' ;

    j = 19;
    temp = n;
    while(j>16)
    {
        simDNSresponse[j] = '0' + temp%2;
        temp/=2;
        j--;
    }

    j=20;
    for(int n=0;n<2;n++)
    {
        // first bit tells if it valid or not
        simDNSresponse[j] = '1';

        j++;

        // put the ip address
        for(int i=31;i>=0;i--)
        {
            simDNSresponse[j+i] = '0' + ip[n]%2;
            ip[n]/=2;
        
        }
        j+=32;
    }

    printf("%s\n", simDNSresponse);



}