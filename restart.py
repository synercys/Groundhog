#!/usr/bin/env python3
import logging
import subprocess
import sys
import socket

exe = "/usr/local/bin/test" # Path to the program this script runs
logfile = "/usr/local/app.log" # File where logs will be saved
logging.basicConfig(filename=logfile, filemode='w', level=logging.DEBUG, format='%(name)s - %(levelname)s - %(message)s') # Set up logging so we can keep track of what the program is doing

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # Create a socket using the internet protocol (IPv4) and UDP
server_addr = ('10.0.0.254', 5000)# IP and port of the server that keeps track of which nodes are up or down
proto_dn = b'd' # Means this node is down
proto_up = b'u' # Means this node is up
proto_rq = b'r' # request

def getIP(): # Get the current computer’s IP address
	import socket
	hostname = socket.gethostname()
	return socket.gethostbyname(hostname)


def findNextPrime(n): # Find the next prime number after n (used for math tricks later)
	if(n <= 1):
		return 2
	prime = n
	while(1):
		prime += 1
		if(isPrime(prime)):
			break
	return prime



def isPrime(n): # Check if a number is prime (used inside findNextPrime)
	if (n <= 1):
		return False 
	if (n <= 3):
		return True 
	if (n%2 == 0 or n%3 == 0):
		return False
	i= 5
	while(i*i<=n):
		if(n%i ==0 or n%(i+2)==0):
			return False
		i = i+6
	return True


def getCurrNodeIdx(ips,ip): # Get this node’s index in the list of all node IPs
	current_node = -1
	for i in range(len(ips)):
		if(ips[i] == ip):
			current_node = i
			break
	return current_node


class RandomNodePicker: # Picks which node should reboot next using modular arithmetic
	def __init__(self, n):
		self.n = n  # Number of nodes
		self.prime = findNextPrime(n)  # Find a prime number > n
		self.generators = [] # Store all valid generator sequences
		for i in range(1,n+1):
			generatedNums = []
			generatedNums = self.findGeneratedNums(i,generatedNums)
			if(len(generatedNums) == n):
				self.generators.append((i,generatedNums)) # Save the generator and sequence
				break
			self.currGeneratorIdx = 0 # Start at first generator and first number in sequence
			self.nextGeneratedNumIdx = 0


	def nextNode(self): # Return the next node number in the current sequence
		nodeNum = self.generators[self.currGeneratorIdx][1][self.nextGeneratedNumIdx]
		self.nextGeneratedNumIdx += 1
		if(self.nextGeneratedNumIdx == n):
			self.nextGeneratedNumIdx = 0
			self.currGeneratorIdx = (self.currGeneratorIdx + 1)% len(self.generators)
		return nodeNum

	def findGeneratedNums(self,i,generatedNums): # Generate a unique sequence of node numbers using modular math
		generatedNumsSet = []
		powersOfiModPrime = 1
		for x in range(0,self.prime):
			if(powersOfiModPrime not in generatedNumsSet):
				generatedNumsSet.append(powersOfiModPrime)
				if(powersOfiModPrime >= 1 and powersOfiModPrime <= self.n):
					generatedNums.append(powersOfiModPrime-1)
			powersOfiModPrime = (powersOfiModPrime * i ) % self.prime
		return generatedNums


