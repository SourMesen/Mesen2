using System;
using System.IO;
using System.IO.Pipes;
using System.Threading;
using System.Collections.Generic;
using System.Threading.Tasks;

namespace Mesen.GUI
{
	public class ArgumentsReceivedEventArgs : EventArgs
	{
		public String[] Args { get; set; }
	}

	public class SingleInstance : IDisposable
	{
		/// <summary>
		/// Taken from flawlesscode.com (website no longer available)
		/// </summary>
		private Mutex _mutex = null;
		private bool _firstInstance = false;
		private Guid _identifier = Guid.Empty;
		private FileStream _fileStream = null;

		/// <summary>
		/// Enforces single instance for an application.
		/// </summary>
		/// <param name="identifier">An identifier unique to this application.</param>
		public SingleInstance()
		{
			if(Program.IsMono) {
				try {
					_fileStream = System.IO.File.Open("mesen.lock", FileMode.OpenOrCreate, FileAccess.ReadWrite, FileShare.None);
					_fileStream.Lock(0, 0);
					_firstInstance = true;
				} catch {
					_firstInstance = false;
				}
				try {
					if(File.Exists("mesen.arguments")) {
						File.Delete("mesen.arguments");
					}
				} catch { }
			} else {
				this._identifier = new Guid("{A46606B7-2D1C-4CC5-A52F-43BCAF094AED}");
				this._mutex = new Mutex(true, _identifier.ToString(), out _firstInstance);
			}
		}

		/// <summary>
		/// Indicates whether this is the first instance of this application.
		/// </summary>
		public bool FirstInstance { get { return _firstInstance; } }

		/// <summary>
		/// Passes the given arguments to the first running instance of the application.
		/// </summary>
		/// <param name="arguments">The arguments to pass.</param>
		/// <returns>Return true if the operation succeded, false otherwise.</returns>
		public bool PassArgumentsToFirstInstance(string[] arguments)
		{
			if(Program.IsMono) {
				try {
					File.WriteAllText("mesen.arguments", string.Join(Environment.NewLine, arguments));
				} catch { }

				return true;
			} else {
				try {
					using(NamedPipeClientStream client = new NamedPipeClientStream(_identifier.ToString())) {
						using(StreamWriter writer = new StreamWriter(client)) {
							client.Connect(200);

							foreach(String argument in arguments) {
								writer.WriteLine(argument);
							}
						}
					}
					return true;
				} catch { }

				return false;
			}
		}

		/// <summary>
		/// Listens for arguments being passed from successive instances of the applicaiton.
		/// </summary>
		public void ListenForArgumentsFromSuccessiveInstances()
		{
			Task.Run(() => this.ListenForArguments());
		}

		/// <summary>
		/// Listens for arguments on a named pipe.
		/// </summary>
		private void ListenForArguments()
		{
			if(Program.IsMono) {
				while(true) {
					if(File.Exists("mesen.arguments")) {
						try {
							string[] arguments = File.ReadAllLines("mesen.arguments");
							ThreadPool.QueueUserWorkItem(new WaitCallback(CallOnArgumentsReceived), arguments);							
							File.Delete("mesen.arguments");
						} catch { }
					}
					System.Threading.Thread.Sleep(200);
				}
			} else {
				try {
					while(true) {
						using(NamedPipeServerStream server = new NamedPipeServerStream(_identifier.ToString())) {
							using(StreamReader reader = new StreamReader(server)) {
								server.WaitForConnection();

								List<String> arguments = new List<String>();
								while(server.IsConnected) {
									arguments.Add(reader.ReadLine());
								}

								ThreadPool.QueueUserWorkItem(new WaitCallback(CallOnArgumentsReceived), arguments.ToArray());
							}
						}
					}
				} catch(IOException) {
					//Pipe was broken
				} finally {
					Thread.Sleep(10000);
					Task.Run(() => this.ListenForArguments());
				}
			}
		}

		/// <summary>
		/// Calls the OnArgumentsReceived method casting the state Object to String[].
		/// </summary>
		/// <param name="state">The arguments to pass.</param>
		private void CallOnArgumentsReceived(object state)
		{
			OnArgumentsReceived((string[])state);
		}

		/// <summary>
		/// Event raised when arguments are received from successive instances.
		/// </summary>
		public event EventHandler<ArgumentsReceivedEventArgs> ArgumentsReceived;

		/// <summary>
		/// Fires the ArgumentsReceived event.
		/// </summary>
		/// <param name="arguments">The arguments to pass with the ArgumentsReceivedEventArgs.</param>
		private void OnArgumentsReceived(string[] arguments)
		{
			if(ArgumentsReceived != null)
				ArgumentsReceived(this, new ArgumentsReceivedEventArgs() { Args = arguments });
		}

		#region IDisposable
		private Boolean _disposed = false;

		protected virtual void Dispose(bool disposing)
		{
			if(!_disposed) {
				if(_fileStream != null) {
					_fileStream.Dispose();
				}

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
}