using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;

namespace MKXLTrainer.Core
{
    public class InputManager
    {
        public event Action<int>? OnFrameTimerChanged;
        public event Action? OnMacroStarted;
        public event Action? OnMacroFinished;
        
        private readonly MemoryReader _memoryReader;
        private Timer? _memoryReadTimer;
        private bool _isBlockingInput = false;
        private bool _isMacroRunning = false;
        private readonly List<int> _blockedKeys = new List<int>();
        private int _blockThreshold = 260; // Default threshold in ms
        private int _macroThreshold = 280; // Default macro threshold in ms
        private List<MacroStep>? _currentMacro;
        private int _currentMacroStepIndex = 0;

        public InputManager(string processName)
        {
            _memoryReader = new MemoryReader(processName, IntPtr.Zero);
            _memoryReader.OnFrameTimerRead += OnFrameTimerRead;
        }

        public bool IsBlockingInput 
        { 
            get => _isBlockingInput; 
            set 
            { 
                _isBlockingInput = value;
                // Here we would communicate with the DLL to start/stop blocking
            } 
        }

        public List<int> BlockedKeys => new List<int>(_blockedKeys);
        
        public int BlockThreshold 
        { 
            get => _blockThreshold; 
            set => _blockThreshold = value; 
        }
        
        public int MacroThreshold 
        { 
            get => _macroThreshold; 
            set => _macroThreshold = value; 
        }

        public void SetBlockedKeys(List<int> keys)
        {
            _blockedKeys.Clear();
            _blockedKeys.AddRange(keys);
        }

        public void StartMonitoring()
        {
            // Read memory every 10ms as specified in the requirements
            _memoryReadTimer = new Timer(async (_) => await ReadMemoryAsync(), null, 0, 10);
        }

        public void StopMonitoring()
        {
            _memoryReadTimer?.Dispose();
            _memoryReadTimer = null;
        }

        private async Task ReadMemoryAsync()
        {
            try
            {
                int frameTimer = _memoryReader.ReadFrameTimer();
                if (frameTimer != -1)
                {
                    OnFrameTimerChanged?.Invoke(frameTimer);
                    
                    // Check if we should block input
                    if (frameTimer >= _blockThreshold && !_isBlockingInput)
                    {
                        IsBlockingInput = true;
                    }
                    else if (frameTimer < _blockThreshold && _isBlockingInput)
                    {
                        IsBlockingInput = false;
                    }
                    
                    // Check if we should start a macro
                    if (frameTimer >= _macroThreshold && !_isMacroRunning)
                    {
                        await StartMacroAsync();
                    }
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine($"Error reading memory: {ex.Message}");
            }
        }

        private void OnFrameTimerRead(int value)
        {
            // This is called from the MemoryReader when a value is read
            // The actual logic is handled in ReadMemoryAsync
        }

        public async Task StartMacroAsync(List<MacroStep>? macro = null)
        {
            if (_isMacroRunning) return;
            
            if (macro != null)
            {
                _currentMacro = macro;
            }
            
            if (_currentMacro == null || !_currentMacro.Any())
            {
                return;
            }

            _isMacroRunning = true;
            OnMacroStarted?.Invoke();
            
            _currentMacroStepIndex = 0;
            while (_currentMacroStepIndex < _currentMacro.Count && _isMacroRunning)
            {
                var step = _currentMacro[_currentMacroStepIndex];
                
                // Execute the macro step
                await ExecuteMacroStepAsync(step);
                
                // Wait for the delay after this step
                if (step.DelayAfterMs > 0)
                {
                    await Task.Delay(step.DelayAfterMs);
                }
                
                _currentMacroStepIndex++;
            }
            
            _isMacroRunning = false;
            OnMacroFinished?.Invoke();
        }

        private async Task ExecuteMacroStepAsync(MacroStep step)
        {
            // This would communicate with the DLL to simulate input
            // Since we're just defining the interface, the actual implementation
            // would be in the injected DLL
            Debug.WriteLine($"Executing macro step: {step.Button} for {step.DurationMs}ms");
            
            if (step.DurationMs > 0)
            {
                await Task.Delay(step.DurationMs);
            }
        }

        public void StopMacro()
        {
            _isMacroRunning = false;
        }

        public void Dispose()
        {
            StopMonitoring();
            _memoryReader.Dispose();
        }
    }

    public class MacroStep
    {
        public int Button { get; set; } // Button identifier (key code, etc.)
        public int DurationMs { get; set; } // How long to hold the button
        public int DelayAfterMs { get; set; } // Delay after releasing the button
    }
}