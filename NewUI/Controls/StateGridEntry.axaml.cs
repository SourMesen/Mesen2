using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Avalonia.Media.Imaging;
using Avalonia.Threading;
using Mesen.Interop;
using Mesen.Localization;
using Mesen.ViewModels;
using ReactiveUI.Fody.Helpers;
using System.IO;
using System.IO.Compression;
using System.Threading.Tasks;

namespace Mesen.Controls
{
	public class StateGridEntry : UserControl
	{
		private static readonly WriteableBitmap EmptyImage = new WriteableBitmap(new PixelSize(256, 240), new Vector(96, 96), Avalonia.Platform.PixelFormat.Rgba8888, Avalonia.Platform.AlphaFormat.Opaque);

		public static readonly StyledProperty<RecentGameInfo> EntryProperty = AvaloniaProperty.Register<StateGridEntry, RecentGameInfo>(nameof(Entry));
		public static readonly StyledProperty<Bitmap?> ImageProperty = AvaloniaProperty.Register<StateGridEntry, Bitmap?>(nameof(Image));
		public static readonly StyledProperty<string> TitleProperty = AvaloniaProperty.Register<StateGridEntry, string>(nameof(Title));
		public static readonly StyledProperty<string> SubTitleProperty = AvaloniaProperty.Register<StateGridEntry, string>(nameof(SubTitle));
		public static readonly StyledProperty<bool> EnabledProperty = AvaloniaProperty.Register<StateGridEntry, bool>(nameof(Enabled));

		public RecentGameInfo Entry
		{
			get { return GetValue(EntryProperty); }
			set { SetValue(EntryProperty, value); }
		}

		public Bitmap? Image
		{
			get { return GetValue(ImageProperty); }
			set { SetValue(ImageProperty, value); }
		}

		public bool Enabled
		{
			get { return GetValue(EnabledProperty); }
			set { SetValue(EnabledProperty, value); }
		}

		public string Title
		{
			get { return GetValue(TitleProperty); }
			set { SetValue(TitleProperty, value); }
		}

		public string SubTitle
		{
			get { return GetValue(SubTitleProperty); }
			set { SetValue(SubTitleProperty, value); }
		}

		public StateGridEntry()
		{
			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		private void OnImageClick(object sender, RoutedEventArgs e)
		{
			if(!IsEffectivelyVisible) {
				return;
			}

			RecentGameInfo game = Entry;
			if(Path.GetExtension(game.FileName) == ".mss") {
				Task.Run(() => {
					//Run in another thread to prevent deadlocks etc. when emulator notifications are processed UI-side
					if(game.SaveMode) {
						EmuApi.SaveStateFile(game.FileName);
					} else {
						EmuApi.LoadStateFile(game.FileName);
					}
					EmuApi.Resume();
				});
			} else {
				Task.Run(() => {
					//Run in another thread to prevent deadlocks etc. when emulator notifications are processed UI-side
					EmuApi.LoadRecentGame(game.FileName, false);
				});
			}
		}

		public void Init()
		{
			RecentGameInfo game = Entry;
			if(game == null) {
				return;
			}

			Title = game.Name;

			bool fileExists = File.Exists(game.FileName);
			if(fileExists) {
				SubTitle = new FileInfo(game.FileName).LastWriteTime.ToString();
			} else {
				SubTitle = ResourceHelper.GetMessage("EmptyState");
			}
			Enabled = fileExists || game.SaveMode;

			if(fileExists) {
				Task.Run(() => {
					Bitmap? img = null;
					try {
						if(Path.GetExtension(game.FileName) == ".mss") {
							img = EmuApi.GetSaveStatePreview(game.FileName);
						} else {
							using FileStream fs = File.Open(game.FileName, FileMode.Open, FileAccess.Read, FileShare.Read);
							ZipArchive zip = new ZipArchive(fs);
							ZipArchiveEntry? entry = zip.GetEntry("Screenshot.png");
							if(entry != null) {
								using Stream stream = entry.Open();

								//Copy to a memory stream (to avoid what looks like a Skia or Avalonia issue?)
								using MemoryStream ms = new MemoryStream();
								stream.CopyTo(ms);
								ms.Seek(0, SeekOrigin.Begin);

								img = new Bitmap(ms);
							}
						}
					} catch { }

					Dispatcher.UIThread.Post(() => {
						Image = img;
					});
				});
			} else {
				Image = StateGridEntry.EmptyImage;
			}
		}
	}
}