class Algorithm: # This class runs the logic to determine when to reboot this node
	def __init__(self,ips, n, attackTime, rebootTime, t, nodePicker):
		self.ip = getIP()
		self.mIntervals = max(1,attackTime//rebootTime)  # Total number of reboot periods
		self.currNodeIdx = getCurrNodeIdx(ips,self.ip) # This node’s position in the network
		self.t = t # Minimum number of nodes that should always be up
		self.attackTime = attackTime
		self.rebootTime = rebootTime
		self.nodePicker = nodePicker  # How to pick next node to reboot
		self.n = n  # Total number of nodes
		self.numRebootsSoFar = 0 # Count of how many times this node has rebooted


	def rebootAfterTime(self, timeToReboot): # Reboot the node after a certain time
		import time
		import os
		self.numRebootsSoFar += 1
		sock.sendto(proto_up, server_addr) # Notify server: this node is going up
		print("Going up")

		# Reboot logic
		# timetoReboot is the time after which the node is scheduled to be rebooted. 
		try: # Try to run the test process for timeToReboot seconds
			# process = subprocess.run("/home/ubuntu/redise/dise/bin/test",universal_newlines=True,capture_output=False,timeout=timeToReboot)
			process = subprocess.run([exe, "-n", str(server_count)],
			# process = subprocess.run(["/bin/sleep", "100"],
					universal_newlines=True, capture_output=True, timeout=timeToReboot)
			sys.stdout.flush()
		except subprocess.TimeoutExpired:
			sock.sendto(proto_dn, server_addr) # Notify server: timeout = going down
			logging.debug("timeout done")
			print("timeout done")
		finally:
			sock.sendto(proto_dn, server_addr)  # Notify server again we are down
			time.sleep(self.rebootTime) # Wait for reboot time
		print("Went down")

	def run(self): # The logic that decides when this node should reboot
		if ((self.t) < self.mIntervals):
			# Simple case: fewer intervals than required nodes
			subsetSize = self.t
			# print("here")
			# print("subset size: ", subsetSize, " mIntervals:",self.mIntervals)
			logging.debug("node number: "  + str(self.currNodeIdx))
			logging.debug("subset size: " + str(subsetSize) + " mIntervals:" + str(self.mIntervals))
			N = self.numRebootsSoFar*n
			while(self.nodePicker.nextNode() != self.currNodeIdx):
				N += 1
			# print("N",N)
			logging.debug("N" + str(N))
			if(self.numRebootsSoFar == 0):
				N = ((N//subsetSize) * self.mIntervals) + (N % subsetSize)
				timeToReboot = N* rebootTime
			else:
				M = N - self.n
				N = ((N//subsetSize) * self.mIntervals) + (N % subsetSize)
				M = ((M//subsetSize) * self.mIntervals) + (M % subsetSize)
				timeToReboot = (N - M-1) * self.rebootTime
			# print("timeToReboot: ", timeToReboot) 
			logging.debug("timeToReboot: " + str(timeToReboot))
			if(self.numRebootsSoFar>0):
				timeToReboot += 10  # Extra delay after first reboot
			self.rebootAfterTime(timeToReboot)

		else:
			# More intervals than required nodes
			import math
			subsetSize = int(math.ceil(self.t/self.mIntervals))
			# print("subset size: ", subsetSize, " mIntervals:",self.mIntervals)
			logging.debug("node number: "  + str(self.currNodeIdx))
			logging.debug("subset size: " +  str(subsetSize) + " mIntervals:" + str(self.mIntervals))
			N = self.numRebootsSoFar*n
			while(self.nodePicker.nextNode() != self.currNodeIdx):
				N += 1
			# print("N",N)
			logging.debug("N" + str(N))
			if(self.numRebootsSoFar == 0):
				timeToReboot = N//subsetSize * rebootTime
			else:
				M = N - self.n
				M = (M//subsetSize)*subsetSize + subsetSize
				N = (N//subsetSize)*subsetSize
				# print(N,M)    
				timeToReboot = ((N-M)//subsetSize)*self.rebootTime
			
			# print("timeToReboot: ", timeToReboot) 
			logging.debug("timeToReboot: " +  str(timeToReboot))
			if(self.numRebootsSoFar>0):
				timeToReboot += 10
			self.rebootAfterTime(timeToReboot)
# === Start of the main program ===
# Make sure the user gave 4 arguments
if len(sys.argv) != 1+4:
	print("Must specify [server count] [threshold fraction] [attack time] [reboot time]")
	exit(1)
# Read arguments from command line
server_count = int(sys.argv[1]) # How many other servers there are
node_count = server_count + 1  # Include this node

# Create a list of IP addresses for each node
ips = []
for i in range(node_count):
	ips.append("10.0.0." + str(i+2))

# DARPA study 
# Time and threshold parameters
attackTime = int(sys.argv[3]) 
rebootTime = int(sys.argv[4])
t = int(node_count * float(sys.argv[2]))  # t = number of nodes that must be running at all times

# Initialize node picker and algorithm
n = node_count
nodePicker = RandomNodePicker(n)
# print(nodePicker.generators)
logging.debug(nodePicker.generators)
algo = Algorithm(ips,n,attackTime,rebootTime,t,nodePicker)
while(1): # Keep running the reboot logic forever
	algo.run()










#case 2
# timeToReboot = (((N//subsetSize) + (N % subsetSize)) - ((M//subsetSize) + (M % subsetSize)) ) * self.rebootTime
# if(self.numRebootsSoFar > 0):
#     self.nodePicker.currGeneratorIdx = (self.numRebootsSoFar-1) % len(self.nodePicker.generators)
#     self.nodePicker.nextGeneratedNumIdx = 0
#     while(self.nodePicker.nextNode()!= self.currNodeIdx):
#         N = N + 1
#     while(self.nodePicker.nextGeneratedNumIdx % subsetSize != 0):
#         self.nodePicker.nextNode()
# # print("Generator's state: (", self.nodePicker.currGeneratorIdx,", ", self.nodePicker.nextGeneratedNumIdx, ")" )
# nodesToWait = 0
# while(self.nodePicker.nextNode() != self.currNodeIdx):
#     nodesToWait += 1
# print("nodesToWait: ", nodesToWait)
# timeToReboot = (nodesToWait//subsetSize)*rebootTime

#case 1

