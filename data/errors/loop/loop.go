package main

import (
	"fmt"
)

func main() {

	
	fmt.Print("Content-Type: text/html\n");
	fmt.Print("<h1>This return error</h1>");

	for {
		fmt.Print("looping...");
	}
}