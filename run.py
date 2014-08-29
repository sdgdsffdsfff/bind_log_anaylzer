
import time
import MySQLdb

cur = None
conn = None

def connect():
    global cur
    global conn
    try:
        conn=MySQLdb.connect(host='127.0.0.1',user='root',passwd='chinom',db='bind',port=3306)
        cur=conn.cursor()
    except MySQLdb.Error,e:
        print "Mysql Error %d: %s" % (e.args[0], e.args[1])
    pass

def close():
    global cur
    global conn
    cur.close()
    conn.close()

def add_to_table(res):
    global cur
    global conn

    if not res:
        return
    sql = 'INSERT INTO data (id, time, ip, port, domain, type, edns) VALUES (NULL, %s, %s, %s, %s, %s, %s)'
    params = [res[i] for i in xrange(len(res))]

    # cur.executemany(sql, params)
    cur.execute(sql, params)
    conn.commit()
    pass

def split_info(line):
    exp = line.split(' ')
    # print exp
    if len(exp) != 10:
        return None
    dt = time.strptime(' '.join([exp[0], exp[1] ]), "%d-%b-%Y %H:%M:%S.%f")
    dt = time.strftime('%Y-%m-%d %H:%M:%S')
    ip, port = exp[4].split('#')
    port = port[:-1]
    edns = (exp[9]=='-E\n') and 1 or 0
    # ret = {
    #     'time':dt,
    #     'ip':ip,
    #     'port':port,
    #     'domain':exp[6],
    #     'type':exp[8],
    #     'edns':edns
    # }

    ret = [str(dt), ip, port , exp[6], exp[8], edns]

    return ret
    pass

def anayse_file(f):
    connect()
    cur.execute('truncate table data')
    for line in open(f).xreadlines():
        add_to_table(split_info(line))
        # print split_info(line)
    close()

if __name__ == '__main__':
    anayse_file('log')