

using System;

namespace MonoBridgeTest {
    public class LargeMem : IDisposable {
        int [] large_data;

        public LargeMem() {
            large_data = new int[10000];
            Random r = new Random();
            for (int i=0; i<large_data.Length; i++) {
                large_data[i] = r.Next();
            }
        }
        public void Write(string s) {

        }
        public void Dispose() {
            large_data = null;
        }
    }
}
