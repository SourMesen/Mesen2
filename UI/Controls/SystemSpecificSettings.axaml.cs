using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using System;
using System.Collections.Generic;
using System.Reflection;
using System.Linq;
using Mesen.Config;
using Mesen.Localization;
using Avalonia.Interactivity;
using Avalonia.Data;
using Mesen.ViewModels;
using Mesen.Windows;

namespace Mesen.Controls
{
	public class SystemSpecificSettings : UserControl
	{
		public static readonly StyledProperty<ConfigType> ConfigTypeProperty = AvaloniaProperty.Register<SystemSpecificSettings, ConfigType>(nameof(ConfigType));

		public ConfigType ConfigType
		{
			get { return GetValue(ConfigTypeProperty); }
			set { SetValue(ConfigTypeProperty, value); }
		}

		public SystemSpecificSettings()
		{
			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		private void OnClickNes(object sender, RoutedEventArgs e)
		{
			NavigateTo(ConfigWindowTab.Nes);
		}

		private void OnClickSnes(object sender, RoutedEventArgs e)
		{
			NavigateTo(ConfigWindowTab.Snes);
		}

		private void OnClickGameboy(object sender, RoutedEventArgs e)
		{
			NavigateTo(ConfigWindowTab.Gameboy);
		}

		private void OnClickGba(object sender, RoutedEventArgs e)
		{
			NavigateTo(ConfigWindowTab.Gba);
		}

		private void OnClickPcEngine(object sender, RoutedEventArgs e)
		{
			NavigateTo(ConfigWindowTab.PcEngine);
		}

		private void OnClickSms(object sender, RoutedEventArgs e)
		{
			NavigateTo(ConfigWindowTab.Sms);
		}

		private void OnClickWs(object sender, RoutedEventArgs e)
		{
			NavigateTo(ConfigWindowTab.Ws);
		}

		private void NavigateTo(ConfigWindowTab console)
		{
			if(VisualRoot is ConfigWindow wnd && wnd.DataContext is ConfigViewModel cfg) {
				cfg.SelectTab(console);
				
				switch(console) {
					case ConfigWindowTab.Nes:
						if(cfg.Nes != null) {
							cfg.Nes.SelectedTab = ConfigType switch {
								ConfigType.Audio => NesConfigTab.Audio,
								ConfigType.Emulation => NesConfigTab.Emulation,
								ConfigType.Input => NesConfigTab.Input,
								_ or ConfigType.Video => NesConfigTab.Video,
							};
						}
						break;
					
					case ConfigWindowTab.Snes:
						if(cfg.Snes != null) {
							cfg.Snes.SelectedTab = ConfigType switch {
								ConfigType.Audio => SnesConfigTab.Audio,
								ConfigType.Emulation => SnesConfigTab.Emulation,
								ConfigType.Input => SnesConfigTab.Input,
								_ or ConfigType.Video => SnesConfigTab.Video,
							};
						}
						break;
					
					case ConfigWindowTab.Gameboy:
						if(cfg.Gameboy != null) {
							cfg.Gameboy.SelectedTab = ConfigType switch {
								ConfigType.Audio => GameboyConfigTab.Audio,
								ConfigType.Emulation => GameboyConfigTab.Emulation,
								ConfigType.Input => GameboyConfigTab.Input,
								_ or ConfigType.Video => GameboyConfigTab.Video,
							};
						}
						break;

					case ConfigWindowTab.Gba:
						if(cfg.Gba != null) {
							cfg.Gba.SelectedTab = ConfigType switch {
								ConfigType.Audio => GbaConfigTab.Audio,
								ConfigType.Emulation => GbaConfigTab.Emulation,
								ConfigType.Input => GbaConfigTab.Input,
								_ or ConfigType.Video => GbaConfigTab.Video,
							};
						}
						break;

					case ConfigWindowTab.PcEngine:
						if(cfg.PcEngine != null) {
							cfg.PcEngine.SelectedTab = ConfigType switch {
								ConfigType.Audio => PceConfigTab.Audio,
								ConfigType.Emulation => PceConfigTab.Emulation,
								ConfigType.Input => PceConfigTab.Input,
								_ or ConfigType.Video => PceConfigTab.Video,
							};
						}
						break;

					case ConfigWindowTab.Sms:
						if(cfg.Sms != null) {
							cfg.Sms.SelectedTab = ConfigType switch {
								ConfigType.Audio => SmsConfigTab.Audio,
								ConfigType.Emulation => SmsConfigTab.Emulation,
								ConfigType.Input => SmsConfigTab.Input,
								_ or ConfigType.Video => SmsConfigTab.Video,
							};
						}
						break;

					case ConfigWindowTab.Ws:
						if(cfg.Ws != null) {
							cfg.Ws.SelectedTab = ConfigType switch {
								ConfigType.Audio => WsConfigTab.Audio,
								ConfigType.Emulation => WsConfigTab.Emulation,
								ConfigType.Input => WsConfigTab.Input,
								_ or ConfigType.Video => WsConfigTab.Video,
							};
						}
						break;
				}
			}
		}
	}

	public enum ConfigType
	{
		Audio,
		Emulation,
		Input,
		Video
	}
}
