
using System;


namespace MonoBridgeTest {
    public class FileIOLib : IDisposable {
        public FileIOLib() {
            System.Console.WriteLine("FileIOLib created");
        }

        public void Dispose() {
            System.Console.WriteLine("FileIOLib disposed");
        }

    }
}
