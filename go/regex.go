package main

import (
	"fmt"
	"regexp"
)

func main() {
	prefix := "IPC"

	pattern := fmt.Sprintf(`^%s\.(.*)\.log$`, prefix)
	filenameRegex, err := regexp.Compile(pattern)
	if err != nil {
		fmt.Println("error")
		return
	}

	str := "IPC.2014-10-20T10:35:47.882206159-07:00.log"
	b := filenameRegex.FindStringSubmatch(str)

	fmt.Println(b)
}
