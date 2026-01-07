using System;
using System.Diagnostics;
using System.Runtime.InteropServices;

namespace MKXLTrainer.Core
{
    public class MemoryReader
    {
        // Import Windows API functions
        [DllImport("kernel32.dll")]
        public static extern IntPtr OpenProcess(int dwDesiredAccess, bool bInheritHandle, int dwProcessId);

        [DllImport("kernel32.dll")]
        public static extern bool ReadProcessMemory(IntPtr hProcess, IntPtr lpBaseAddress, byte[] lpBuffer, int dwSize, out IntPtr lpNumberOfBytesRead);

        [DllImport("kernel32.dll")]
        public static extern bool CloseHandle(IntPtr hObject);

        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern IntPtr GetProcAddress(IntPtr hModule, string lpProcName);

        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern IntPtr LoadLibrary(string lpFileName);

        // Constants
        private const int PROCESS_VM_READ = 0x0010;
        private const int PROCESS_VM_WRITE = 0x0020;
        private const int PROCESS_VM_OPERATION = 0x0008;

        private Process? _process;
        private IntPtr _processHandle;
        private readonly int[] _offsets = { 0x8, 0x130, 0x108, 0x78, 0x90, 0x120, 0xF20 };
        private readonly IntPtr _baseAddress;

        public MemoryReader(string processName, IntPtr baseAddress)
        {
            ProcessName = processName;
            _baseAddress = baseAddress;
            FindProcess();
        }

        public string ProcessName { get; }
        public bool IsProcessFound => _process != null && !_process.HasExited;

        public event Action<int>? OnFrameTimerRead;

        public bool FindProcess()
        {
            var processes = Process.GetProcessesByName(ProcessName);
            if (processes.Length > 0)
            {
                _process = processes[0];
                _processHandle = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, false, _process.Id);
                return _processHandle != IntPtr.Zero;
            }
            return false;
        }

        public int ReadFrameTimer()
        {
            if (!IsProcessFound || _processHandle == IntPtr.Zero)
            {
                FindProcess();
                if (!IsProcessFound || _processHandle == IntPtr.Zero)
                    return -1;
            }

            // Start with the base address: MK10.exe + 0x33C8A98
            IntPtr currentAddress;
            if (_process?.MainModule != null)
            {
                currentAddress = IntPtr.Add(_process.MainModule.BaseAddress, 0x33C8A98);
            }
            else
            {
                return -1;
            }

            // Apply offsets step by step: 8, 130, 108, 78, 90, 120, F20
            for (int i = 0; i < _offsets.Length - 1; i++)
            {
                byte[] buffer = new byte[8]; // Read 8 bytes for a pointer
                IntPtr bytesRead;
                
                if (!ReadProcessMemory(_processHandle, currentAddress, buffer, 8, out bytesRead) || bytesRead == IntPtr.Zero)
                {
                    return -1;
                }

                // Read the pointer value (assuming it's a 64-bit pointer)
                long pointerValue = BitConverter.ToInt64(buffer, 0);
                if (pointerValue == 0)
                {
                    return -1; // Null pointer
                }

                currentAddress = (IntPtr)pointerValue;
                currentAddress = IntPtr.Add(currentAddress, _offsets[i]);
            }

            // Apply the last offset to get to the final value address
            currentAddress = IntPtr.Add(currentAddress, _offsets[_offsets.Length - 1]);

            // Now read the actual value at the final address
            byte[] valueBuffer = new byte[4];
            IntPtr bytesRead2;
            
            if (!ReadProcessMemory(_processHandle, currentAddress, valueBuffer, 4, out bytesRead2) || bytesRead2 == IntPtr.Zero)
            {
                return -1;
            }

            int value = BitConverter.ToInt32(valueBuffer, 0);
            OnFrameTimerRead?.Invoke(value);
            return value;
        }

        public void Dispose()
        {
            if (_processHandle != IntPtr.Zero)
            {
                CloseHandle(_processHandle);
                _processHandle = IntPtr.Zero;
            }
        }
    }
}