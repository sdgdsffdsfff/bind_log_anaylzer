#include <sys/socket.h> 
#include <netinet/in.h> 
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
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


struct SegInfomation
{
    IpSegment seg;
    string region;
    int count;
};

typedef list<unsigned int> IP_LIST;
typedef map<unsigned int,SegInfomation> IP_RANGE_MAP;

int get_ip_segmen(const string & line, unsigned int & ipwithmask, IpSegment & ipseg) {
    if (line.find("//") == 0) {
        cout << "comment:" << line << endl;
        return FAILED;
    }
    string ipstring;
    size_t pos = line.find("/");
    if (pos == string::npos) {
        // cout << "fail to find / :" << line << endl;
        return FAILED;
    }
    unsigned int ip;
    unsigned int ipstart;
    unsigned int ipend;
    ipstring = line.substr(0, pos);
    ip = ntohl(inet_addr(ipstring.c_str()));
    ipstart = ip; //找到掩码, /和;之间的数字
    
    int bite = 32 - atoi(line.substr(pos + 1, line.length() - 1).c_str());
    if (bite >= 0 && bite <= 32) {
        ipend = ipstart | (0xffffffff << (32 - bite)) >> (32 - bite);
        if (bite == 0) {
            ipend = ipstart;
        }
    }
    else {
        cout << "wrong mask: " << line << endl;
        return FAILED;
    }
    ipwithmask = (ip >> bite) << bite;
    ipseg.start = ipstart;
    ipseg.end = ipend;
    return SUCCESS;
}


string get_region_name(const string &line)
{
    string key = "region ";
    string tail = " {";
    if (line.find(key) == 0 &&  line.substr(line.length() - tail.length(), tail.length()) == tail)
    {
        return line.substr(key.length(), line.length() - key.length() - tail.length());
    }
    return "";
}

int store_ip_segments(const string & line, const string &region, IP_RANGE_MAP & ipmap) {
    unsigned int ipwithmask;
    IpSegment ipseg;
    SegInfomation  info;

    int ret = get_ip_segmen(line, ipwithmask, ipseg);
    if (ret != SUCCESS) {
        return ret;
    }
    info.seg = ipseg;
    info.region = region;
    info.count = 0;
    ipmap.insert(pair < unsigned int, SegInfomation > (ipwithmask, info));
    return SUCCESS;
}
bool is_in_range(unsigned int ip, IP_RANGE_MAP & ipmap) {
    unsigned int iptmp = ip;
    for (int i = 0; i < 32; i++) {
        IP_RANGE_MAP::iterator it = ipmap.find(ip);
        if (it != ipmap.end() && iptmp >= it-> second.seg.start && iptmp <= it-> second.seg.end) {
            (it->second.count)++;
            return true;
        }
        else {
            ip = (ip >> i) << i;
        }
    }
    return false;
}
int main(int argc, char ** argv) { 
    //存储所有ip段,红黑树存储
    IP_RANGE_MAP g_ip_map;
    if (argc < 3) {
        cout << "** ipseg iplist outputfile" << endl;
        return 0;
    }
    char *ipseg_path = argv[1];
    char *iplist_path = argv[2];
    char *out = argv[3];
    ifstream ipseg_file(ipseg_path);
    if (!ipseg_file.is_open()) {
        cout << "open ip segment file fail" << endl;
        return 0;
    }
    ifstream iplist_file(iplist_path);
    if (!iplist_file.is_open()) {
        cout << "open ip list file fail" << endl;
        return 0;
    }
    
    string szline;
    vector < string > lines;
    string region;
    while (getline(ipseg_file, szline)) {
        string r = get_region_name(szline);
        if (r != "")
        {
            region = r;
            cout << region <<endl;
        }
        else
        {
            if (store_ip_segments(szline, region, g_ip_map)) {
                cout << "ignore line:" << szline << endl;
                continue;
            }
        }
    }
    ipseg_file.close();

    //创建输出文件
    ofstream ofs(out);
    if (!ofs.is_open()) {
        cout << "open output file faile" << endl;
    }
    
    while (getline(iplist_file, szline)) {
        unsigned int ip = ntohl(inet_addr(szline.c_str()));
        is_in_range(ip, g_ip_map);
    }

    IP_RANGE_MAP::const_iterator it;
    for (it = g_ip_map.begin(); it != g_ip_map.end(); ++it)
    {
        ofs<< it->second.region << ":" << it->second.count << endl;
    }
    ofs.close();
    return 0;
}