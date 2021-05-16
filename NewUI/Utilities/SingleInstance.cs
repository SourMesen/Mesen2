using System;
using System.IO;
using System.IO.Pipes;
using System.Threading;
using System.Collections.Generic;
using System.Threading.Tasks;

namespace Mesen.Utilities
{
	public class SingleInstance : IDisposable
	{
		public static SingleInstance Instance { get; private set; } = new SingleInstance();

		private Mutex? _mutex;
		private Guid _identifier = Guid.Empty;
		private bool _disposed = false;
		private bool _firstInstance = false;

		public bool FirstInstance => _firstInstance;
		public event EventHandler<ArgumentsReceivedEventArgs>? ArgumentsReceived;

		public void Init(string[] args, bool enableSingleInstanceMode)
		{
			if(enableSingleInstanceMode) {
				_identifier = new Guid("{A46696B7-2D1C-4CC5-A52F-43BCAF094AEF}");
				_mutex = new Mutex(true, _identifier.ToString(), out _firstInstance);

				if(_firstInstance) {
					Task.Run(() => ListenForArguments());
				} else {
					PassArgumentsToFirstInstance(args);
				}
			} else {
				_firstInstance = true;
			}
		}

		private bool PassArgumentsToFirstInstance(string[] args)
		{
			try {
				using(NamedPipeClientStream client = new NamedPipeClientStream(_identifier.ToString())) {
					using(StreamWriter writer = new StreamWriter(client)) {
						client.Connect(200);

						foreach(string argument in args) {
							writer.WriteLine(argument);
						}
					}
				}
				return true;
			} catch { }

			return false;
		}

		private void ListenForArguments()
		{
			try {
				while(true) {
					using NamedPipeServerStream server = new NamedPipeServerStream(_identifier.ToString());
					using StreamReader reader = new StreamReader(server);
					server.WaitForConnection();

					List<string> args = new List<string>();
					while(server.IsConnected) {
						string? arg = reader.ReadLine();
						if(arg != null) {
							args.Add(arg);
						}
					}

					Task.Run(() => {
						ArgumentsReceived?.Invoke(this, new ArgumentsReceivedEventArgs(args.ToArray()));
					});
				}
			} catch(IOException) {
				//Pipe was broken
			} finally {
				Thread.Sleep(10000);
				Task.Run(() => ListenForArguments());
			}
		}

		#region IDisposable

		protected virtual void Dispose(bool disposing)
		{
			if(!_disposed) {
				if(_mutex != null && _firstInstance) {
					_mutex.ReleaseMutex();
					_mutex = null;
				}
				_disposed = true;
			}
		}

		~SingleInstance()
		{
			Dispose(false);
		}

		public void Dispose()
		{
			Dispose(true);
			GC.SuppressFinalize(this);
		}
		#endregion
	}

	public class ArgumentsReceivedEventArgs : EventArgs
	{
		public string[] Args { get; }

		public ArgumentsReceivedEventArgs(string[] args)
		{
			Args = args;
		}
	}
}