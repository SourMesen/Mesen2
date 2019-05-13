using System;
using System.ComponentModel;
using System.Windows.Forms;

namespace FastColoredTextBoxNS
{
    ///
    /// These classes are required for correct data binding to Text property of FastColoredTextbox
    /// 
    class FCTBDescriptionProvider : TypeDescriptionProvider
    {
        public FCTBDescriptionProvider(Type type)
            : base(GetDefaultTypeProvider(type))
        {
        }

        private static TypeDescriptionProvider GetDefaultTypeProvider(Type type)
        {
            return TypeDescriptor.GetProvider(type);
        }



        public override ICustomTypeDescriptor GetTypeDescriptor(Type objectType, object instance)
        {
            ICustomTypeDescriptor defaultDescriptor = base.GetTypeDescriptor(objectType, instance);
            return new FCTBTypeDescriptor(defaultDescriptor, instance);
        }
    }

    class FCTBTypeDescriptor : CustomTypeDescriptor
    {
        object instance;

        public FCTBTypeDescriptor(ICustomTypeDescriptor parent, object instance)
            : base(parent)
        {
            this.instance = instance;
        }

        public override string GetComponentName()
        {
            var ctrl = (instance as Control);
            return ctrl == null ? null : ctrl.Name;
        }

        public override EventDescriptorCollection GetEvents()
        {
            var coll = base.GetEvents();
            var list = new EventDescriptor[coll.Count];

            for (int i = 0; i < coll.Count; i++)
                if (coll[i].Name == "TextChanged")//instead of TextChanged slip BindingTextChanged for binding
                    list[i] = new FooTextChangedDescriptor(coll[i]);
                else
                    list[i] = coll[i];

            return new EventDescriptorCollection(list);
        }
    }

    class FooTextChangedDescriptor : EventDescriptor
    {
        public FooTextChangedDescriptor(MemberDescriptor desc)
            : base(desc)
        {
        }

        public override void AddEventHandler(object component, Delegate value)
        {
            (component as FastColoredTextBox).BindingTextChanged += value as EventHandler;
        }

        public override Type ComponentType
        {
            get { return typeof(FastColoredTextBox); }
        }

        public override Type EventType
        {
            get { return typeof(EventHandler); }
        }

        public override bool IsMulticast
        {
            get { return true; }
        }

        public override void RemoveEventHandler(object component, Delegate value)
        {
            (component as FastColoredTextBox).BindingTextChanged -= value as EventHandler;
        }
    }
}
