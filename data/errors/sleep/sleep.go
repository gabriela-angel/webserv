package main

import (
    "fmt"
    "time"
)

func main() {
    fmt.Print("Content-Type: text/html\r\n")
    fmt.Print("\r\n")
    fmt.Print("<h1>Sleeping for 60 seconds...</h1>")
    time.Sleep(10 * time.Second)
    fmt.Print("<p>Done!</p>")
}