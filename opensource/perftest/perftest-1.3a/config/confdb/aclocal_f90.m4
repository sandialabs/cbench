dnl
dnl Macros for Fortran 90
dnl
dnl We'd like to have a PAC_LANG_FORTRAN90 that worked with AC_TRY_xxx, but
dnl that would require too many changes to autoconf macros.
dnl
AC_DEFUN(PAC_LANG_FORTRAN90,
[AC_REQUIRE([PAC_PROG_F90])
define([AC_LANG], [FORTRAN90])dnl
ac_ext=$pac_cv_f90_ext
ac_compile='${F90-f90} -c $F90FLAGS conftest.$ac_ext 1>&AC_FD_CC'
ac_link='${F90-f90} -o conftest${ac_exeext} $F90FLAGS $LDFLAGS conftest.$ac_ext $LIBS 1>&AC_FD_CC'
cross_compiling=$pac_cv_prog_f90_cross
])
dnl
dnl This is an addition for AC_TRY_COMPILE, but for f90.  If the current 
dnl language is not f90, it does a save/restore
AC_DEFUN(PAC_TRY_F90_COMPILE,
[AC_REQUIRE([PAC_LANG_FORTRAN90])
ifelse(AC_LANG, FORTRAN90,,[AC_LANG_SAVE
PAC_LANG_FORTRAN90
define([NEED_POP],yes)])
cat > conftest.$ac_ext <<EOF
      program main
[$2]
      end
EOF
if AC_TRY_EVAL(ac_compile); then
  ifelse([$3], , :, [rm -rf conftest*
  $3])
else
  echo "configure: failed program was:" >&AC_FD_CC
  cat conftest.$ac_ext >&AC_FD_CC
ifelse([$4], , , [  rm -rf conftest*
  $4
])dnl
fi
rm -f conftest*
ifelse(NEED_POP,yes,[
undefine([NEED_POP])
AC_LANG_RESTORE])
])
dnl
dnl PAC_F90_MODULE_EXT(action if found,action if not found)
dnl
AC_DEFUN(PAC_F90_MODULE_EXT,
[AC_CACHE_CHECK([for Fortran 90 module extension],
pac_cv_f90_module_ext,[
pac_cv_f90_module_case="unknown"
cat >conftest.$ac_f90ext <<EOF
	module conftest
        integer n
        parameter (n=1)
        end module conftest
EOF
if AC_TRY_EVAL(ac_f90compile) ; then
   dnl Look for module name
   pac_MOD=`ls conftest* 2>&1 | grep -v conftest.$ac_f90ext | grep -v conftest.o`
   pac_MOD=`echo $pac_MOD | sed -e 's/conftest\.//g'`
   pac_cv_f90_module_case="lower"
   if test "X$pac_MOD" = "X" ; then
	pac_MOD=`ls CONFTEST* 2>&1 | grep -v CONFTEST.f | grep -v CONFTEST.o`
        pac_MOD=`echo $pac_MOD | sed -e 's/CONFTEST\.//g'`
	if test -n "$pac_MOD" ; then
	    testname="CONFTEST"
	    pac_cv_f90_module_case="upper"
	fi
    fi
    if test -z "$pac_MOD" ; then 
	pac_cv_f90_module_ext="unknown"
    else
	pac_cv_f90_module_ext=$pac_MOD
    fi
else
    echo "configure: failed program was:" >&AC_FD_CC
    cat conftest.$ac_f90ext >&AC_FD_CC
    pac_cv_f90_module_ext="unknown"
fi
rm -f conftest*
])
AC_SUBST(F90MODEXT)
if test "$pac_cv_f90_module_ext" = "unknown" ; then
    ifelse($2,,:,[$2])
else
    ifelse($1,,F90MODEXT=$pac_MOD,[$1])
fi
])
dnl
dnl PAC_F90_MODULE_INCFLAG
AC_DEFUN(PAC_F90_MODULE_INCFLAG,[
AC_CACHE_CHECK([for Fortran 90 module include flag],
pac_cv_f90_module_incflag,[
AC_REQUIRE([PAC_F90_MODULE_EXT])
cat >conftest.$ac_f90ext <<EOF
	module conf
        integer n
        parameter (n=1)
        end module conf
EOF
pac_madedir="no"
if test ! -d conf ; then mkdir conf ; pac_madedir="yes"; fi
if test "$pac_cv_f90_module_case" = "upper" ; then
    pac_module="CONF.$pac_cv_f90_module_ext"
else
    pac_module="conf.$pac_cv_f90_module_ext"
fi
if AC_TRY_EVAL(ac_f90compile) ; then
    cp $pac_module conf
else
    echo "configure: failed program was:" >&AC_FD_CC
    cat conftest.$ac_f90ext >&AC_FD_CC
fi
rm -f conftest.$ac_f90ext
cat >conftest.$ac_f90ext <<EOF
    program main
    use conf
    end
EOF
if ${F90-f90} -c $F90FLAGS -Iconf conftest.$ac_f90ext 1>&AC_FD_CC && \
	test -s conftest.o ; then
    pac_cv_f90_module_incflag="-I"
elif ${F90-f90} -c $F90FLAGS -Mconf conftest.$ac_f90ext 1>&AC_FD_CC && \
	test-s conftest.o ; then
    pac_cv_f90_module_incflag="-M"
elif ${F90-f90} -c $F90FLAGS -pconf conftest.$ac_f90ext 1>&AC_FD_CC && \
	test -s conftest.o ; then
    pac_cv_f90_module_incflag="-p"
else
    pac_cv_f90_module_incflag="unknown"
fi
if test "$pac_madedir" = "yes" ; then rm -rf conf ; fi
rm -f conftest*
])
AC_SUBST(F90MODINCFLAG)
F90MODINCFLAG=$pac_cv_f90_module_incflag
])
AC_DEFUN(PAC_F90_MODULE,[
PAC_F90_MODULE_EXT
PAC_F90_MODULE_INCFLAG
])
AC_DEFUN(PAC_F90_EXT,[
AC_CACHE_CHECK([whether Fortran 90 accepts f90 suffix],
pac_cv_f90_ext_f90,[
save_ac_f90ext=$ac_f90ext
ac_f90ext="f90"
PAC_TRY_F90_COMPILE(,,pac_cv_f90_ext_f90="yes",pac_cv_f90_ext_f90="no")
ac_f90ext=$save_ac_f90ext
])
])
dnl
dnl/*D 
dnl PAC_PROG_F90_INT_KIND - Determine kind parameter for an integer with
dnl the specified number of bytes.
dnl
dnl Synopsis:
dnl  PAC_PROG_F90_INT_KIND(variable-to-set,number-of-bytes,[cross-size])
dnl
dnl D*/
AC_DEFUN(PAC_PROG_F90_INT_KIND,[
# Set the default
$1=-1
if test "$pac_cv_prog_f90_cross" = "yes" ; then
    $1="$3"
else
if test -n "$ac_f90compile" ; then
    AC_MSG_CHECKING([for Fortran 90 integer kind for $2-byte integers])
    # Convert bytes to digits
    case $2 in 
	1) sellen=2 ;;
	2) sellen=4 ;;
	4) sellen=8 ;;
	8) sellen=16 ;;
       16) sellen=30 ;;
	*) sellen=8 ;;
    esac
    # Check for cached value
    eval testval=\$"pac_cv_prog_f90_int_kind_$sellen"
    if test -n "$testval" ; then 
        AC_MSG_RESULT([$testval (cached)])
	$1=$testval
    else
        # must compute
        rm -f conftest*
        cat <<EOF > conftest.$ac_f90ext
      program main
      integer i
      i = selected_int_kind($sellen)
      open(8, file="conftest1.out", form="formatted")
      write (8,*) i
      close(8)
      stop
      end
