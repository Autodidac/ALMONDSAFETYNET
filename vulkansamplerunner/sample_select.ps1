Add-Type -AssemblyName "System.Windows.Forms"

# Define the list of options
$options = @(
    "hdr", "dynamic_uniform_buffers", "fullscreen_triangle", "screen_texture", "screen_texel_size", "last_time_step",
    "gamma", "frustum_planes", "matrix_projection", "matrix_view", "view_position", "input_state", "time_step",
    "time_since_start", "render_flags", "camera_position", "camera_direction", "camera_up", "camera_near_clip",
    "camera_far_clip", "camera_aspect_ratio", "camera_fov", "camera_lookat"
)

# Create and configure the form
$form = New-Object System.Windows.Forms.Form
$form.Text = "Select Option"
$form.Size = New-Object System.Drawing.Size(400, 200)
$form.StartPosition = "CenterScreen"

# Create and configure the label
$label = New-Object System.Windows.Forms.Label
$label.Text = "Please select an option:"
$label.AutoSize = $true
$label.Location = New-Object System.Drawing.Point(10, 20)
$form.Controls.Add($label)

# Create and configure the combo box
$comboBox = New-Object System.Windows.Forms.ComboBox
$comboBox.Location = New-Object System.Drawing.Point(10, 50)
$comboBox.Size = New-Object System.Drawing.Size(360, 30)
$comboBox.DropDownStyle = "DropDownList"
$comboBox.Items.AddRange($options)
$form.Controls.Add($comboBox)

# Create and configure the OK button
$okButton = New-Object System.Windows.Forms.Button
$okButton.Text = "OK"
$okButton.Location = New-Object System.Drawing.Point(200, 120)
$okButton.DialogResult = [System.Windows.Forms.DialogResult]::OK
$form.Controls.Add($okButton)

# Create and configure the Cancel button
$cancelButton = New-Object System.Windows.Forms.Button
$cancelButton.Text = "Cancel"
$cancelButton.Location = New-Object System.Drawing.Point(300, 120)
$cancelButton.DialogResult = [System.Windows.Forms.DialogResult]::Cancel
$form.Controls.Add($cancelButton)

# Set default dialog button
$form.AcceptButton = $okButton
$form.CancelButton = $cancelButton

# Show the form and capture the result
$result = $form.ShowDialog()

# Handle the result
if ($result -eq [System.Windows.Forms.DialogResult]::OK -and $comboBox.SelectedItem -ne $null) {
    $selectedOption = $comboBox.SelectedItem
    Write-Host "You selected: $selectedOption"

    # Run the selected example
    $vulkanExecutable = ".\vulkan_samples.exe" # Path to the Vulkan samples executable
    $arguments = "sample $selectedOption"

    if (Test-Path $vulkanExecutable) {
        & $vulkanExecutable $arguments
    } else {
        Write-Host "Error: Vulkan samples executable not found at $vulkanExecutable"
    }
} elseif ($result -eq [System.Windows.Forms.DialogResult]::Cancel) {
    Write-Host "No option selected. Exiting."
} else {
    Write-Host "No option selected. Exiting."
}

Write-Host "Press Enter to continue..."
[Console]::ReadLine()
