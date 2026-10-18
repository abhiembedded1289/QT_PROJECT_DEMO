param(
    [Parameter(Mandatory = $true)]
    [string]$Name
)

$projectFolder = Join-Path (Get-Location) $Name

if (Test-Path $projectFolder) {
    Write-Error "Folder '$Name' already exists."
    exit 1
}

New-Item -ItemType Directory -Path $projectFolder -Force | Out-Null

$mainCpp = @'
#include <QApplication>
#include <QLabel>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QLabel label("Hello from Qt practice!");
    label.resize(260, 80);
    label.setAlignment(Qt::AlignCenter);
    label.show();
    return app.exec();
}
'@

$cmake = @"
cmake_minimum_required(VERSION 3.16)
project($($Name.Replace('-', '_')) LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable($($Name.Replace('-', '_')) main.cpp)
target_link_libraries($($Name.Replace('-', '_')) PRIVATE Qt6::Widgets)
"@

Set-Content -Path (Join-Path $projectFolder 'main.cpp') -Value $mainCpp
Set-Content -Path (Join-Path $projectFolder 'CMakeLists.txt') -Value $cmake

Write-Host "Created project: $projectFolder"
Write-Host "Build with: cmake -S . -B build"
Write-Host "Then: cmake --build build"
