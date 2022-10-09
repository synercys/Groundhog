#!/usr/bin/env python3
import asyncio
import sys
import os
import time
import re

if len(sys.argv) != 1+1:
	print("Must pass server count")
	exit(1)
node_count = int(sys.argv[1])+1

#protocol: send one byte
#  if   byte is 'u', node that sent message is going up.
#  elif byte is 'd', node that sent message is going down.
#  else reply with string of bools corresponding to index

class UptimeServerProtocol:
	def __init__(self):
		print("Starting UDP server")
		self.state = [b'u']*(node_count)
		self.state_file = 'state.txt'
		self.prev_time = time.time()
		self.write_state_to_file(self.state_file)

	def connection_made(self, transport):
		self.transport = transport
	
	def change_in_state(self, prev_state):
		if prev_state == self.state:
			return False
		else:
			return True

	def write_state_to_file(self, state_file):
		with open(os.path.join(os.getcwd(), state_file), mode="a") as f:
			char_to_replace={" ":"", "'":"", "[":"", "]":"", ",":"", "b":""}
			value = re.sub(r"[b\[\ \'\],]", lambda x:char_to_replace[x.group(0)], str(self.state))
			print(f"{time.time()- self.prev_time}  {value}", file=f)

	def datagram_received(self, data, addr):
		print(f"Received {data.decode()} from {addr}")
		# ASHISH TODO: print to file self.state  everytime the state is updated.
		if data == b'u' or data == b'd':
			(ip, _) = addr
			idx = int(ip.split('.')[-1]) - 2
			print(f"Received {data.decode()} from {ip}")
			if idx in range(node_count):
				prev_state = self.state.copy()
				self.state[idx] = data
				if self.change_in_state(prev_state):
					self.write_state_to_file(self.state_file)
			else:
				print("IP out of expected range!")
		else:
			# replies saying uuddduuu
			print("sending", b"".join(self.state).decode())
			self.transport.sendto(b''.join(self.state), addr)

loop = asyncio.new_event_loop()
asyncio.set_event_loop(loop)

# One protocol instance will be created to serve all client requests
listen = loop.create_datagram_endpoint(UptimeServerProtocol,
	local_addr=('0.0.0.0', 5000))

transport, protocol = loop.run_until_complete(listen)

try:
	loop.run_forever()
except KeyboardInterrupt:
	pass

transport.close()
loop.close()
