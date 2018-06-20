using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;

namespace Knife {
    static class Extension {
        public static U Cast<T, U>(this T obj) {
            return (U) Convert.ChangeType(obj, typeof(U));
        }

        public static T With<T>(this T obj, Action<T> fn) {
            fn(obj);
            return obj;
        }
        public static U With<T, U>(this T obj, Func<T, U> fn) {
            return fn(obj);
        }
        public static Stream ToMemoryStream(this byte[] b) {
            return new MemoryStream(b);
        }
        public static string ToRawString(this byte[] b) {
            return Encoding.UTF8.GetString(b);
        }
        public static byte[] ToByteArray(this string str) {
            return Encoding.UTF8.GetBytes(str);
        }
        public static byte[] ToRawByteArray(this string str) {
            return str.Select(c => (byte) c).ToArray();
        }

        public static string FromBase64String(this string src) {
            return Convert.FromBase64String(src).ToString();
        }
        public static string ToBase64String(this string src) {
            return Convert.ToBase64String(src.ToByteArray());
        }
        public static void Each<T>(this IEnumerable<T> e, Action<T> fn) {
            foreach (var t in e) {
                fn(t);
            }
        }

        public static void Each<T>(this ICollection<T> e, Action<T> fn) {
            foreach (var t in e) {
                fn(t);
            }
        }

        public static bool IsNullOrEmpty(this string src) {
            return string.IsNullOrEmpty(src);
        }
    }
}

