
using System;

namespace MonoBridgeTest {
    public class Logger : IDisposable {
        public Logger() {

        }
        public void Dispose() {

        }

        public void Log(string msg) {
            System.Console.WriteLine("* LOG: " + msg);
        }
    }
}
