#include <sys/socket.h> 
#include <netinet/in.h> 
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <list>
#include <map>
#include <algorithm>

using namespace std;

#define FAILED -1
#define SUCCESS 0


struct IpSegment { 
    unsigned int start; 
    unsigned int end;
};

struct SegInfo
{
    IpSegment seg;
    string region;
    int count;
};

typedef list < unsigned int > IP_LIST;
typedef map < unsigned int, SegInfo > IP_RANGE_MAP; 

int get_ip_segmen(const string & line, unsigned int & ipwithmask, IpSegment & ipseg) { 
    if (line.find("//") == 0)  { 
        cout << "comment:" << line << endl; 
        return FAILED; 
    } 
    string ipstring; 
    size_t pos = line.find("/"); 
    if (pos == string::npos)  { 
        cout << "fail to find / :" << line << endl; 
        return FAILED; 
    } 
    unsigned int ip; 
    unsigned int ipstart; 
    unsigned int ipend; 
    ipstring = line.substr(0, pos); 
    ip = ntohl(inet_addr(ipstring.c_str())); 
    ipstart = ip;  //找到掩码, /和;之间的数字
     
    int bite = 32 - atoi(line.substr(pos + 1, line.length() - 1).c_str()); 
    if (bite >= 0 && bite <= 32)  { 
        ipend = ipstart | (0xffffffff << (32 - bite)) >> (32 - bite); 
        if (bite == 0)  { 
            ipend = ipstart; 
        } 
    } 
    else  { 
        cout << "wrong mask: " << line << endl; 
        return FAILED; 
    } 
    ipwithmask = (ip >> bite) << bite; 
    ipseg.start = ipstart; 
    ipseg.end = ipend; 
    return SUCCESS;
}
int store_ip_segments(const string & line, IP_RANGE_MAP & ipmap) { 
    unsigned int ipwithmask; 
    IpSegment ipseg;
    SegInfo  info;
    int ret = get_ip_segmen(line, ipwithmask, ipseg); 
    if (ret != SUCCESS)  { 
        return ret; 
    }
    info.seg = ipseg;
    ipmap.insert(pair < unsigned int, IpSegment > (ipwithmask, ipseg)); 
    return SUCCESS;
}
bool is_in_range(unsigned int ip,
    const IP_RANGE_MAP & ipmap) { 
    unsigned int iptmp = ip; 
    for (int i = 0; i < 32; i++)  { 
        IP_RANGE_MAP::const_iterator it = ipmap.find(ip); 
        if (it != ipmap.end() && iptmp >= it - > second.start && iptmp <= it - > second.end)  { 
            return true; 
        } 
        else  { 
            ip = (ip >> i) << i; 
        } 
    } 
    return false;
}
int main(int argc, char ** argv) {  
    //存储所有ip段,红黑树存储
    IP_RANGE_MAP g_ip_map; 
    if (argc < 3)  { 
        cout << "** oldipseg newipseg outputfile" << endl; 
        return 0; 
    } 
    char *oldipseg = argv[1]; 
    char *newiplist = argv[2]; 
    char *out = argv[3]; 
    ifstream oldifs(oldipseg); 
    if (!oldifs.is_open())  { 
        cout << "open old file fail" << endl; 
        return 0; 
    } 
    ifstream newifs(newiplist); 
    if (!newifs.is_open())  { 
        cout << "open new ip list file fail" << endl; 
        return 0; 
    }  //读取old
     
    string szline; 
    vector < string > lines; 
    while (getline(oldifs, szline))  { 
        if (store_ip_segments(szline, g_ip_map))  { 
            cout << "ignore line:" << szline << endl; 
            continue; 
        } 
    } 
    oldifs.close();  //创建输出文件
     
    ofstream ofs(out); 
    if (!ofs.is_open())  { 
        cout << "open output file faile" << endl; 
    } 
    struct in_addr addr;  //读取new ip段文件
     
    while (getline(newifs, szline))  { 
        unsigned useless; 
        IpSegment ipseg; 
        if (get_ip_segmen(szline, useless, ipseg))  { 
            cout << "ignore line in new file: " << szline << endl; 
        } 
        else  { 
            cout << "deal " << szline << endl; 
            for (unsigned int i = ipseg.start; i <= ipseg.end; ++i)  { 
                if (is_in_range(i, g_ip_map))  { 
                    unsigned int ip = htonl(i); 
                    memcpy( & addr, & ip, 4); 
                    ofs << inet_ntoa(addr) << endl; 
                } 
            } 
            ofs.flush(); 
        } 
    } 
    ofs.close(); 
    return 0;
}