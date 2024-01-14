param([string]$projectDir, [string]$outputPath);

# some logging
Write-Host "ProjectDir = $projectDir"
Write-Host "OutputPath = $outputPath"

# 1. Copy dlls to output directory
Write-Host "Copying dlls to output dir..."
Copy-Item -Path "$projectDir\dependencies\dlls\*" -Destination "$outputPath" -Force -Recurse # this \* will make the .dll files, not the folder.

# 2. Copy resources to output directory
Write-Host "Copying assets to output dir..."
Copy-Item -Path "$projectDir\assets" -Destination "$outputPath" -Force -Recurse # copies assets folder

# 3. Copy shaders to output directory
Write-Host "Copying shaders to output dir..."
Copy-Item -Path "$projectDir\shaders" -Destination "$outputPath" -Force -Recurse # copies shaders folder

Write-Host "DONE."