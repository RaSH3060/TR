using System;
using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;

namespace MKXLTrainer.Core
{
    public class DllInjector
    {
        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern IntPtr OpenProcess(uint processAccess, bool bInheritHandle, int processId);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern IntPtr VirtualAllocEx(IntPtr hProcess, IntPtr lpAddress, uint dwSize, uint flAllocationType, uint flProtect);

        [DllImport("kernel32.dll")]
        private static extern bool WriteProcessMemory(IntPtr hProcess, IntPtr lpBaseAddress, byte[] lpBuffer, uint nSize, out UIntPtr lpNumberOfBytesWritten);

        [DllImport("kernel32.dll")]
        private static extern IntPtr GetProcAddress(IntPtr hModule, string lpProcName);

        [DllImport("kernel32.dll")]
        private static extern IntPtr LoadLibrary(string lpFileName);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern IntPtr CreateRemoteThread(IntPtr hProcess, IntPtr lpThreadAttributes, uint dwStackSize, IntPtr lpStartAddress, IntPtr lpParameter, uint dwCreationFlags, out uint lpThreadId);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern bool CloseHandle(IntPtr hObject);

        private const uint PROCESS_CREATE_THREAD = 0x0002;
        private const uint PROCESS_QUERY_INFORMATION = 0x0400;
        private const uint PROCESS_VM_OPERATION = 0x0008;
        private const uint PROCESS_VM_WRITE = 0x0020;
        private const uint PROCESS_VM_READ = 0x0010;

        private const uint MEM_COMMIT = 0x00001000;
        private const uint MEM_RESERVE = 0x00002000;
        private const uint PAGE_READWRITE = 0x04;

        public bool InjectDll(string processName, string dllPath)
        {
            if (!File.Exists(dllPath))
            {
                throw new FileNotFoundException($"DLL file not found: {dllPath}");
            }

            var processes = Process.GetProcessesByName(processName);
            if (processes.Length == 0)
            {
                throw new InvalidOperationException($"Process {processName} not found");
            }

            IntPtr processHandle = IntPtr.Zero;
            IntPtr memoryAddress = IntPtr.Zero;

            try
            {
                Process process = processes[0];
                processHandle = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, false, process.Id);

                if (processHandle == IntPtr.Zero)
                {
                    throw new InvalidOperationException("Could not open process");
                }

                // Allocate memory in the target process for the DLL path
                int size = (dllPath.Length + 1) * Marshal.SizeOf(typeof(char));
                memoryAddress = VirtualAllocEx(processHandle, IntPtr.Zero, (uint)size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

                if (memoryAddress == IntPtr.Zero)
                {
                    throw new InvalidOperationException("Could not allocate memory in target process");
                }

                // Write the DLL path to the allocated memory
                byte[] dllPathBytes = Encoding.Unicode.GetBytes(dllPath);
                if (!WriteProcessMemory(processHandle, memoryAddress, dllPathBytes, (uint)dllPathBytes.Length, out _))
                {
                    throw new InvalidOperationException("Could not write DLL path to target process memory");
                }

                // Get the address of LoadLibraryW function
                IntPtr kernel32Handle = LoadLibrary("kernel32.dll");
                IntPtr loadLibraryAddress = GetProcAddress(kernel32Handle, "LoadLibraryW");

                // Create a remote thread in the target process that calls LoadLibraryW with our DLL path
                IntPtr threadHandle = CreateRemoteThread(processHandle, IntPtr.Zero, 0, loadLibraryAddress, memoryAddress, 0, out uint _);

                if (threadHandle == IntPtr.Zero)
                {
                    throw new InvalidOperationException("Could not create remote thread");
                }

                // Wait for the thread to complete
                // We could use WaitForSingleObject here, but for simplicity we'll just close the handle
                CloseHandle(threadHandle);

                return true;
            }
            finally
            {
                if (memoryAddress != IntPtr.Zero)
                {
                    // Free the allocated memory
                    VirtualFreeEx(processHandle, memoryAddress, 0, 0x8000); // MEM_RELEASE
                }

                if (processHandle != IntPtr.Zero)
                {
                    CloseHandle(processHandle);
                }

                processes[0].Dispose();
            }
        }

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern bool VirtualFreeEx(IntPtr hProcess, IntPtr lpAddress, uint dwSize, uint dwFreeType);

        public bool CopyDInput8Dll(string mk10ExePath, string dllSourcePath)
        {
            try
            {
                string targetDir = Path.GetDirectoryName(mk10ExePath);
                if (string.IsNullOrEmpty(targetDir))
                {
                    throw new ArgumentException("Invalid executable path", nameof(mk10ExePath));
                }

                string targetDllPath = Path.Combine(targetDir, "dinput8.dll");
                
                // Copy the DLL to the game directory
                File.Copy(dllSourcePath, targetDllPath, true);
                
                return true;
            }
            catch (Exception ex)
            {
                throw new InvalidOperationException($"Failed to copy dinput8.dll: {ex.Message}", ex);
            }
        }
    }
}