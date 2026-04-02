package main

import (
    "fmt"
    "os"
)

func main() {
    fmt.Print("Content-Type: text/html\r\n")
    fmt.Print("\r\n")
    fmt.Print("<h1>This script exits with error</h1>")
    os.Exit(1)
}