import emu
class LogStream(object):
	def __init__(self):
		import sys
		
		self.log = emu.log
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

class memoryType:
	def __init__(self):
		self.nesMemory = 7
		self.ppuMemory = 8
		self.nesPrgRom = 36
		self.nesInternalRam = 37
		self.nesWorkRam = 38
		self.nesSaveRam = 39
		self.nesNametableRam = 40
		self.nesSpriteRam = 41
		self.nesSecondarySpriteRam = 42
		self.nesPaletteRam = 43
		self.nesChrRam = 44
		self.nesChrRom = 45

memoryType = memoryType()

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