# just using this to be able to deal with the 
# directory tree easier

# build a list of directories, once... thus the := syntax
DIRLIST := $(filter-out Make%, $(shell ls))

default:
	$(MAKE) PASSTARGET=all $(DIRLIST)

%:
	$(MAKE) PASSTARGET=$@ $(DIRLIST)

.PHONY: $(DIRLIST)
$(DIRLIST):
	$(MAKE) -C $@ $(PASSTARGET)

