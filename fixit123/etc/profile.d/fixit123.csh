if ( $?FIXIT123_CONF ) then
    eval `grep -v '^[:blank:]*#' $FIXIT123_CONF | sed 's|\([^=]*\)=\([^=]*\)|setenv \1 \2|g' | sed 's|$|;|'`
else if ( -f /etc/sysconfig/fixit123 ) then
    eval `grep -v '^[:blank:]*#' /etc/sysconfig/fixit123 | sed 's|\([^=]*\)=\([^=]*\)|setenv \1 \2|g' | sed 's|$|;|'`
	setenv FIXIT123_CONF /etc/sysconfig/fixit123
else
    echo "fixit123 configuration not defined!"
    echo "I hope you have set proper environmental variables!"
endif

if ( ! $?NODECHECKS_HOME ) then
	setenv NODECHECKS_HOME $FIXIT123_HOME/libexec/nodechecks
endif

