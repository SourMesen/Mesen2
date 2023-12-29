import emu
class LogStream(object):
	def __init__(self):
		from emu import log
		import sys
		
		self.log = log
		sys.stdout = self
		sys.stderr = self
		
	def write(self, msg):
		self.log(msg)
	def flush(self):
		pass

LogStream()

class eventType:
	def __init__(self):
		self.reset = 0
		self.nmi = 1
		self.irq = 2
		self.startFrame = 3
		self.endFrame = 4
		self.codeBreak = 5
		self.stateLoaded = 6
		self.stateSaved = 7
		self.inputPolled = 8
		self.spriteZeroHit = 9
		self.scriptEnded = 10

eventType = eventType()