using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Avalonia.Metadata;
using Mesen.Config;
using Mesen.Interop;
using Mesen.Localization;
using Mesen.Utilities;
using Mesen.Windows;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace Mesen.Controls;

public class FirmwareSelect : UserControl
{
	public static readonly StyledProperty<FirmwareType> FirmwareTypeProperty = AvaloniaProperty.Register<FirmwareSelect, FirmwareType>(nameof(FirmwareType));
	public static readonly StyledProperty<string> FilenameProperty = AvaloniaProperty.Register<FirmwareSelect, string>(nameof(Filename), "");
	public static readonly StyledProperty<string> WarningMessageProperty = AvaloniaProperty.Register<FirmwareSelect, string>(nameof(WarningMessage), "");

	public FirmwareType FirmwareType
	{
		get { return GetValue(FirmwareTypeProperty); }
		set { SetValue(FirmwareTypeProperty, value); }
	}

	public string Filename
	{
		get { return GetValue(FilenameProperty); }
		set { SetValue(FilenameProperty, value); }
	}

	public string WarningMessage
	{
		get { return GetValue(WarningMessageProperty); }
		set { SetValue(WarningMessageProperty, value); }
	}

	public FirmwareSelect()
	{
		InitializeComponent();
	}

	private void InitializeComponent()
	{
		AvaloniaXamlLoader.Load(this);
	}

	protected override void OnAttachedToVisualTree(VisualTreeAttachmentEventArgs e)
	{
		ValidateFirmware();
		base.OnAttachedToVisualTree(e);
	}

	private void ValidateFirmware()
	{
		bool incorrectSize = false;
		long fileSize = 0;

		Filename = "";
		WarningMessage = "";

		FirmwareFiles firmwareFiles = FirmwareType.GetFirmwareInfo();
		string? path = firmwareFiles.Names.Select(filename => Path.Combine(ConfigManager.FirmwareFolder, filename)).Where(File.Exists).FirstOrDefault();
		if(path != null) {
			fileSize = new FileInfo(path).Length;
			Filename = Path.GetFileName(path);

			foreach(FirmwareFileInfo firmwareInfo in firmwareFiles) {
				if(fileSize == firmwareInfo.Size) {
					string hash = FirmwareHelper.GetFileHash(path);
					if(Array.IndexOf(firmwareInfo.Hashes, hash) < 0) {
						WarningMessage = ResourceHelper.GetMessage("UnknownFirmwareHash", firmwareInfo.Hashes[0], hash);
					}
					incorrectSize = false;
					break;
				} else {
					incorrectSize = true;
				}
			}
		}

		if(incorrectSize) {
			WarningMessage = ResourceHelper.GetMessage("InvalidFirmwareSize", FirmwareType.GetFirmwareInfo()[0].Size, fileSize);
		}
	}

	private async void btnBrowse_OnClick(object sender, RoutedEventArgs e)
	{
		while(true) {
			string? selectedFile = await FileDialogHelper.OpenFile(null, VisualRoot, FileDialogHelper.FirmwareExt);
			if(selectedFile?.Length > 0 && File.Exists(selectedFile)) {
				if(await FirmwareHelper.SelectFirmwareFile(FirmwareType, selectedFile, VisualRoot)) {
					break;
				}
			} else {
				break;
			}
		}
		ValidateFirmware();
	}

	public async void DeleteFirmware(object parameter)
	{
		string path = Path.Combine(ConfigManager.FirmwareFolder, Filename);
		if(File.Exists(path)) {
			DialogResult result = await MesenMsgBox.Show(VisualRoot, "PromptDeleteFirmware", MessageBoxButtons.OKCancel, MessageBoxIcon.Warning, path);
			if(result == DialogResult.OK) {
				File.Delete(path);
			}
		}
		ValidateFirmware();
	}

	[DependsOn(nameof(Filename))]
	public bool CanDeleteFirmware(object parameter)
	{
		return Filename != null && Filename.Length > 0 && File.Exists(Path.Combine(ConfigManager.FirmwareFolder, Filename));
	}
}
