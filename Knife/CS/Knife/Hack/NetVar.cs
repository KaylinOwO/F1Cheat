using System.Collections.Generic;
using Knife.Classes;
using Knife.Structs;

namespace Knife.Hack {
    unsafe class NetVar {
        // table name -> field name -> offset
        public Dictionary<string, Dictionary<string, int>> ManagedNetVar { get; set; }
            = new Dictionary<string, Dictionary<string, int>>();

        void DumpTable(RecvTable table) {
            if (ManagedNetVar.ContainsKey(table.TableName)) {
                return;
            }
            ManagedNetVar[table.TableName] = new Dictionary<string, int>();
            for (var i = 0; i < table.PropertiesCount; i++) {
                var prop = table.Properties[i];

                if (!ManagedNetVar[table.TableName].ContainsKey(prop.VariableName)) {
                    ManagedNetVar[table.TableName][prop.VariableName] = prop.Offset;
                }

                if (prop.ChildDataTable != null) {
                    DumpTable(*prop.ChildDataTable);
                }
            }
        }

        public void Init() {
            for (var it = Ioc.Get<HLClient>().AllClasses; it != null; it = (*it).Next) {
                var clientClass = *it;
                DumpTable(*clientClass.ReceiveTable);
            }
#if false
            if (File.Exists("netvar.json")) {
                File.Delete("netvar.json");

                var sorted = new SortedDictionary<string, SortedDictionary<string, int>>();
                foreach (var kv in ManagedNetVar) {
                    if (!sorted.ContainsKey(kv.Key)) {
                        sorted[kv.Key] = new SortedDictionary<string, int>();
                    }
                    foreach (var kv1 in kv.Value) {
                        sorted[kv.Key].Add(kv1.Key, kv1.Value);
                    }
                }
                File.WriteAllText("netvar.json", JsonConvert.SerializeObject(sorted, Formatting.Indented, new HexConverter()));
            }
#endif
        }

        public Dictionary<string, int> this[string id] => ManagedNetVar[id];
    }
}
#if false
class HexConverter : JsonConverter {
    public override void WriteJson(JsonWriter writer, object value, JsonSerializer serializer) => writer.WriteValue($"0x{(int)value:X}");
    public override object ReadJson(JsonReader reader, Type objectType, object existingValue, JsonSerializer serializer) {
        throw new NotImplementedException();
    }
    public override bool CanConvert(Type objectType) => objectType == typeof(int);
}
#endif
