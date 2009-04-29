# just using this to be able to deal with the 
# directory tree easier

# build a list of directories, once... thus the := syntax
DIRLIST := $(filter-out Make%, $(shell ls -p | grep \/))

default:
	$(MAKE) PASSTARGET=all $(DIRLIST)

binstatus:
	$(MAKE) PASSTARGET=$@ $(DIRLIST)
	@echo;echo
	@for dir in $(DIRLIST); do \
		[ -f $$dir/.binstatus ] && cat $$dir/.binstatus; \
	done

%:
	$(MAKE) PASSTARGET=$@ $(DIRLIST)

.PHONY: $(DIRLIST)
$(DIRLIST):
	$(MAKE) -C $@ $(PASSTARGET)

