using Mesen.Interop;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Config;

public class WsConfig : BaseConfig<WsConfig>
{
	[Reactive] public ConsoleOverrideConfig ConfigOverrides { get; set; } = new();

	[Reactive] public ControllerConfig ControllerHorizontal { get; set; } = new();
	[Reactive] public ControllerConfig ControllerVertical { get; set; } = new();

	[Reactive] public WsModel Model { get; set; } = WsModel.Auto;
	[Reactive] public bool UseBootRom { get; set; } = false;

	[Reactive] public bool AutoRotate { get; set; } = true;

	[Reactive] public bool BlendFrames { get; set; } = true;
	[Reactive] public bool LcdAdjustColors { get; set; } = true;
	[Reactive] public bool LcdShowIcons { get; set; } = true;

	[Reactive] public bool HideBgLayer1 { get; set; } = false;
	[Reactive] public bool HideBgLayer2 { get; set; } = false;
	[Reactive] public bool DisableSprites { get; set; } = false;

	[Reactive] public WsAudioMode AudioMode { get; set; } = WsAudioMode.Headphones;
	[Reactive][MinMax(0, 100)] public UInt32 Channel1Vol { get; set; } = 100;
	[Reactive][MinMax(0, 100)] public UInt32 Channel2Vol { get; set; } = 100;
	[Reactive][MinMax(0, 100)] public UInt32 Channel3Vol { get; set; } = 100;
	[Reactive][MinMax(0, 100)] public UInt32 Channel4Vol { get; set; } = 100;
	[Reactive][MinMax(0, 100)] public UInt32 Channel5Vol { get; set; } = 100;

	public void ApplyConfig()
	{
		ControllerHorizontal.Type = ControllerType.WsController;
		ControllerVertical.Type = ControllerType.WsControllerVertical;

		ConfigManager.Config.Video.ApplyConfig();

		ConfigApi.SetWsConfig(new InteropWsConfig() {
			ControllerHorizontal = ControllerHorizontal.ToInterop(),
			ControllerVertical = ControllerVertical.ToInterop(),

			Model = Model,
			UseBootRom = UseBootRom,
			
			AutoRotate= AutoRotate,

			BlendFrames = BlendFrames,
			LcdAdjustColors = LcdAdjustColors,
			LcdShowIcons = LcdShowIcons,

			HideBgLayer1 = HideBgLayer1,
			HideBgLayer2 = HideBgLayer2,
			DisableSprites = DisableSprites,

			AudioMode = AudioMode,
			Channel1Vol = Channel1Vol,
			Channel2Vol = Channel2Vol,
			Channel3Vol = Channel3Vol,
			Channel4Vol = Channel4Vol,
			Channel5Vol = Channel5Vol,
		});
	}

	internal void InitializeDefaults(DefaultKeyMappingType defaultMappings)
	{
		ControllerHorizontal.InitDefaults(defaultMappings, ControllerType.WsController);
		ControllerVertical.InitDefaults(defaultMappings, ControllerType.WsControllerVertical);
	}
}

[StructLayout(LayoutKind.Sequential)]
public struct InteropWsConfig
{
	public InteropControllerConfig ControllerHorizontal;
	public InteropControllerConfig ControllerVertical;

	public WsModel Model;
	[MarshalAs(UnmanagedType.I1)] public bool UseBootRom;

	[MarshalAs(UnmanagedType.I1)] public bool AutoRotate;

	[MarshalAs(UnmanagedType.I1)] public bool BlendFrames;
	[MarshalAs(UnmanagedType.I1)] public bool LcdAdjustColors;
	[MarshalAs(UnmanagedType.I1)] public bool LcdShowIcons;
	
	[MarshalAs(UnmanagedType.I1)] public bool HideBgLayer1;
	[MarshalAs(UnmanagedType.I1)] public bool HideBgLayer2;
	[MarshalAs(UnmanagedType.I1)] public bool DisableSprites;

	public WsAudioMode AudioMode;
	public UInt32 Channel1Vol;
	public UInt32 Channel2Vol;
	public UInt32 Channel3Vol;
	public UInt32 Channel4Vol;
	public UInt32 Channel5Vol;
}

public enum WsModel : byte
{
	Auto,
	Monochrome,
	Color,
	SwanCrystal
}

public enum WsAudioMode : byte
{
	Headphones,
	Speaker
}