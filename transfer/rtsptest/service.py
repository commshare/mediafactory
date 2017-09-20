import socket
import time
import threading

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)  

s.bind(('127.0.0.1', 10021))

s.listen(1)

print('Server is running...')                          
                               
def TCP(sock, addr):

    print('Accept new connection from %s:%s.' %addr)

    while True:
        data = sock.recv(1024)                         
        time.sleep(1)                                  

        print 'received:', data
        if not data:
            print 'not data...' 
            break
        if data.decode() == 'quit':       
            print 'quit....' 
            break
        sock.send(data.decode('utf-8').upper().encode())

    sock.close()                                       
    print('Connection from %s:%s closed.' %addr)       

while True:
    
    sock, addr = s.accept()                           
    TCP(sock, addr)                                    
