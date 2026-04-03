package main

import (
	"fmt"
	"os"
    "strconv"
    "strings"
)



func main() {
	enemies_killed := 0

	cookie := os.Getenv("HTTP_COOKIE")
	if cookie != "" {
        pairs := strings.Split(cookie, "; ")
        for _, pair := range pairs {
            parts := strings.SplitN(pair, "=", 2)
            if len(parts) == 2 && parts[0] == "enemies_killed" {
                if count, err := strconv.Atoi(parts[1]); err == nil {
                    enemies_killed = count
                }
                break
            }
        }
    }

	requestMethod := os.Getenv("REQUEST_METHOD")
    if requestMethod == "POST" {
        enemies_killed++
    }

	fmt.Printf("Set-Cookie: enemies_killed=%d; Path=/; Max-Age=31536000\n", enemies_killed)
    fmt.Println("Content-Type: text/html")
    fmt.Println()
	
	html:= fmt.Sprintf(`<!DOCTYPE html>
<html>
	<head>
		<title>Deadly Mode</title>
		<link rel="stylesheet" href="/style.css">
	</head>

	<body class="bg-deadly center-container">
		<p class="title">Kill Enemy</p>
		<p class="subtitle">%d Enemies Killed</p>

		<form method="POST" action="/deadly/deadly.go">
			<button class="btn" type="submit">Click to kill enemy</button>
		</form>
	</body>
</html>`, enemies_killed)
	
	fmt.Print(html)
	
}
