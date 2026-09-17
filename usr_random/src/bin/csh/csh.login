# standard MACH3.0+BSD4.3 .login

setenv TERM ""

eval "`machterm -csh`"

if ("$TERM" == '') then
  echo -n "term: "
  set i = $<
  if ("$i" == '') then
    setenv TERM "ansi"
  else
    setenv TERM "$i"
  endif
  unset i
  echo ""
endif

