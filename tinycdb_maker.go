package tinycdb

import (
	"errors"
	"fmt"
	"github.com/ebitengine/purego"
	"strings"
)

const (
	Put_Add = iota
	Put_Replace
	Put_Insert
	Put_Warn
	Put_Replace0
)

type CDb struct {
	shared uintptr
	lib    string
	errs   []error
	file   string

	finder  uintptr
	read    func(string) uintptr
	find    func(uintptr, string, uint, func(uintptr, uint) uint) int
	findAll func(uintptr, string, uint, func(uintptr, uint) uint) int
	foreach func(uintptr, func(uintptr, uint, uintptr, uint) uint) int
	release func(uintptr) int

	// cdb maker
	maker  uintptr
	create func(string) uintptr
	add    func(uintptr, string, uint, string, uint) int
	put    func(uintptr, string, uint, string, uint, uint) int
	finish func(uintptr) int
	free   func(uintptr) int
}

func New(lib string) (*CDb, error) {
	cdb := &CDb{lib: lib}

	err := cdb.open()
	if err != nil {
		return nil, err
	}

	return cdb, nil
}
func (c *CDb) Ok() bool {
	return len(c.errs) == 0
}

func (c *CDb) Expect(format string, v ...interface{}) {
	c.errs = append(c.errs, fmt.Errorf(format, v...))
}

func (c *CDb) Warp() error {
	if c.Ok() {
		return nil
	}

	builder := &strings.Builder{}
	for _, err := range c.errs {
		builder.WriteString(err.Error())
		builder.WriteByte('\n')
	}
	return errors.New(builder.String())
}

func (c *CDb) open() error {
	if len(c.lib) == 0 {
		c.Expect("empty lib")
		return c.Warp()
	}

	ptr, err := purego.Dlopen(c.lib, purego.RTLD_NOW|purego.RTLD_GLOBAL)
	if err != nil {
		c.Expect("load %s fail %v", c.lib, err)
		return c.Warp()
	}

	defer func() {
		if e := recover(); e != nil {
			c.Expect("load %s panic %v", c.lib, e)
			c.Expect("closed %v", purego.Dlclose(ptr))
		}
	}()

	c.shared = ptr
	purego.RegisterLibFunc(&c.create, ptr, "cdb_make_start_go")
	purego.RegisterLibFunc(&c.add, ptr, "cdb_make_add_go")
	purego.RegisterLibFunc(&c.put, ptr, "cdb_make_put_go")
	purego.RegisterLibFunc(&c.free, ptr, "cdb_make_free_go")
	purego.RegisterLibFunc(&c.finish, ptr, "cdb_make_finish_go")
	purego.RegisterLibFunc(&c.read, ptr, "cdb_find_start_go")
	purego.RegisterLibFunc(&c.find, ptr, "cdb_find_go")
	purego.RegisterLibFunc(&c.findAll, ptr, "cdb_find_all_go")
	purego.RegisterLibFunc(&c.foreach, ptr, "cdb_foreach_go")
	purego.RegisterLibFunc(&c.release, ptr, "cdb_find_close")
	return nil
}

func (c *CDb) Finish() error {
	if !c.Ok() {
		return c.Warp()
	}

	c.finish(c.maker)
	return nil
}

func (c *CDb) Free() error {
	if !c.Ok() {
		return c.Warp()
	}

	c.free(c.maker)
	return nil
}

func (c *CDb) Maker(file string) error {
	if !c.Ok() {
		return c.Warp()
	}

	c.file = file
	maker := c.create(file)
	if maker == 0 {
		c.Expect("%s c maker failed", c.lib)
		return c.Warp()
	}

	c.maker = maker
	return c.Warp()
}

func (c *CDb) Add(key string, val string) error {
	if !c.Ok() {
		return c.Warp()
	}

	nk := uint(len(key))
	nv := uint(len(val))
	c.add(c.maker, key, nk, val, nv)
	return nil
}

func (c *CDb) Put(key string, val string, flag int) error {
	if !c.Ok() {
		return c.Warp()
	}

	nk := uint(len(key))
	nv := uint(len(val))
	c.put(c.maker, key, nk, val, nv, uint(flag))
	return nil
}
