param([Parameter(Mandatory)][string]$BinaryPath,
      [Parameter(Mandatory)][string]$TokenFile,
      [string]$Tools = "../../dosbox-agent-tools")
# Set POWERSHELL_TELEMETRY_OPTOUT=1 in the parent before launching pwsh.
# Primary DOSBox config must mount build/ as C: and allow that mount path.
Import-Module (Join-Path $PSScriptRoot "$Tools/dosbox-agent-tools.psd1")
$s = Start-DosboxSession -BinaryPath $BinaryPath -TokenFile $TokenFile
try {
    Invoke-DosboxCommand -Command "TESTDOS" -Session $s
    Wait-DosboxScreenText -Match "S2ALLDONE" -TimeoutSec 120 -Session $s | Out-Null
} finally {
    Stop-DosboxSession -Session $s
}
