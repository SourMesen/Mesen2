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
		self.NesPrgRom = 36
		self.NesInternalRam = 37
		self.NesWorkRam = 38
		self.NesSaveRam = 39
		self.NesNametableRam = 40
		self.NesSpriteRam = 41
		self.NesSecondarySpriteRam = 42
		self.NesPaletteRam = 43
		self.NesChrRam = 44
		self.NesChrRom = 45

memoryType = memoryType()