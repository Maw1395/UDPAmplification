from scapy.all import *
import threading
import sys, os,time 
import socket

def main():
	conf.L3socket=L3RawSocket
	fakesrc = '192.168.1.148'
	server = '73.57.245.108'
	device = 'wlp58s0'
	payload = "\x00"

	mypacket = (IP(dst="127.0.0.1", src='192.168.1.149')/UDP(dport=4352, sport=53)/payload)
	#request-
	print mypacket
	send(mypacket, iface="lo", loop=1)
if __name__=="__main__":
	main()
