import optparse, socket, time, binascii

BUF_SIZE = 1600		# > 1500

ETH_P_ALL = 3		# To receive all Ethernet protocols
Interface = "wlan0"

### Packet field access ###

def SMAC(packet):
   #return binascii.hexlify(packet[6:12])
   return binascii.hexlify(packet[25:31])

def DMAC(packet):
   #return binascii.hexlify(packet[0:6])
   return binascii.hexlify(packet[31:37])

def EtherType(packet):
   return binascii.hexlify(packet[12:14]).decode()

def Payload(packet):
   return binascii.hexlify(packet[14:]).decode()


### Packet handler ###

def printPacket(packet, now, message):
   # print(message, len(packet), "bytes  time:", now,
   #       "\n  SMAC:", SMAC(packet), " DMAC:", DMAC(packet),
   #       " Type:", EtherType(packet), "\n  Payload:", Payload(packet)) # !! Python 3 !!
   #if SMAC(packet) == '0016ea123456' or DMAC(packet) == '0016ea123456':
	#print binascii.hexlify(packet)
	
   	print message, len(packet), "bytes time:", now, \
       	"\n  SMAC:", SMAC(packet), " DMAC:", DMAC(packet), " Type:", \
       	EtherType(packet)\
	, "\n ", binascii.hexlify(packet)
	#, "\n  Payload:", Payload(packet) # !! Python 2 !!
	

def terminal():
   # Open socket
   sock = socket.socket(socket.AF_PACKET, socket.SOCK_RAW, socket.htons(ETH_P_ALL))
   sock.bind((Interface, ETH_P_ALL))
   sock.setblocking(0)

   # Contents of packet to send (constant)
   #lastTime = time.time()
   while True:
      #now = time.time()

      try:
         packet = sock.recv(BUF_SIZE)
      except socket.error:
         pass
      else:
         if (DMAC(packet) == '0016ea123456'):
		break
         #dmac = DMAC(packet)
         #print dmac
      #printPacket(packet, now, "Received:")

terminal()
