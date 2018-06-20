using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;

namespace Knife {
    class ContainerTypeExistException : Exception { }
    class ContainerBadEntryException : Exception { }

    class IocEntry {
        public IocEntry() {
            ValueReference = null;
            Generator = null;
        }

        public object ValueReference {
            get; set;
        }

        public object Generator {
            get; set;
        }
    }

    class Ioc {
		public static IocContainer Instance { get; } = new IocContainer();

        public static IocContainer Register<T>(Func<T> input) where T : class => Instance.Register(input);
        public static IocContainer Register<T>(T input) where T : class => Instance.Register(input);
        public static IocContainer Register<T>() where T : class => Instance.Register<T>();
        public static bool Registered<T>() => Instance.Registered<T>();
        public static T Get<T>() where T : class => Instance.Get<T>();
    }

    class IocContainer {
        IDictionary<Type, IocEntry> m_IocTable = new Dictionary<Type, IocEntry>();

        public IocContainer Register<T>(T input) where T : class {
            m_IocTable[typeof(T)] = new IocEntry {
                ValueReference = input,
            };
            return this;
        }

        public IocContainer Register<T>(Func<T> input) where T : class {
            m_IocTable[typeof(T)] = new IocEntry {
                Generator = input,
            };
            return this;
        }

        public IocContainer Register<T>() where T : class => Register(Activator.CreateInstance<T>());

        public bool Registered<T>() => m_IocTable.ContainsKey(typeof(T));

        public T Get<T>(bool initIfNull = true) where T : class {
            if (!Registered<T>()) {
                if (!initIfNull) {
                    return default(T);
                }
                Register<T>();
                return (T)m_IocTable[typeof(T)].ValueReference;
            }
            var tuple = m_IocTable[typeof(T)];
            if (tuple.ValueReference != null) {
                return (T)tuple.ValueReference;
            }
            if (tuple.Generator != null) {
                return ((Func<T>)tuple.Generator)();
            }
            throw new ContainerBadEntryException();
        }

        public void Lock() => m_IocTable = new ReadOnlyDictionary<Type, IocEntry>(m_IocTable);
    }
}
