#!/bin/zsh

# COLORS
RED='\033[0;31m'
GREEN='\033[0;32m'
CYAN='\033[0;36m'
YELLOW='\033[0;33m'
NC='\033[0m'

if [[ $# -ne 1 ]]; then
	echo "${RED}Uso: $0 ${YELLOW}'<program_name>'${NC}"
	exit 1
fi

config='[
			{
				"description": "Seguir o processo pai",
				"text": "set follow-fork-mode parent",
				"ignoreFailures": true
			}
		]'

mkdir -p .vscode

cat > .vscode/tasks.json <<EOF
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "build",
            "type": "shell",
            "command": "make",
            "group": {
                "kind": "build",
                "isDefault": true
            }
        }
    ]
}
EOF

cat > .vscode/launch.json <<EOF
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug (make)",
            "type": "cppdbg",
            "request": "launch",
            "program": "\${workspaceFolder}/$1",
            "args": [],
            "stopAtEntry": false,
            "cwd": "\${workspaceFolder}",
            "environment": [],
            "externalConsole": false,
            "MIMode": "gdb",
            "setupCommands": $config,
            "miDebuggerPath": "/usr/bin/gdb",
            "preLaunchTask": "build",
            "miDebuggerArgs": "",
            "logging": {
                "moduleLoad": false,
                "trace": false,
                "engineLogging": false
            }
        }
    ]
}
EOF

echo "${CYAN}Arquivos .vscode/tasks.json e .vscode/launch.json criados!\n${NC}"