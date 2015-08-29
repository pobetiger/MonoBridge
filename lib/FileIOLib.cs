
using System;


namespace MonoBridgeTest {
    public class FileIOLib : IDisposable {

        string myport;
        string mydesc;

        public FileIOLib() {
            System.Console.WriteLine("FileIOLib created");
            myport = "unknown";
            mydesc = "default";
        }

        public void Setup(string port) {

            System.Console.WriteLine(string.Format("Setup to use port: {0}", port));
            myport = port;

        }

        public void Setup(string port, string desc) {
            System.Console.WriteLine(string.Format("Setup to use port: {0} for {1}", port, desc));
            myport = port;
            mydesc = desc;
        }


        public void Dispose() {
            System.Console.WriteLine(string.Format("FileIOLib port={0} disposed", myport));
        }

    }
}
