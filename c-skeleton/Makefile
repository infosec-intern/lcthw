# may need to be adjusted depending on platform
CFLAGS=-g -Wall -Wextra -Isrc -rdynamic -DNDEBUG $(OPFLAGS)
LIBS=-ldl $(OPTLIBS)
# only takes effect if the builder doesn't give a PREFIX setting
PREFIX?=/usr/local

# both wildcard patterns needed so subdirectories get built too
SOURCES=$(wildcard src/**/*.c src/*.c)
# generate a list of object files from all the above C source files
# mv xxx.c xxx.o && echo "xxx.o" >> SOURCES
OBJECTS=$(patsubst %.c,%.o,$(SOURCES))

TEST_SRC=$(wildcard tests/*_tests.c)
TESTS=$(patsubst %.c,%,$(TEST_SRC))

# holds the ultimate target to build
TARGET=build/liblchtw.a
SO_TARGET=$(patsubst %.a,%.so,$(TARGET))

# The Target Build
all: $(TARGET) $(SO_TARGET) tests

dev: CFLAGS=-g -Wall -Isrc -Wall -Wextra $(OPTFLAGS)
dev: all

$(TARGET): CFLAGS += -fPIC
$(TARGET): build $(OBJECTS)
	ar rcs $@ $(OBJECTS)
	ranlib $@

$(SO_TARGET): $(TARGET) $(OBJECTS)
	$(CC) -shared -o $@ $(OBJECTS)

build:
	@mkdir -p build
	@mkdir -p bin

# The Unit Tests
# ignore trying to find the 'tests' file and just treat it as a directory
.PHONY: tests
# link the target we've built to teach test
tests: CFLAGS += $(TARGET)
tests: $(TESTS)
	sh ./tests/runtests.sh

# The Cleaner
# .dSYM files are artifacts from XCode on OSX
clean:
	rm -rf build $(OBJECTS) $(TESTS)
	rm -rf tests/tests.log
	find . -name "*.gc*" -exec rm {} \;
	rm -rf `find . -name "*.dSYM" -print`

# The Install
install: all
	install -d $(DESTDIR)/$(PREFIX)/lib/
	install $(TARGET) $(DESTDIR)/$(PREFIX)/lib/

# The Checker
check:
	@echo Files with potentially dangerous functions.
	@egrep '[^_.>a-zA-Z0-9](str(n?cpy|n?cat|xfrm|n?dup|str|pbrk|tok|_)|stpn?cpy|a?sn?printf|byte_)' $(SOURCES) || true
