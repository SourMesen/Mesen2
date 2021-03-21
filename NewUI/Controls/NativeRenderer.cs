using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Avalonia.Platform;
using Mesen.GUI;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen
{
   public class NativeRenderer : NativeControlHost
   {
      public NativeRenderer()
      {
         
      }

      public IntPtr Handle { get; private set; }

      protected override IPlatformHandle CreateNativeControlCore(IPlatformHandle parent)
      {
         var handle = base.CreateNativeControlCore(parent);
         Handle = handle.Handle;
         return handle;
      }
   }
}
