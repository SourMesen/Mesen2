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
		private CpuType _cpuType;

		[Reactive] public List<CodeLabel> Labels { get; private set; } = new List<CodeLabel>();

		//For designer
		public LabelListViewModel() : this(CpuType.Cpu) { }

		public LabelListViewModel(CpuType cpuType)
		{
			_cpuType = cpuType;
			Id = "Labels";
			Title = "Labels";
			UpdateLabelList();
		}

		public void UpdateLabelList()
		{
  			Labels = LabelManager.GetLabels(_cpuType);
		}
	}
}
