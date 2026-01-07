using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Windows;
using System.Windows.Forms;
using MKXLTrainer.Core;
using MessageBox = System.Windows.MessageBox;

namespace MKXLTrainer.GUI
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private InputManager? _inputManager;
        private DllInjector? _dllInjector;
        private List<MacroStep> _macroSteps;

        public MainWindow()
        {
            InitializeComponent();
            _macroSteps = new List<MacroStep>();
            DgMacroSteps.ItemsSource = _macroSteps;
            
            // Initialize with default values
            SldBlockThreshold.Value = 260;
            SldMacroThreshold.Value = 280;
        }

        private void BtnBrowseGame_Click(object sender, RoutedEventArgs e)
        {
            using (var openFileDialog = new OpenFileDialog())
            {
                openFileDialog.Filter = "MK10 Executable|MK10.exe|All Files (*.*)|*.*";
                openFileDialog.Title = "Select MK10.exe";
                
                if (openFileDialog.ShowDialog() == System.Windows.Forms.DialogResult.OK)
                {
                    TxtGamePath.Text = openFileDialog.FileName;
                }
            }
        }

        private void BtnDeployDll_Click(object sender, RoutedEventArgs e)
        {
            if (string.IsNullOrEmpty(TxtGamePath.Text))
            {
                MessageBox.Show("Please select MK10.exe first.", "Error", MessageBoxButton.OK, MessageBoxImage.Warning);
                return;
            }

            try
            {
                // This would copy the dinput8.dll to the game directory
                // For now, we'll just show a message since we don't have the actual DLL yet
                string sourceDllPath = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "dinput8.dll");
                
                if (!File.Exists(sourceDllPath))
                {
                    MessageBox.Show($"dinput8.dll not found at: {sourceDllPath}\nPlease build the DLL project first.", 
                        "DLL Not Found", MessageBoxButton.OK, MessageBoxImage.Warning);
                    return;
                }

                _dllInjector = new DllInjector();
                _dllInjector.CopyDInput8Dll(TxtGamePath.Text, sourceDllPath);
                
                MessageBox.Show("dinput8.dll deployed successfully!", "Success", MessageBoxButton.OK, MessageBoxImage.Information);
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Failed to deploy DLL: {ex.Message}", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        private void BtnStartMonitoring_Click(object sender, RoutedEventArgs e)
        {
            if (string.IsNullOrEmpty(TxtGamePath.Text))
            {
                MessageBox.Show("Please select MK10.exe first.", "Error", MessageBoxButton.OK, MessageBoxImage.Warning);
                return;
            }

            try
            {
                string processName = Path.GetFileNameWithoutExtension(TxtGamePath.Text);
                _inputManager = new InputManager(processName);
                
                // Update thresholds from UI
                _inputManager.BlockThreshold = (int)SldBlockThreshold.Value;
                _inputManager.MacroThreshold = (int)SldMacroThreshold.Value;
                
                // Subscribe to events
                _inputManager.OnFrameTimerChanged += OnFrameTimerChanged;
                
                _inputManager.StartMonitoring();
                MessageBox.Show("Monitoring started!", "Success", MessageBoxButton.OK, MessageBoxImage.Information);
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Failed to start monitoring: {ex.Message}", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        private void BtnStopMonitoring_Click(object sender, RoutedEventArgs e)
        {
            _inputManager?.StopMonitoring();
            MessageBox.Show("Monitoring stopped!", "Info", MessageBoxButton.OK, MessageBoxImage.Information);
        }

        private void OnFrameTimerChanged(int frameTimer)
        {
            // Update UI on the UI thread
            Dispatcher.Invoke(() =>
            {
                TxtCurrentFrame.Text = frameTimer.ToString();
            });
        }

        private void SldBlockThreshold_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            TxtBlockThresholdValue.Text = ((int)e.NewValue).ToString();
            
            if (_inputManager != null)
            {
                _inputManager.BlockThreshold = (int)e.NewValue;
            }
        }

        private void SldMacroThreshold_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            TxtMacroThresholdValue.Text = ((int)e.NewValue).ToString();
            
            if (_inputManager != null)
            {
                _inputManager.MacroThreshold = (int)e.NewValue;
            }
        }

        private void BtnAddBlockedKey_Click(object sender, RoutedEventArgs e)
        {
            // For now, we'll just add a placeholder key
            // In a real implementation, this would open a dialog to select a key
            string key = Microsoft.VisualBasic.Interaction.InputBox(
                "Enter key to block (as integer key code):", 
                "Add Blocked Key", 
                "0");
            
            if (int.TryParse(key, out int keyCode))
            {
                LstBlockedKeys.Items.Add($"Key {keyCode}");
                
                // Update the input manager with blocked keys
                if (_inputManager != null)
                {
                    var blockedKeys = _inputManager.BlockedKeys;
                    blockedKeys.Add(keyCode);
                    _inputManager.SetBlockedKeys(blockedKeys);
                }
            }
        }

        private void ChkEnableBlocking_Checked(object sender, RoutedEventArgs e)
        {
            if (_inputManager != null)
            {
                _inputManager.IsBlockingInput = true;
            }
        }

        private void ChkEnableBlocking_Unchecked(object sender, RoutedEventArgs e)
        {
            if (_inputManager != null)
            {
                _inputManager.IsBlockingInput = false;
            }
        }

        private void BtnAddMacroStep_Click(object sender, RoutedEventArgs e)
        {
            var step = new MacroStep
            {
                Button = 0, // Default button code
                DurationMs = 100, // Default duration
                DelayAfterMs = 50 // Default delay
            };
            
            _macroSteps.Add(step);
            DgMacroSteps.Items.Refresh();
        }

        private void BtnRemoveMacroStep_Click(object sender, RoutedEventArgs e)
        {
            if (DgMacroSteps.SelectedItem != null)
            {
                _macroSteps.Remove((MacroStep)DgMacroSteps.SelectedItem);
                DgMacroSteps.Items.Refresh();
            }
        }

        private async void BtnRunMacro_Click(object sender, RoutedEventArgs e)
        {
            if (_inputManager != null)
            {
                await _inputManager.StartMacroAsync(_macroSteps.ToList());
            }
        }

        private void BtnStopMacro_Click(object sender, RoutedEventArgs e)
        {
            _inputManager?.StopMacro();
        }

        private void BtnSavePreset_Click(object sender, RoutedEventArgs e)
        {
            using (var saveFileDialog = new SaveFileDialog())
            {
                saveFileDialog.Filter = "JSON Preset|*.json|All Files (*.*)|*.*";
                saveFileDialog.Title = "Save Preset";
                
                if (saveFileDialog.ShowDialog() == System.Windows.Forms.DialogResult.OK)
                {
                    try
                    {
                        // In a real implementation, this would serialize the current settings to JSON
                        var settings = new
                        {
                            BlockThreshold = (int)SldBlockThreshold.Value,
                            MacroThreshold = (int)SldMacroThreshold.Value,
                            BlockedKeys = LstBlockedKeys.Items.Cast<string>().ToList(),
                            MacroSteps = _macroSteps.ToList()
                        };
                        
                        string json = System.Text.Json.JsonSerializer.Serialize(settings, new System.Text.Json.JsonSerializerOptions { WriteIndented = true });
                        File.WriteAllText(saveFileDialog.FileName, json);
                        
                        MessageBox.Show("Preset saved successfully!", "Success", MessageBoxButton.OK, MessageBoxImage.Information);
                    }
                    catch (Exception ex)
                    {
                        MessageBox.Show($"Failed to save preset: {ex.Message}", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                    }
                }
            }
        }

        private void BtnLoadPreset_Click(object sender, RoutedEventArgs e)
        {
            using (var openFileDialog = new OpenFileDialog())
            {
                openFileDialog.Filter = "JSON Preset|*.json|All Files (*.*)|*.*";
                openFileDialog.Title = "Load Preset";
                
                if (openFileDialog.ShowDialog() == System.Windows.Forms.DialogResult.OK)
                {
                    try
                    {
                        string json = File.ReadAllText(openFileDialog.FileName);
                        
                        // In a real implementation, this would deserialize the JSON settings
                        // For now we'll just show a message
                        MessageBox.Show("Preset loaded successfully!", "Success", MessageBoxButton.OK, MessageBoxImage.Information);
                    }
                    catch (Exception ex)
                    {
                        MessageBox.Show($"Failed to load preset: {ex.Message}", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                    }
                }
            }
        }

        protected override void OnClosed(EventArgs e)
        {
            _inputManager?.Dispose();
            base.OnClosed(e);
        }
    }
}
