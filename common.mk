SRCS := $(shell find src|grep '\.c$$')
OBJS := $(SRCS:src/%.c=obj/%.o)

SHADERS := base.bin basic.bin
ASSETS := quad.bin camera.bin camera-albedo.bin

CC := $(CROSS_COMPILE)gcc
STRIP := $(CROSS_COMPILE)strip
GDB := $(CROSS_COMPILE)gdb

CFLAGS_COMMON := -Ivendor/bulletcapi/capi
CFLAGS_COMMON += -Iinclude -std=c2x -m64
CFLAGS_COMMON += -DLOG_USE_SDL

CFLAGS_DEBUG := $(CFLAGS_COMMON)
CFLAGS_DEBUG += -pedantic -Wall -Wextra
CFLAGS_DEBUG += -g3 -O0 -DDEBUG

CFLAGS_RELEASE := $(CFLAGS_COMMON)
CFLAGS_RELEASE += -Ofast
CFLAGS_RELEASE += -ftree-vectorize -ffast-math -funroll-loops

LDFLAGS_COMMON := -lSDL2 -lGLESv2 -lcglm
LDFLAGS_COMMON += -L. -Wl,-rpath,.
LDFLAGS_COMMON += -lcapi

all: $(O) $(SHADERS) $(ASSETS)

clean:
	@rm -Rfv $(O) obj

distclean: clean
	@make -C vendor/bulletcapi/capi cleanall
	@rm -fv vendor/bulletcapi/capi/{libcapi.so,capi.o}
	@make -C tools/model distclean
	@make -C tools/texture distclean
	@rm -Rfv $(shell cat .gitignore)

$(O): $(OBJS)
	$(CC) $^ -o $@ $(LDFLAGS_COMMON) $(LDFLAGS)

obj/%.o: src/%.c
	@mkdir -pv $(shell dirname $@)
	$(CC) -c $< -o $@ $(CFLAGS_DEBUG) $(CFLAGS)

run: all
	./$(O)

debug: all
	$(GDB) -ex run --batch ./$(O)

release: $(SRCS)
	$(CC) $^ -o $(O) $(CFLAGS_RELEASE) $(CFLAGS) $(LDFLAGS_COMMON) $(LDFLAGS)
	$(STRIP) -s $(O)

dist: release
	@mkdir -pv dist
	@cp -Rv $(O) lib*.so shaders $(ASSETS) dist

%.bin: shaders/%.vert shaders/%.frag
	@rm -fv $@

camera-albedo.bin: albedo.bin
	@cp -v $< $@

help:
	@echo "all - Build executable"
	@echo "clean - Remove executable and obj directory"
	@echo "distclean - Remove everything in .gitignore"
	@echo "$(O) - Build $(O)"
	@echo "obj/<object>.o - Build obj/<object>.o"
	@echo "run - Run executable"
	@echo "debug - Run executable with gdb"
	@echo "release - Build optimised executable"
	@echo "dist - Package optimised executable and all local dependencies in a dist directory"
	@echo "<shader>.bin - Remove a shader program binary for re-creating if the source shader files have changed"
	@echo "camera-albedo.bin - Copy albedo.bin to camera-albedo.bin"
