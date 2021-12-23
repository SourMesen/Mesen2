using Dock.Model.ReactiveUI.Controls;
using Mesen.Debugger.Labels;
using Mesen.Interop;
using Mesen.ViewModels;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;

namespace Mesen.Debugger.ViewModels
{
	public class LabelListViewModel : Tool
	{
		public CpuType CpuType { get; }

		[Reactive] public List<CodeLabel> Labels { get; private set; } = new List<CodeLabel>();

		//For designer
		public LabelListViewModel() : this(CpuType.Cpu) { }

		public LabelListViewModel(CpuType cpuType)
		{
			CpuType = cpuType;
			Id = "Labels";
			Title = "Labels";
			CanPin = false;
			UpdateLabelList();
		}

		public void UpdateLabelList()
		{
  			Labels = LabelManager.GetLabels(CpuType);
		}
	}
}
