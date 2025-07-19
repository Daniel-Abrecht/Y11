
SHELL=/bin/bash

COMPONENTS += Y11-server
COMPONENTS += Y11-ui
COMPONENTS += libY11-client

all: init

$(COMPONENTS):
	git worktree add $@

init: $(COMPONENTS)

destroy:
	for component in $(COMPONENTS); \
	do \
	  if [ -d "$$component" ]; then git worktree remove "$$component"; fi \
	done

force-destroy:
	read -n 1 -p "Delete directories Y11-server Y11-ui and libY11-client? [yN]" yn; echo; [ "$$yn" = 'y' ]
	for component in $(COMPONENTS); \
	do \
	  if [ -d "$$component" ]; then git worktree remove --force "$$component"; fi \
	done

.PHONY: all init

