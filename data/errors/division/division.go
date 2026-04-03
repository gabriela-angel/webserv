package main

import "fmt"

func main() {
    fmt.Print("Content-Type: text/html\r\n")
    fmt.Print("\r\n")
    fmt.Print("<h1>This will panic</h1>")
    
    var divisor int = 0
    result := 42 / divisor
    fmt.Print(result)
}