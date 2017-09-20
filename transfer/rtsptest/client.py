#!/usr/bin/env python
#coding:utf-8

import socket

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM) 

s.connect(('127.0.0.1', 10021))                       

while True:                                           

    ttt = raw_input('pressss:')
    print(ttt)

    s.sendall(bytes('yaoyao'))

    print(s.recv(1024).decode('utf-8'))               

s.send(b'quit')                                       

s.close()                                             