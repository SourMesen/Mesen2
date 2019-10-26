using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Mesen.GUI
{
	public static class MonoThemeHelper
	{
		public static MonoTheme Theme { get; private set; } = new MonoTheme();
		private static Dictionary<string, List<WeakReference<object>>> _excludedControls = new Dictionary<string, List<WeakReference<object>>>();

		public static void InitTheme(Color backColor)
		{
			if(backColor.R < 128 && backColor.G < 128 && backColor.B < 128) {
				Theme = new DarkMonoTheme();
			} else {
				Theme = new MonoTheme();
			}
		}

		public static bool IsDark { get { return Theme is DarkMonoTheme; } }

		public static void ExcludeFromTheme(Control ctrl)
		{
			if(Program.IsMono) {
				List<WeakReference<object>> refList;
				if(!_excludedControls.TryGetValue(ctrl.Name, out refList)) {
					refList = new List<WeakReference<object>>();
					_excludedControls[ctrl.Name] = refList;
				}
				refList.Add(new WeakReference<object>(ctrl));
			}
		}

		public static bool IsExcludedFromTheme(Control ctrl)
		{
			if(Program.IsMono) {
				List<WeakReference<object>> refList;
				if(_excludedControls.TryGetValue(ctrl.Name, out refList)) {
					foreach(WeakReference<object> weakRef in refList) {
						object target;
						if(weakRef.TryGetTarget(out target)) {
							return target == ctrl;
						}
					}
				}
			}
			return false;
		}

		public static void FixMonoColors(Form form)
		{
			if(Program.IsMono) {
				form.BackColor = Theme.FormBgColor;
				FixMonoColors(form, Theme);
			}
		}

		public static void FixMonoColors(ContextMenuStrip menu)
		{
			if(Program.IsMono) {
				if(menu.Tag == null || (bool)menu.Tag != true) {
					//Only process this context menu a single time (uses its Tag to know if we've processed it or not)
					FixMonoColors(menu, Theme);
					menu.Tag = true;
				}
			}
		}

		private static void FixMonoColors(ToolStripItem item, MonoTheme theme)
		{
			item.ForeColor = item.Enabled ? theme.ToolStripItemForeColor : theme.ToolStripItemDisabledForeColor;
			item.BackColor = theme.ToolStripItemBgColor;
			item.Font = new Font("Microsoft Sans Serif", 8.25f);

			item.EnabledChanged += (object sender, EventArgs e) => {
				((ToolStripItem)sender).ForeColor = ((ToolStripItem)sender).Enabled ? theme.ToolStripItemForeColor : theme.ToolStripItemDisabledForeColor;
			};

			if(item is ToolStripDropDownItem) {
				ToolStripDropDownItem ddItem = item as ToolStripDropDownItem;

				ddItem.DropDownOpening += MonoToolStripHelper.DropdownOpening;
				ddItem.DropDownClosed += MonoToolStripHelper.DropdownClosed;

				ddItem.DropDownOpening += (s, e) => {
					ddItem.DropDown.BackColor = theme.ToolStripItemBgColor;
					foreach(ToolStripItem subItem in ddItem.DropDownItems) {
						FixMonoColors(subItem, theme);
					}
				};

				foreach(ToolStripItem subItem in ddItem.DropDownItems) {
					FixMonoColors(subItem, theme);
				}
			}
		}

		private static void FixMonoColors(Control container, MonoTheme theme)
		{
			if(MonoThemeHelper.IsExcludedFromTheme(container)) {
				return;
			}

			if(container is TextBox) {
				TextBox txt = (TextBox)container;
				txt.BorderStyle = BorderStyle.FixedSingle;
				txt.BackColor = txt.ReadOnly ? theme.TextBoxDisabledBgColor : theme.TextBoxEnabledBgColor;
				txt.ForeColor = theme.TextBoxForeColor;
				txt.ReadOnlyChanged += (object sender, EventArgs e) => {
					((TextBox)sender).BackColor = ((TextBox)sender).ReadOnly ? theme.TextBoxDisabledBgColor : theme.TextBoxEnabledBgColor;
				};
			} else if(container is Label) {
				Label lbl = (Label)container;
				if(lbl.BackColor == Color.White) {
					//Trackbar labels
					lbl.BackColor = theme.TextBoxEnabledBgColor;
					lbl.ForeColor = theme.TextBoxForeColor;
				} else {
					if(lbl.ForeColor == SystemColors.GrayText) {
						//Headers
						lbl.ForeColor = theme.GrayTextColor;
					} else if(lbl.ForeColor == SystemColors.ControlDark) {
						//ctrlRiskyOption
						lbl.ForeColor = theme.DarkTextColor;
					} else {
						//Regular label
						lbl.ForeColor = lbl.Enabled ? theme.LabelForeColor : theme.LabelDisabledForeColor;
						lbl.EnabledChanged += (object sender, EventArgs e) => {
							((Label)sender).ForeColor = ((Label)sender).Enabled ? theme.LabelForeColor : theme.LabelDisabledForeColor;
						};
					}
				}
			} else if(container is CheckBox) {
				CheckBox chk = (CheckBox)container;
				chk.FlatStyle = FlatStyle.Flat;
				chk.ForeColor = chk.Enabled ? theme.LabelForeColor : theme.LabelDisabledForeColor;
				if(chk.BackColor == SystemColors.ControlLightLight) {
					//Enable equalizer checkbox
					chk.BackColor = theme.TabBgColor;
				}
				chk.EnabledChanged += (object sender, EventArgs e) => {
					((CheckBox)sender).ForeColor = ((CheckBox)sender).Enabled ? theme.LabelForeColor : theme.LabelDisabledForeColor;
				};
			} else if(container is RadioButton) {
				((RadioButton)container).ForeColor = ((RadioButton)container).Enabled ? theme.LabelForeColor : theme.LabelDisabledForeColor;
				((RadioButton)container).EnabledChanged += (object sender, EventArgs e) => {
					((RadioButton)sender).ForeColor = ((RadioButton)sender).Enabled ? theme.LabelForeColor : theme.LabelDisabledForeColor;
				};
			} else if(container is TrackBar) {
				((TrackBar)container).BackColor = theme.TabBgColor;
			} else if(container is Button) {
				Button btn = (Button)container;
				btn.FlatStyle = FlatStyle.Flat;
				btn.BackColor = btn.Enabled ? theme.ButtonEnabledBgColor : theme.ButtonDisabledBgColor;
				btn.ForeColor = theme.ButtonForeColor;

				btn.EnabledChanged += (object sender, EventArgs e) => {
					((Button)sender).BackColor = ((Button)sender).Enabled ? theme.ButtonEnabledBgColor : theme.ButtonDisabledBgColor;
				};
			} else if(container is ComboBox) {
				ComboBox cbo = (ComboBox)container;
				cbo.FlatStyle = FlatStyle.Flat;
				cbo.BackColor = cbo.Enabled ? theme.ComboEnabledBgColor : theme.ComboDisabledBgColor;
				cbo.ForeColor = theme.TextBoxForeColor;
				cbo.EnabledChanged += (object sender, EventArgs e) => {
					((ComboBox)sender).BackColor = ((ComboBox)sender).Enabled ? theme.ComboEnabledBgColor : theme.ComboDisabledBgColor;
				};
			} else if(container is GroupBox) {
				((GroupBox)container).ForeColor = theme.LabelForeColor;
			} else if(container is TabControl) {
				((TabControl)container).BackColor = theme.TabBgColor;
			} else if(container is TabPage) {
				((TabPage)container).BackColor = theme.TabBgColor;
			} else if(container is Panel && !(container is TableLayoutPanel) && !(container is FlowLayoutPanel)) {
				((Panel)container).BackColor = theme.PanelBgColor;
			} else if(container is DataGridView) {
				DataGridView dgv = (DataGridView)container;
				dgv.BackgroundColor = theme.ListBgColor;
				dgv.ForeColor = theme.LabelForeColor;
				dgv.GridColor = theme.LabelForeColor;
				dgv.DefaultCellStyle.ForeColor = theme.LabelForeColor;
				dgv.DefaultCellStyle.BackColor = theme.ListBgColor;
				dgv.ColumnHeadersDefaultCellStyle.ForeColor = theme.LabelForeColor;
				dgv.ColumnHeadersDefaultCellStyle.BackColor = theme.TabBgColor;
			} else if(container is DataGridTextBox) {
				((DataGridTextBox)container).BackColor = theme.TextBoxEnabledBgColor;
				((DataGridTextBox)container).ForeColor = theme.TextBoxForeColor;
			} else if(container is ListView) {
				((ListView)container).BackColor = theme.ListBgColor;
				((ListView)container).ForeColor = theme.LabelForeColor;
			} else if(container is ToolStrip) {
				((ToolStrip)container).BackColor = theme.FormBgColor;

				if(container is ContextMenuStrip) {
					((ContextMenuStrip)container).Opening += MonoToolStripHelper.ContextMenuOpening;
					((ContextMenuStrip)container).Closed += MonoToolStripHelper.ContextMenuClosed;
				} else {
					((ToolStrip)container).RenderMode = ToolStripRenderMode.System;
				}

				foreach(ToolStripItem item in ((ToolStrip)container).Items) {
					FixMonoColors(item, theme);
				}
			}

			if(container.ContextMenuStrip != null) {
				container.ContextMenuStrip.Opening += MonoToolStripHelper.ContextMenuOpening;
				container.ContextMenuStrip.Closed += MonoToolStripHelper.ContextMenuClosed;

				foreach(ToolStripItem item in container.ContextMenuStrip.Items) {
					FixMonoColors(item, theme);
				}
			}

			foreach(Control ctrl in container.Controls) {
				FixMonoColors(ctrl, theme);
			}
		}

		public class MonoTheme
		{
			public virtual Color TextBoxDisabledBgColor { get; } = Color.FromArgb(240, 240, 240);
			public virtual Color TextBoxEnabledBgColor { get; } = Color.FromArgb(255, 255, 255);
			public virtual Color TextBoxForeColor { get; } = Color.FromArgb(0, 0, 0);

			public virtual Color ButtonDisabledBgColor { get; } = Color.FromArgb(180, 180, 180);
			public virtual Color ButtonEnabledBgColor { get; } = Color.FromArgb(230, 230, 230);
			public virtual Color ButtonForeColor { get; } = Color.FromArgb(0, 0, 0);

			public virtual Color LabelForeColor { get; } = Program.IsMono ? Color.Black : SystemColors.ControlText;
			public virtual Color LabelDisabledForeColor { get; } = Color.Gray;
			public virtual Color ErrorTextColor { get; } = Color.Red;
			public virtual Color GrayTextColor { get; } = Program.IsMono ? Color.Gray : SystemColors.GrayText;
			public virtual Color DarkTextColor { get; } = Program.IsMono ? Color.LightGray : SystemColors.ControlDark;
			public virtual Color LinkTextColor { get; } = Color.FromArgb(61, 125, 255);

			public virtual Color ComboEnabledBgColor { get; } = Color.FromArgb(230, 230, 230);
			public virtual Color ComboDisabledBgColor { get; } = Color.FromArgb(180, 180, 180);

			public virtual Color FormBgColor { get; } = Program.IsMono ? Color.FromArgb(239, 240, 241) : SystemColors.Control;
			public virtual Color TabBgColor { get; } = Program.IsMono ? Color.White : Color.Transparent;
			public virtual Color PanelBgColor { get; } = Program.IsMono ? Color.FromArgb(239, 240, 241) : SystemColors.Control;
			public virtual Color ListBgColor { get; } = Program.IsMono ? Color.White : SystemColors.ControlLightLight;

			public virtual Color ToolStripItemBgColor { get; } = Program.IsMono ? Color.FromArgb(239, 240, 241) : SystemColors.Control;
			public virtual Color ToolStripItemForeColor { get; } = Color.Black;
			public virtual Color ToolStripItemDisabledForeColor { get; } = Color.Gray;
		}

		class DarkMonoTheme : MonoTheme
		{
			private static readonly Color TextColor = Color.FromArgb(255, 255, 255);
			private static readonly Color MainBgColor = Color.FromArgb(49, 54, 69);
			private static readonly Color HighlightBgColor = Color.FromArgb(69, 73, 70);
			private static readonly Color HighlightDisabledBgColor = Color.FromArgb(47, 52, 57);

			public override Color TextBoxDisabledBgColor { get; } = DarkMonoTheme.HighlightDisabledBgColor;
			public override Color TextBoxEnabledBgColor { get; } = Color.FromArgb(69, 73, 78);
			public override Color TextBoxForeColor { get; } = DarkMonoTheme.TextColor;

			public override Color ButtonDisabledBgColor { get; } = DarkMonoTheme.HighlightDisabledBgColor;
			public override Color ButtonEnabledBgColor { get; } = DarkMonoTheme.HighlightBgColor;
			public override Color ButtonForeColor { get; } = DarkMonoTheme.TextColor;

			public override Color LabelForeColor { get; } = DarkMonoTheme.TextColor;
			public override Color LabelDisabledForeColor { get; } = Color.Gray;
			public override Color ErrorTextColor { get; } = Color.IndianRed;
			public override Color GrayTextColor { get; } = Color.LightGray;
			public override Color DarkTextColor { get; } = Color.LightGray;
			public override Color LinkTextColor { get; } = Color.FromArgb(61, 125, 255);

			public override Color ComboEnabledBgColor { get; } = Color.FromArgb(69, 73, 78);
			public override Color ComboDisabledBgColor { get; } = Color.FromArgb(47, 52, 47);

			public override Color FormBgColor { get; } = DarkMonoTheme.MainBgColor;
			public override Color TabBgColor { get; } = Color.FromArgb(45, 49, 54);
			public override Color PanelBgColor { get; } = DarkMonoTheme.MainBgColor;
			public override Color ListBgColor { get; } = Color.FromArgb(45, 49, 54);

			public override Color ToolStripItemBgColor { get; } = DarkMonoTheme.MainBgColor;
			public override Color ToolStripItemForeColor { get; } = DarkMonoTheme.TextColor;
			public override Color ToolStripItemDisabledForeColor { get; } = Color.Gray;
		}
	}
}
