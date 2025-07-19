
SHELL=/bin/bash

COMPONENTS += Y11-server
COMPONENTS += libY11-ui
COMPONENTS += libY11-client

all: build

$(COMPONENTS):
	git worktree add $@ $@

init: $(COMPONENTS)

build: $(patsubst %,build//%,$(COMPONENTS))
build//%:
	make -C $* all

clean: $(patsubst %,clean//%,$(COMPONENTS))
clean//%:
	make -C $* clean

destroy:
	for component in $(COMPONENTS); \
	do \
	  if [ -d "$$component" ]; then git worktree remove "$$component"; fi \
	done

force-destroy:
	read -n 1 -p "Delete directories $(COMPONENTS)? [yN]" yn; echo; [ "$$yn" = 'y' ]
	for component in $(COMPONENTS); \
	do \
	  if [ -d "$$component" ]; then git worktree remove --force "$$component"; fi \
	done

.PHONY: all init build build//% clean clean//% destroy force-destroy

