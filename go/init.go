package main

import "fmt"

type Test struct {
	a string
}

func NewTest() (*Test, error) {
	t := &Test{
		a: "hello world",
	}

	return t, nil
}

func (t *Test) init() error {
	fmt.Println("Test::init function called")
	t.a = "mine!"
	return nil
}

func init() {
	fmt.Println("init function called")
}

func main() {
	fmt.Println("main function called")
	t, _ := NewTest()
	t.init()
	fmt.Println(t.a)
}
