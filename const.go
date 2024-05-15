package tinycdb

import "fmt"

func NotFoundE(filename string) error {
	return fmt.Errorf("%s not found", filename)
}
