include ../../make.def

VERSION = 1.13
NAME = GotoBLAS2
SOURCE = $(NAME)-$(VERSION)
TAR = $(SOURCE)_bsd.tar.gz

default: .make-$(NAME)

.make-$(NAME): $(NAME)
	@if [ $(ARCH) = "x86_64" ]; then cd $(NAME) && ./quickbuild.64bit; touch $@; \
	elif [ $(ARCH) = "x86" ]; then  cd $(NAME) && ./quickbuild.32; touch $@; \
	else \
		echo No $(ARCH) found for $(NAME); \
		exit 1;	\
	fi

$(NAME):
	@if [ -f $(TAR) ] ; then zcat $(TAR) | tar xf -; touch $@; \
	else								\
		echo No $(TAR) found;				\
		echo Try download from: http://www.tacc.utexas.edu/tacc-projects/gotoblas2/downloads/ ; \
		exit 1;							\
	fi


install: .make-$(NAME)
	mkdir -p $(CBENCHTEST)/$(NAME)
	cp libgoto* $(CBENCHTEST)/$(NAME) 

clean:

distclean: 
	@if [ -f .make-$(NAME) ]; then rm -f .make-$(NAME) ; fi
	@if [ -f ._$(NAME) ]; then rm -f ._$(NAME) ; fi
	@if [ -d $(NAME) ]; then rm -rf $(NAME) ; fi