EOF
        KINDVAL="unavailable"
        eval "pac_cv_prog_f90_int_kind_$sellen"=-1
        if AC_TRY_EVAL(ac_f90link) && test -s conftest ; then
            ./conftest >>config.log 2>&1
            if test -s conftest1.out ; then
	        # Because of write, there may be a leading blank.
                KINDVAL=`cat conftest1.out | sed 's/ //g'`
 	        eval "pac_cv_prog_f90_int_kind_$sellen"=$KINDVAL
	        $1=$KINDVAL
            fi
        fi
        rm -f conftest*
	AC_MSG_RESULT($KINDVAL)
    fi # not cached
fi # Has Fortran 90
fi # is not cross compiling
])dnl
dnl
dnl
dnl Note: This checks for f95 before f90, since F95 is the more recent
dnl revision of Fortran 90.  efc is the Intel Fortran 77/90/95 compiler
AC_DEFUN(PAC_PROG_F90,[
if test -z "$F90" ; then
    AC_CHECK_PROGS(F90,f95 f90 xlf90 pgf90 efc)
    test -z "$F90" && AC_MSG_WARN([no acceptable Fortran 90 compiler found in \$PATH])
fi
if test -n "$F90" ; then
     PAC_PROG_F90_WORKS
fi
dnl Cache these so we don't need to change in and out of f90 mode
ac_f90ext=$pac_cv_f90_ext
ac_f90compile='${F90-f90} -c $F90FLAGS conftest.$ac_f90ext 1>&AC_FD_CC'
ac_f90link='${F90-f90} -o conftest${ac_exeext} $F90FLAGS $LDFLAGS conftest.$ac_f90ext $LIBS 1>&AC_FD_CC'
# Check for problems with Intel efc compiler
cat > conftest.$ac_f90ext <<EOF
        program main
        end
EOF
pac_msg=`$F90 -o conftest $F90FLAGS $LDFLAGS conftest.$ac_f90ext $LIBS 2>&1 | grep 'bfd assertion fail'`
if test -n "$pac_msg" ; then
    pac_msg=`$F90 -o conftest $F90FLAGS $LDFLAGS conftest.$ac_f90ext -i_dynamic $LIBS 2>&1 | grep 'bfd assertion fail'`
    if test -z "$pac_msg" ; then LDFLAGS="-i_dynamic" ; fi
    # There should really be f90linker flags rather than generic ldflags.
fi
])
dnl Internal routine for testing F90
dnl PAC_PROG_F90_WORKS()
AC_DEFUN(PAC_PROG_F90_WORKS,
[AC_MSG_CHECKING([for extension for Fortran 90 programs])
pac_cv_f90_ext="f90"
cat > conftest.$pac_cv_f90_ext <<EOF
      program conftest
      end
EOF
ac_compile='${F90-f90} -c $F90FLAGS conftest.$pac_cv_f90_ext 1>&AC_FD_CC'
if AC_TRY_EVAL(ac_compile) ; then
    AC_MSG_RESULT([f90])
else
    rm -f conftest*
    pac_cv_f90_ext="f"
    cat > conftest.$pac_cv_f90_ext <<EOF
      program conftest
      end
EOF
    if AC_TRY_EVAL(ac_compile) ; then
	AC_MSG_RESULT([f])
    else
        AC_MSG_RESULT([unknown!])
    fi
fi
AC_MSG_CHECKING([whether the Fortran 90 compiler ($F90 $F90FLAGS $LDFLAGS) works])
AC_LANG_SAVE
# We cannot use _LANG_FORTRAN90 here because we will usually be executing this
# test in the context of _PROG_F90, which is a require on _LANG_FORTRAN90.
# Instead, we insert the necessary code from _LANG_FORTRAN90 here
dnl PAC_LANG_FORTRAN90
dnl define(ifdef([_AC_LANG],[_AC_LANG],[AC_LANG]), [FORTRAN90])dnl
define([AC_LANG], [FORTRAN90])dnl
ac_ext=$pac_cv_f90_ext
ac_compile='${F90-f90} -c $F90FLAGS conftest.$ac_ext 1>&AC_FD_CC'
ac_link='${F90-f90} -o conftest${ac_exeext} $F90FLAGS $LDFLAGS conftest.$ac_ext $LIBS 1>&AC_FD_CC'
cross_compiling=$pac_cv_prog_f90_cross
cat >conftest.$ac_ext <<EOF
      program conftest
      end
EOF
if AC_TRY_EVAL(ac_link) && test -s conftest${ac_exeect} ; then
    pac_cv_prog_f90_works="yes"
    if (./conftest; exit) 2>/dev/null ; then
        pac_cv_prog_f90_cross="no"
    else
        pac_cv_prog_f90_cross="yes"
    fi
else
  echo "configure: failed program was:" >&AC_FD_CC
  cat conftest.$ac_ext >&AC_FD_CC
  pac_cv_prog_f90_works="no"
fi
rm -f conftest*
AC_LANG_RESTORE
AC_MSG_RESULT($pac_cv_prog_f90_works)
if test $pac_cv_prog_f90_works = no; then
  AC_MSG_WARN([installation or configuration problem: Fortran 90 compiler cannot create executables.])
fi
AC_MSG_CHECKING([whether the Fortran 90 compiler ($F90 $F90FLAGS $LDFLAGS) is a cross-compiler])
AC_MSG_RESULT($pac_cv_prog_f90_cross)
cross_compiling=$pac_cv_prog_f90_cross
])
dnl
dnl The following looks for F90 options to enable th specified f90 compiler
dnl to work with the f77 compiler, particularly for accessing command-line
dnl arguments
