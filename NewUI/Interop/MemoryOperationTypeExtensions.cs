namespace Mesen.Interop
{
	public static class MemoryOperationTypeExtensions
	{
		public static bool IsRead(this MemoryOperationType opType)
		{
			switch(opType) {
				default: return false;
				
				case MemoryOperationType.Read:
				case MemoryOperationType.DmaRead:
				case MemoryOperationType.DummyRead:
				case MemoryOperationType.ExecOpCode:
				case MemoryOperationType.ExecOperand:
				case MemoryOperationType.PpuRenderingRead:
					return true;
			}
		}

		public static bool IsWrite(this MemoryOperationType opType)
		{
			switch(opType) {
				default: return false;

				case MemoryOperationType.Write:
				case MemoryOperationType.DmaWrite:
				case MemoryOperationType.DummyWrite:
					return true;
			}
		}
	}
}
