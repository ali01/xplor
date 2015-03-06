package main

import (
	"fmt"
)

func main() {
	var fnptr func(string) string
	fnptr = func(name string) string {
		return fmt.Sprintf("huzza! %s", name)
	}

	if fnptr == nil {
		fmt.Println("hello world")
	} else {
		fmt.Println(fnptr("hello world"))
	}
}
