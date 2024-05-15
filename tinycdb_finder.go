package tinycdb

import (
	"fmt"
)

func (c *CDb) Finder(file string) error {
	if !c.Ok() {
		return c.Warp()
	}

	c.file = file
	finder := c.read(file)
	if finder == 0 {
		c.Expect("%s c reader failed", c.lib)
		return c.Warp()
	}

	c.finder = finder
	return c.Warp()

}

func (c *CDb) Get(key string) (string, error) {
	if !c.Ok() {
		return "", c.Warp()
	}

	nk := uint(len(key))

	var elem string
	ret := c.find(c.finder, key, nk, func(v uintptr, size uint) uint {
		if size == 0 {
			elem = ""
		} else {
			elem = clone(v, size)

		}
		return 1
	})

	if ret < 0 {
		return "", fmt.Errorf("内部错误")
	}
	return elem, nil
}

func (c *CDb) GetAll(key string) ([]string, error) {
	if !c.Ok() {
		return nil, c.Warp()
	}

	nk := uint(len(key))

	var arr []string
	ret := c.findAll(c.finder, key, nk, func(v uintptr, size uint) uint {
		if size == 0 {
			arr = append(arr, "")
		} else {
			item := clone(v, size)
			arr = append(arr, item)
		}
		return 0
	})

	if ret < 0 {
		return nil, fmt.Errorf("内部错误")
	}
	return arr, nil
}

func (c *CDb) Foreach(callback func(k, v string) (stop bool)) error {
	if !c.Ok() {
		return c.Warp()
	}

	ret := c.foreach(c.finder, func(k uintptr, kl uint, v uintptr, vl uint) uint {
		key := clone(k, kl)
		val := clone(v, vl)
		stop := callback(key, val)
		if stop {
			return 1
		}
		return 0
	})

	if ret < 0 {
		return fmt.Errorf("内部错误")
	}
	return nil
}

func (c *CDb) Release() error {
	if !c.Ok() {
		return c.Warp()
	}

	c.release(c.finder)
	return nil
}
